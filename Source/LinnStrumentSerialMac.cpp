/*
 Copyright 2015 Uwyn SPRL (www.uwyn.com)
 
 Written by Geert Bevin (http://gbevin.com).
 
 This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
 or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
*/
#include "LinnStrumentSerialMac.h"

#include "ControlApplication.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include <AvailabilityMacros.h>
#include <sys/param.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <sys/ioctl.h>
#include <IOKit/serial/ioss.h>
#include <errno.h>

static const long VENDOR_ID = 0xf055;
static const long PRODUCT_ID = 0x0070;

LinnStrumentSerialMac::LinnStrumentSerialMac()
{
}

LinnStrumentSerialMac::~LinnStrumentSerialMac()
{
}

String LinnStrumentSerialMac::getFullLinnStrumentDevice()
{
    if (!isDetected()) return String::empty;
    
    return "/dev/"+linnstrumentDevice;
}

typedef struct SerialDevice {
    char port[MAXPATHLEN];
    UInt16 vendorId;
    UInt16 productId;
} stSerialDevice;

typedef struct DeviceListItem {
    struct SerialDevice value;
    struct DeviceListItem *next;
    int* length;
} stDeviceListItem;

static kern_return_t FindModems(io_iterator_t *matchingServices);
static io_registry_entry_t GetUsbDevice(char *pathName);
static stDeviceListItem* GetSerialDevices();

static kern_return_t FindModems(io_iterator_t *matchingServices)
{
    kern_return_t kernResult;
    CFMutableDictionaryRef classesToMatch;
    classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
    if (classesToMatch != NULL)
    {
        CFDictionarySetValue(classesToMatch, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));
    }
    
    kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, classesToMatch, matchingServices);
    
    return kernResult;
}

static void ExtractUsbInformation(stSerialDevice *serialDevice, IOUSBDeviceInterface197 **deviceInterface)
{
    kern_return_t kernResult;
    
    UInt16 vendorID;
    kernResult = (*deviceInterface)->GetDeviceVendor(deviceInterface, &vendorID);
    if (KERN_SUCCESS == kernResult)
    {
        serialDevice->vendorId = vendorID;
    }
    
    UInt16 productID;
    kernResult = (*deviceInterface)->GetDeviceProduct(deviceInterface, &productID);
    if (KERN_SUCCESS == kernResult)
    {
        serialDevice->productId = productID;
    }
}

static void MatchUsbDevice(char* pathName, stSerialDevice *serialDevice)
{
    kern_return_t e;
    
    CFMutableDictionaryRef d = IOServiceMatching(kIOUSBDeviceClassName);
    if (d == NULL)
    {
        return;
    }
    
    CFNumberRef numberRef;
    numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &VENDOR_ID);
    CFDictionarySetValue(d, CFSTR(kUSBVendorID), numberRef);
    CFRelease(numberRef);
    numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &PRODUCT_ID);
    CFDictionarySetValue(d, CFSTR(kUSBProductID), numberRef);
    CFRelease(numberRef);
    numberRef = nil;
    
    io_iterator_t matchingServices;
    if ((e = IOServiceGetMatchingServices(kIOMasterPortDefault, d, &matchingServices)))
    {
        return;
    }
    
    io_service_t service;
    Boolean deviceFound = false;
    
    SInt32 score;
    IOCFPlugInInterface **plug;
    IOUSBDeviceInterface197 **deviceInterface;
    while ((service = IOIteratorNext(matchingServices)) && !deviceFound)
    {
        e = IOCreatePlugInInterfaceForService(service, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plug, &score);
        IOObjectRelease(service);
        if (e || !plug) continue;
        
        (*plug)->QueryInterface(plug, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID197), (void **)&deviceInterface);
        (*plug)->Stop(plug);
        
        IODestroyPlugInInterface(plug);
        if (!deviceInterface) continue;
        
        UInt32 locationID;
        (*deviceInterface)->GetLocationID(deviceInterface, &locationID);
        char locationIdHex[255];
        snprintf(locationIdHex, 254, "%X", locationID);
        
        String locationIdString(locationIdHex);
        locationIdString = locationIdString.trimCharactersAtStart("0");
        locationIdString = locationIdString.trimCharactersAtEnd("0");
        String deviceName = "cu.usbmodem"+locationIdString+"1";
        String devicePath = "/dev/"+deviceName;
        File deviceFile(pathName);
        if (devicePath.equalsIgnoreCase(pathName) && deviceFile.exists())
        {
            ExtractUsbInformation(serialDevice, deviceInterface);
            (*deviceInterface)->Release(deviceInterface);
            
            deviceFound = true;
        }
    }

    IOObjectRelease(matchingServices);
}

static stDeviceListItem* GetSerialDevices()
{
    io_iterator_t serialPortIterator;
    char bsdPath[MAXPATHLEN];
    
    FindModems(&serialPortIterator);
    
    io_service_t modemService;
    
    // Initialize the returned path
    *bsdPath = '\0';
    
    stDeviceListItem* devices = NULL;
    stDeviceListItem* lastDevice = NULL;
    int length = 0;
    
    while ((modemService = IOIteratorNext(serialPortIterator)))
    {
        CFTypeRef bsdPathAsCFString;
        
        bsdPathAsCFString = IORegistryEntrySearchCFProperty(modemService, kIOServicePlane, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, kIORegistryIterateRecursively);
        
        if (bsdPathAsCFString)
        {
            Boolean result;
            
            // Convert the path from a CFString to a C (NUL-terminated)
            result = CFStringGetCString((CFStringRef) bsdPathAsCFString, bsdPath, sizeof(bsdPath), kCFStringEncodingUTF8);
            CFRelease(bsdPathAsCFString);
            
            if (result)
            {
                stDeviceListItem *deviceListItem = (stDeviceListItem*) malloc(sizeof(stDeviceListItem));
                stSerialDevice *serialDevice = &(deviceListItem->value);
                strcpy(serialDevice->port, bsdPath);
                serialDevice->vendorId = 0;
                serialDevice->productId = 0;
                deviceListItem->next = NULL;
                deviceListItem->length = &length;
                
                if (devices == NULL)
                {
                    devices = deviceListItem;
                }
                else
                {
                    lastDevice->next = deviceListItem;
                }
                
                lastDevice = deviceListItem;
                length++;
                
                MatchUsbDevice(bsdPath, serialDevice);
            }
        }
        
        // Release the io_service_t now that we are done with it.
        (void) IOObjectRelease(modemService);
    }
    
    IOObjectRelease(serialPortIterator);  // Release the iterator.
    
    return devices;
}

bool LinnStrumentSerialMac::detect()
{
    linnstrumentDevice = String::empty;
    
    stDeviceListItem* devices = GetSerialDevices();
    
    if (*(devices->length) > 0)
    {
        stDeviceListItem* next = devices;
        
        for (int i = 0, len = *(devices->length); i < len && !isDetected(); i++) {
            stSerialDevice device = (* next).value;

            // only use serial devices with the vendor and product ID of LinnStrument
            if (device.vendorId == VENDOR_ID &&
                device.productId == PRODUCT_ID &&
                strstr(device.port, "/dev/") == device.port) {
                linnstrumentDevice = String(device.port);
                linnstrumentDevice = linnstrumentDevice.substring(5);
            }
            
            stDeviceListItem* current = next;
            
            if (next->next != NULL)
            {
                next = next->next;
            }
            
            free(current);
        }
        
    }
        
    return isDetected();
}

bool LinnStrumentSerialMac::isDetected()
{
    return linnstrumentDevice.isNotEmpty();
}
