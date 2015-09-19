/*
 Copyright 2015 Uwyn SPRL (www.uwyn.com)
 
 Written by Geert Bevin (http://gbevin.com).
 
 This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
 or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
*/
#include "LinnStrumentSerialMac.h"

#include "ControlApplication.h"

#include "serial/serial.h"

bool LinnStrumentSerial::readSettings()
{
    if (!isDetected()) return false;
    
    settings.reset();
    
    try {
        serial::Serial linnSerial(getFullLinnStrumentDevice().toRawUTF8(), 115200, serial::Timeout::simpleTimeout(3000));
        
        if (!linnSerial.isOpen()) {
            std::cerr << "Wasn't able to open serial device " << getFullLinnStrumentDevice() << " with baud rate 115200" << std::endl;
            return false;
        }
        
        MessageManager::getInstance()->runDispatchLoopUntil(1500);
        
        if (linnSerial.write("5, 4, 3, 2, 1 ...\n") != 18) {
            std::cerr << "Couldn't write the complete handshake message to serial device " << getFullLinnStrumentDevice() << std::endl;
            return false;
        }
        
        std::string linnGoCode = linnSerial.readline();
        if (linnGoCode != "LinnStruments are go!\n") {
            std::cerr << "Didn't receive the correct go code from serial device " << getFullLinnStrumentDevice() << std::endl;
            return false;
        }
        
        if (linnSerial.write("s") != 1) {
            std::cerr << "Couldn't to give the read settings command to serial device " << getFullLinnStrumentDevice() << std::endl;
            return false;
        }
        
        std::string ackCode = linnSerial.readline();
        if (ackCode != "ACK\n") {
            std::cerr << "Didn't receive the ACK code from serial device " << getFullLinnStrumentDevice() << std::endl;
            return false;
        }
        
        uint8_t sizeBuffer[4];
        if (linnSerial.read(sizeBuffer, 4) != 4) {
            std::cerr << "Couldn't retrieve the size of the settings from device " << getFullLinnStrumentDevice() << std::endl;
            return false;
        }
        
        int32_t settingsSize;
        std::memcpy(&settingsSize, sizeBuffer, sizeof(int32_t));
        
		settings.ensureSize(settingsSize);
		uint8_t* dest = (uint8_t*)settings.getData();
		int32_t remaining = settingsSize;
		while (remaining > 0) {
			size_t actual = linnSerial.read(dest, remaining);
			if (0 == actual) {
				std::cerr << "Couldn't retrieve the settings from device " << getFullLinnStrumentDevice() << std::endl;
				return false;
			}
			remaining -= actual;
			dest += actual;
		}
        
        ackCode = linnSerial.readline();
        if (ackCode != "ACK\n") {
            std::cerr << "Didn't receive read settings finish ACK code from serial device " << getFullLinnStrumentDevice() << std::endl;
            return false;
        }
    }
    catch (serial::SerialException e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    catch (serial::IOException e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    catch (serial::PortNotOpenedException e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    
    return true;
}
