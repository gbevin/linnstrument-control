/*
 Copyright 2015 Uwyn SPRL (www.uwyn.com)
 
 Written by Geert Bevin (http://gbevin.com).
 
 This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
 or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
*/
#include "LinnStrumentSerialMac.h"

#include "ControlApplication.h"

enum {
    kVK_Return                    = 0x24,
    kVK_Tab                       = 0x30,
    kVK_Space                     = 0x31,
    kVK_Delete                    = 0x33,
    kVK_Escape                    = 0x35,
    kVK_Command                   = 0x37,
    kVK_Shift                     = 0x38,
    kVK_CapsLock                  = 0x39,
    kVK_Option                    = 0x3A,
    kVK_Control                   = 0x3B,
    kVK_RightShift                = 0x3C,
    kVK_RightOption               = 0x3D,
    kVK_RightControl              = 0x3E,
    kVK_Function                  = 0x3F,
    kVK_F17                       = 0x40,
    kVK_VolumeUp                  = 0x48,
    kVK_VolumeDown                = 0x49,
    kVK_Mute                      = 0x4A,
    kVK_F18                       = 0x4F,
    kVK_F19                       = 0x50,
    kVK_F20                       = 0x5A,
    kVK_F5                        = 0x60,
    kVK_F6                        = 0x61,
    kVK_F7                        = 0x62,
    kVK_F3                        = 0x63,
    kVK_F8                        = 0x64,
    kVK_F9                        = 0x65,
    kVK_F11                       = 0x67,
    kVK_F13                       = 0x69,
    kVK_F16                       = 0x6A,
    kVK_F14                       = 0x6B,
    kVK_F10                       = 0x6D,
    kVK_F12                       = 0x6F,
    kVK_F15                       = 0x71,
    kVK_Help                      = 0x72,
    kVK_Home                      = 0x73,
    kVK_PageUp                    = 0x74,
    kVK_ForwardDelete             = 0x75,
    kVK_F4                        = 0x76,
    kVK_End                       = 0x77,
    kVK_F2                        = 0x78,
    kVK_PageDown                  = 0x79,
    kVK_F1                        = 0x7A,
    kVK_LeftArrow                 = 0x7B,
    kVK_RightArrow                = 0x7C,
    kVK_DownArrow                 = 0x7D,
    kVK_UpArrow                   = 0x7E
};

LinnStrumentSerial::LinnStrumentSerial() : lastDeviceDetect(0), serialOpenMoment(0), state(NotDetected)
{
    initializeMapping();
}

void LinnStrumentSerial::initializeMapping()
{
    for (int i = 0; i < 208; ++i) {
        keyMapping[i] = UINT16_MAX;
    }
    
    keyMapping[7 + 26*5] = kVK_Escape;
    keyMapping[8 + 26*5] = kVK_F1;
    keyMapping[9 + 26*5] = kVK_F2;
    keyMapping[10 + 26*5] = kVK_F3;
    keyMapping[11 + 26*5] = kVK_F4;
    keyMapping[12 + 26*5] = kVK_F5;
    keyMapping[13 + 26*5] = kVK_ForwardDelete;
    keyMapping[14 + 26*5] = kVK_F6;
    keyMapping[15 + 26*5] = kVK_F7;
    keyMapping[16 + 26*5] = kVK_F8;
    keyMapping[17 + 26*5] = kVK_F9;
    keyMapping[18 + 26*5] = kVK_F10;
    keyMapping[19 + 26*5] = kVK_F11;
    keyMapping[20 + 26*5] = kVK_F12;

    keyMapping[7 + 26*4] = systemEvents.getNativeCode('`');
    keyMapping[8 + 26*4] = systemEvents.getNativeCode('1');
    keyMapping[9 + 26*4] = systemEvents.getNativeCode('2');
    keyMapping[10 + 26*4] = systemEvents.getNativeCode('3');
    keyMapping[11 + 26*4] = systemEvents.getNativeCode('4');
    keyMapping[12 + 26*4] = systemEvents.getNativeCode('5');
    keyMapping[13 + 26*4] = kVK_Delete;
    keyMapping[14 + 26*4] = systemEvents.getNativeCode('6');
    keyMapping[15 + 26*4] = systemEvents.getNativeCode('7');
    keyMapping[16 + 26*4] = systemEvents.getNativeCode('8');
    keyMapping[17 + 26*4] = systemEvents.getNativeCode('9');
    keyMapping[18 + 26*4] = systemEvents.getNativeCode('0');
    keyMapping[19 + 26*4] = systemEvents.getNativeCode('-');
    keyMapping[20 + 26*4] = systemEvents.getNativeCode('=');

    keyMapping[7 + 26*3] = kVK_Tab;
    keyMapping[8 + 26*3] = systemEvents.getNativeCode('q');
    keyMapping[9 + 26*3] = systemEvents.getNativeCode('w');
    keyMapping[10 + 26*3] = systemEvents.getNativeCode('e');
    keyMapping[11 + 26*3] = systemEvents.getNativeCode('r');
    keyMapping[12 + 26*3] = systemEvents.getNativeCode('t');
    keyMapping[13 + 26*3] = kVK_Delete;
    keyMapping[14 + 26*3] = systemEvents.getNativeCode('y');
    keyMapping[15 + 26*3] = systemEvents.getNativeCode('u');
    keyMapping[16 + 26*3] = systemEvents.getNativeCode('i');
    keyMapping[17 + 26*3] = systemEvents.getNativeCode('o');
    keyMapping[18 + 26*3] = systemEvents.getNativeCode('p');
    keyMapping[19 + 26*3] = systemEvents.getNativeCode('[');
    keyMapping[20 + 26*3] = systemEvents.getNativeCode(']');
    
    keyMapping[7 + 26*2] = kVK_Shift;
    keyMapping[8 + 26*2] = systemEvents.getNativeCode('a');
    keyMapping[9 + 26*2] = systemEvents.getNativeCode('s');
    keyMapping[10 + 26*2] = systemEvents.getNativeCode('d');
    keyMapping[11 + 26*2] = systemEvents.getNativeCode('f');
    keyMapping[12 + 26*2] = systemEvents.getNativeCode('g');
    keyMapping[13 + 26*2] = kVK_Return;
    keyMapping[14 + 26*2] = systemEvents.getNativeCode('h');
    keyMapping[15 + 26*2] = systemEvents.getNativeCode('j');
    keyMapping[16 + 26*2] = systemEvents.getNativeCode('k');
    keyMapping[17 + 26*2] = systemEvents.getNativeCode('l');
    keyMapping[18 + 26*2] = systemEvents.getNativeCode(';');
    keyMapping[19 + 26*2] = systemEvents.getNativeCode('\'');
    keyMapping[20 + 26*2] = kVK_Shift;
 
    keyMapping[7 + 26*1] = kVK_Shift;
    keyMapping[8 + 26*1] = systemEvents.getNativeCode('z');
    keyMapping[9 + 26*1] = systemEvents.getNativeCode('x');
    keyMapping[10 + 26*1] = systemEvents.getNativeCode('c');
    keyMapping[11 + 26*1] = systemEvents.getNativeCode('v');
    keyMapping[12 + 26*1] = systemEvents.getNativeCode('b');
    keyMapping[13 + 26*1] = kVK_Return;
    keyMapping[14 + 26*1] = systemEvents.getNativeCode('n');
    keyMapping[15 + 26*1] = systemEvents.getNativeCode('m');
    keyMapping[16 + 26*1] = systemEvents.getNativeCode(',');
    keyMapping[17 + 26*1] = systemEvents.getNativeCode('.');
    keyMapping[18 + 26*1] = systemEvents.getNativeCode('/');
    keyMapping[19 + 26*1] = systemEvents.getNativeCode('\\');
    keyMapping[20 + 26*1] = kVK_Shift;
    keyMapping[22 + 26*1] = kVK_UpArrow;

    keyMapping[7 + 26*0] = kVK_Control;
    keyMapping[8 + 26*0] = kVK_Command;
    keyMapping[9 + 26*0] = kVK_Command;
    keyMapping[10 + 26*0] = kVK_Option;
    keyMapping[11 + 26*0] = kVK_Space;
    keyMapping[12 + 26*0] = kVK_Space;
    keyMapping[13 + 26*0] = kVK_Space;
    keyMapping[14 + 26*0] = kVK_Space;
    keyMapping[15 + 26*0] = kVK_Space;
    keyMapping[16 + 26*0] = kVK_Option;
    keyMapping[17 + 26*0] = kVK_Command;
    keyMapping[18 + 26*0] = kVK_Command;
    keyMapping[19 + 26*0] = kVK_Control;
    keyMapping[21 + 26*0] = kVK_LeftArrow;
    keyMapping[22 + 26*0] = kVK_DownArrow;
    keyMapping[23 + 26*0] = kVK_RightArrow;
    
    startTimer(20);
};

LinnStrumentSerial::~LinnStrumentSerial()
{
    stopTimer();
    
    ensureClosedLinnSerial();
    linnSerial = nullptr;
};

void LinnStrumentSerial::ensureClosedLinnSerial() {
    serial::Serial* s = linnSerial.get();
    linnSerial = nullptr;
    if (s && s->isOpen()) {
        s->close();
    }
}

void LinnStrumentSerial::timerCallback() {
    try {
        if (state != SerialWaitingForRestart && (Time::currentTimeMillis() - lastDeviceDetect > 200)) {
            lastDeviceDetect = Time::currentTimeMillis();
            if (detect()) {
                if (!linnSerial.get()) {
                    state = NotDetected;
                    serialOpenMoment = Time::currentTimeMillis();
                    linnSerial = new serial::Serial(getFullLinnStrumentDevice().toRawUTF8(), 115200, serial::Timeout::simpleTimeout(100));
                    state = SerialWaitingForRestart;
                }
            }
            else {
                state = NotDetected;
                ensureClosedLinnSerial();
            }
        }
        
        if (state == SerialWaitingForRestart) {
            if (Time::currentTimeMillis() - serialOpenMoment > 600) {
                initializeMapping();
                state = SerialOpened;
            }
        }
        else if (state != NotDetected) {
            if (linnSerial->isOpen()) {
                if (state == SerialOpened) {
                    if (initiateControlMode()) {
                        state = ControlModeInitiated;
                    }
                }
                else if (state == ControlModeInitiated ||
                    state == ControlModeActive)
                handleSerialData();
            }
        }
    }
    catch (serial::SerialException e) {
        std::cerr << e.what() << std::endl;
    }
    catch (serial::IOException e) {
        std::cerr << e.what() << std::endl;
    }
    catch (serial::PortNotOpenedException e) {
        std::cerr << e.what() << std::endl;
    }
}

bool LinnStrumentSerial::initiateControlMode() {
    if (!isDetected()) return false;
    if (!linnSerial.get()) return false;
    if (!linnSerial->isOpen()) return false;
    
    try {
        if (linnSerial->write("LC\n") != 3) {
            linnSerial->flushOutput();
            std::cerr << "Couldn't write the complete handshake message to serial device " << getFullLinnStrumentDevice() << std::endl;
            return false;
        }
        
        return true;
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

void LinnStrumentSerial::lightLed(uint8_t col, uint8_t row, LedColor color) {
    static uint8_t led[4];
    led[0] = 'l';
    led[1] = col;
    led[2] = row;
    led[3] = color;
    linnSerial->write(led, 4);
}

void LinnStrumentSerial::handleSerialData()
{
    static uint8_t data[4];
    
    if (!isDetected()) return;
    if (!linnSerial.get()) return;
    if (!linnSerial->isOpen()) return;
    
    try {
        while (linnSerial->available()) {
            int size = linnSerial->read(data, 4);
            if (size <= 0) {
                return;
            }
            
            if (state == ControlModeInitiated) {
                if (strcmp((char *)data, "ACK\n") == 0) {
                    LedColor colorAscii = ColorGreen;
                    LedColor colorModifiers = ColorCyan;
                    LedColor colorSpacing = ColorYellow;

                    lightLed(7, 5, colorModifiers);
                    lightLed(13, 5, colorSpacing);

                    lightLed(8, 4, colorAscii);
                    lightLed(9, 4, colorAscii);
                    lightLed(10, 4, colorAscii);
                    lightLed(11, 4, colorAscii);
                    lightLed(12, 4, colorAscii);
                    lightLed(14, 4, colorAscii);
                    lightLed(15, 4, colorAscii);
                    lightLed(16, 4, colorAscii);
                    lightLed(17, 4, colorAscii);
                    lightLed(18, 4, colorAscii);

                    lightLed(7, 3, colorSpacing);
                    lightLed(8, 3, colorAscii);
                    lightLed(9, 3, colorAscii);
                    lightLed(10, 3, colorAscii);
                    lightLed(11, 3, colorAscii);
                    lightLed(12, 3, colorAscii);
                    lightLed(14, 3, colorAscii);
                    lightLed(15, 3, colorAscii);
                    lightLed(16, 3, colorAscii);
                    lightLed(17, 3, colorAscii);
                    lightLed(18, 3, colorAscii);

                    lightLed(7, 2, colorModifiers);
                    lightLed(8, 2, colorAscii);
                    lightLed(9, 2, colorAscii);
                    lightLed(10, 2, colorAscii);
                    lightLed(11, 2, colorAscii);
                    lightLed(12, 2, colorAscii);
                    lightLed(13, 2, colorSpacing);
                    lightLed(14, 2, colorAscii);
                    lightLed(15, 2, colorAscii);
                    lightLed(16, 2, colorAscii);
                    lightLed(17, 2, colorAscii);
                    lightLed(20, 2, colorModifiers);

                    lightLed(7, 1, colorModifiers);
                    lightLed(8, 1, colorAscii);
                    lightLed(9, 1, colorAscii);
                    lightLed(10, 1, colorAscii);
                    lightLed(11, 1, colorAscii);
                    lightLed(12, 1, colorAscii);
                    lightLed(13, 1, colorSpacing);
                    lightLed(14, 1, colorAscii);
                    lightLed(15, 1, colorAscii);
                    lightLed(20, 1, colorModifiers);
                    lightLed(22, 1, colorSpacing);

                    lightLed(8, 0, colorModifiers);
                    lightLed(9, 0, colorModifiers);
                    lightLed(11, 0, colorSpacing);
                    lightLed(12, 0, colorSpacing);
                    lightLed(13, 0, colorSpacing);
                    lightLed(14, 0, colorSpacing);
                    lightLed(15, 0, colorSpacing);
                    lightLed(17, 0, colorModifiers);
                    lightLed(18, 0, colorModifiers);
                    lightLed(21, 0, colorSpacing);
                    lightLed(22, 0, colorSpacing);
                    lightLed(23, 0, colorSpacing);
                    
                    std::cout << "Control mode activated" << std::endl;
                    state = ControlModeActive;
                }
            }
            else if (state == ControlModeActive) {
                if ((data[0] == 0 || data[0] == 1) && data[3] == '\n') {
                    int key = data[1] + data[2]*26;
                    
                    std::cout << (int)data[0] << " " << (int)data[1] << " " << (int)data[2] << " : " << (int)key << std::endl;

                    if (keyMapping[key] != UINT16_MAX) {
                        unsigned int code = keyMapping[key];
                        
                        switch (data[0]) {
                            case 1:
                                if (code == kVK_Shift) {
                                    currentModifiers = currentModifiers.withFlags(linncontrol::LCModifiers::shiftModifier);
                                }
                                else if (code == kVK_Control) {
                                    currentModifiers = currentModifiers.withFlags(linncontrol::LCModifiers::ctrlModifier);
                                }
                                else if (code == kVK_Option) {
                                    currentModifiers = currentModifiers.withFlags(linncontrol::LCModifiers::altModifier);
                                }
                                else if (code == kVK_Command) {
                                    currentModifiers = currentModifiers.withFlags(linncontrol::LCModifiers::windowsModifier);
                                }
                                
                                systemEvents.keyDown(code, currentModifiers);
                                
                                break;
                            case 0:
                                if (code == kVK_Shift) {
                                    currentModifiers = currentModifiers.withoutFlags(linncontrol::LCModifiers::shiftModifier);
                                }
                                else if (code == kVK_Control) {
                                    currentModifiers = currentModifiers.withoutFlags(linncontrol::LCModifiers::ctrlModifier);
                                }
                                else if (code == kVK_Option) {
                                    currentModifiers = currentModifiers.withoutFlags(linncontrol::LCModifiers::altModifier);
                                }
                                else if (code == kVK_Command) {
                                    currentModifiers = currentModifiers.withoutFlags(linncontrol::LCModifiers::windowsModifier);
                                }

                                systemEvents.keyUp(code, currentModifiers);

                                break;
                        }
                    }
                }
            }
        }
    }
    catch (serial::SerialException e) {
        std::cerr << e.what() << std::endl;
        return;
    }
    catch (serial::IOException e) {
        std::cerr << e.what() << std::endl;
        return;
    }
    catch (serial::PortNotOpenedException e) {
        std::cerr << e.what() << std::endl;
        return;
    }
}
