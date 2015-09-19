/*
 Copyright 2015 Uwyn SPRL (www.uwyn.com)
 
 Written by Geert Bevin (http://gbevin.com).
 
 This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
 or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
*/
#include "LinnStrumentSerialWindows.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <windows.h>
#include <setupapi.h>

#include "ControlApplication.h"

#include "disphelper.h"

LinnStrumentSerialWindows::LinnStrumentSerialWindows() : upgradeVerificationPhase(false), upgradeSuccessful(false)
{
}

LinnStrumentSerialWindows::~LinnStrumentSerialWindows()
{
}

String LinnStrumentSerialWindows::getFullLinnStrumentDevice()
{
    if (!isDetected()) return String::empty;
    
    return linnstrumentDevice;
}

bool LinnStrumentSerialWindows::findFirmwareFile()
{
    File current_app = File::getSpecialLocation(File::SpecialLocationType::currentExecutableFile);
    File parent_dir = current_app.getParentDirectory();
    Array<File> firmware_files;
    parent_dir.findChildFiles(firmware_files, File::TypesOfFileToFind::findFiles, false, "*.bin");
    if (firmware_files.size() > 0)
    {
        firmwareFile = firmware_files[0].getFullPathName();
    }
    
    return hasFirmwareFile();
}

bool LinnStrumentSerialWindows::hasFirmwareFile()
{
    return firmwareFile.isNotEmpty();
}

bool LinnStrumentSerialWindows::detect()
{
    if (!hasFirmwareFile()) return false;

    linnstrumentDevice = String::empty;

	DISPATCH_OBJ(wmiSvc);
	DISPATCH_OBJ(colDevices);
	dhInitialize(TRUE);
	dhToggleExceptions(TRUE);
	dhGetObject(L"winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2", NULL, &wmiSvc);
	// only use entities that are COM ports with the vendor and product ID of LinnStrument
	dhGetValue(L"%o", &colDevices, wmiSvc, L".ExecQuery(%S)", L"SELECT * from Win32_PnPEntity WHERE Name LIKE \"%(COM%\" AND PnPDeviceID LIKE \"%VID_F055&PID_0070%\"");
	FOR_EACH(objDevice, colDevices, NULL)
	{
		char* name = NULL;
		char* pnpid = NULL;
		char* match;
		dhGetValue(L"%s", &name, objDevice, L".Name");
		dhGetValue(L"%s", &pnpid, objDevice, L".PnPDeviceID");
		if (name != NULL && pnpid != NULL && (match = strstr(name, "(COM")) != NULL)
		{
			char* comname = strtok(match, "()");
			linnstrumentDevice = String(comname);
			printf("Found LinnStrument %s - %s\n", comname, pnpid);
		}
		dhFreeString(name);
		dhFreeString(pnpid);
	}
	NEXT(objDevice);
	SAFE_RELEASE(colDevices);
	SAFE_RELEASE(wmiSvc);
	dhUninitialize(TRUE);
    
    return isDetected();
}

bool LinnStrumentSerialWindows::isDetected()
{
    return linnstrumentDevice.isNotEmpty();
}

bool LinnStrumentSerialWindows::prepareDevice()
{
    if (!hasFirmwareFile() || !isDetected()) return false;

	DCB dcb;
	HANDLE hCom;
	BOOL fSuccess;

	//  Open a handle to the specified com port.
	hCom = CreateFile(linnstrumentDevice.toWideCharPointer(),
		GENERIC_READ | GENERIC_WRITE,
		0,                //  must be opened with exclusive-access
		NULL,             //  default security attributes
		OPEN_EXISTING,    //  must use OPEN_EXISTING
		0,                //  not overlapped I/O
		NULL);            //  hTemplate must be NULL for comm devices

	if (hCom == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed with error %d.\n", GetLastError());
		return false;
	}

	//  Initialize the DCB structure.
	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	//  Build on the current configuration by first retrieving all current settings.
	fSuccess = GetCommState(hCom, &dcb);
	if (!fSuccess)
	{
		printf("GetCommState failed with error %d.\n", GetLastError());
		CloseHandle(hCom);
		return false;
	}

	//  Fill in some DCB values and set the com state: 
	//  1200 bps, 8 data bits, no parity, and 1 stop bit.
	dcb.BaudRate = CBR_1200;      //  baud rate
	dcb.ByteSize = 8;             //  data size, xmit and rcv
	dcb.Parity = NOPARITY;        //  parity bit
	dcb.StopBits = ONESTOPBIT;    //  stop bit

	fSuccess = SetCommState(hCom, &dcb);
	if (!fSuccess)
	{
		printf("SetCommState failed with error %d.\n", GetLastError());
		CloseHandle(hCom);
		return false;
	}

	CloseHandle(hCom);
	return true;
}

bool LinnStrumentSerialWindows::performUpgrade()
{
    if (!hasFirmwareFile() || !isDetected()) return false;

	upgradeOutput.clear();
	upgradeVerificationPhase = false;
	upgradeSuccessful = false;

	File current_app = File::getSpecialLocation(File::SpecialLocationType::currentExecutableFile);
	File parent_dir = current_app.getParentDirectory();
	File bossac_tool = parent_dir.getChildFile("tools/bossac.exe");
	if (bossac_tool.exists())
	{
		StringArray args;
		args.add(bossac_tool.getFullPathName());
		args.add("-i");
		args.add("--port=" + linnstrumentDevice);
		args.add("-U");
		args.add("false");
		args.add("-e");
		args.add("-w");
		args.add("-v");
		args.add("-b");
		args.add(firmwareFile);
		args.add("-R");
		upgradeChild.start(args);
		startTimer(1);
		return true;
	}
	else
	{
		return false;
	}

	return false;
}

void LinnStrumentSerialWindows::timerCallback()
{
    if (!upgradeChild.isRunning())
    {
        stopTimer();
        
        if (upgradeSuccessful)
        {
			ControlApplication::getApp().restoreSettings();
        }
        else
        {
            ControlApplication::getApp().setUpgradeFailed();
        }
    }
    
    char buffer;
    if (upgradeChild.readProcessOutput(&buffer, 1) > 0)
    {
        upgradeOutput.append(String::charToString(buffer), 1);
        if (buffer == '%')
        {
            if (upgradeOutput.contains("Verify"))
            {
                upgradeVerificationPhase = true;
            }
            
            int index = upgradeOutput.lastIndexOf("] ");
            String progress = upgradeOutput.substring(index+2);
            if (upgradeVerificationPhase && progress == "100%")
            {
                upgradeSuccessful = true;
            }
            ControlApplication::getApp().setProgressText((upgradeVerificationPhase ? "Verifying... " : "Writing... ")+progress);
            upgradeOutput.clear();
        }
    }
}

