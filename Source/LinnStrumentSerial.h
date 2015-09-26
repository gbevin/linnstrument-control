/*
 Copyright 2015 Uwyn SPRL (www.uwyn.com)
 
 Written by Geert Bevin (http://gbevin.com).
 
 This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
 or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
*/
#ifndef LINNSTRUMENTSERIAL_H_INCLUDED
#define LINNSTRUMENTSERIAL_H_INCLUDED

#include "JuceHeader.h"

#include "serial/serial.h"
#include "NativeSystemEvents.h"

enum CommunicationState
{
    NotDetected,
    SerialWaitingForRestart,
    SerialOpened,
    ControlModeInitiated,
    ControlModeActive
};

enum LedColor
{
    ColorOff = 0,
    ColorRed = 1,
    ColorYellow = 2,
    ColorGreen = 3,
    ColorCyan = 4,
    ColorBlue = 5,
    ColorMagenta = 6,
    ColorBlack = 7
};

class LinnStrumentSerial: public Timer
{
public:
    LinnStrumentSerial();
    virtual ~LinnStrumentSerial();

    void timerCallback() override;

    virtual String getFullLinnStrumentDevice() = 0;
    virtual bool detect() = 0;
    virtual bool isDetected() = 0;
    
private:
    void initializeMapping();
    void ensureClosedLinnSerial();
    bool initiateControlMode();
    void handleSerialData();
    void lightLed(uint8_t col, uint8_t row, LedColor color);
    
    ScopedPointer<serial::Serial> linnSerial;
    int64 lastDeviceDetect;
    int64 serialOpenMoment;
    CommunicationState state;
    linncontrol::NativeSystemEvents systemEvents;
    linncontrol::LCModifiers currentModifiers;
    unsigned int keyMapping[208];
};

#endif  // LINNSTRUMENTSERIAL_H_INCLUDED
