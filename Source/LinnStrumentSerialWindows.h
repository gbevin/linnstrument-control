/*
 Copyright 2015 Uwyn SPRL (www.uwyn.com)
 
 Written by Geert Bevin (http://gbevin.com).
 
 This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
 or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
*/
#ifndef LINNSTRUMENTSERIALWINDOWS_H_INCLUDED
#define LINNSTRUMENTSERIALWINDOWS_H_INCLUDED

#include "JuceHeader.h"

#include "LinnStrumentSerial.h"

class LinnStrumentSerialWindows : public LinnStrumentSerial,
                                  public Timer
{
public:
	LinnStrumentSerialWindows();
	virtual ~LinnStrumentSerialWindows();
    
    void timerCallback() override;
    
    String getFullLinnStrumentDevice() override;
    bool findFirmwareFile() override;
    bool hasFirmwareFile() override;
    bool detect() override;
    bool isDetected() override;
    bool prepareDevice() override;
    bool performUpgrade() override;
    
private:
    String firmwareFile;
    String linnstrumentDevice;
    ChildProcess detectionChild;
    ChildProcess upgradeChild;
    String upgradeOutput;
    bool upgradeVerificationPhase;
    bool upgradeSuccessful;
};

#endif  // LINNSTRUMENTSERIALWINDOWS_H_INCLUDED
