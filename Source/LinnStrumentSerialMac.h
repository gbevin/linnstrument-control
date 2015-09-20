/*
 Copyright 2015 Uwyn SPRL (www.uwyn.com)
 
 Written by Geert Bevin (http://gbevin.com).
 
 This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
 or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
*/
#ifndef LINNSTRUMENTSERIALMAC_H_INCLUDED
#define LINNSTRUMENTSERIALMAC_H_INCLUDED

#include "JuceHeader.h"

#include "LinnStrumentSerial.h"

class LinnStrumentSerialMac : public LinnStrumentSerial {
public:
    LinnStrumentSerialMac();
    virtual ~LinnStrumentSerialMac();
    
    String getFullLinnStrumentDevice() override;
    bool detect() override;
    bool isDetected() override;
    
private:
    String linnstrumentDevice;
};

#endif  // LINNSTRUMENTSERIALMAC_H_INCLUDED
