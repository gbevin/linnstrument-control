/*
 Copyright 2015 Uwyn SPRL (www.uwyn.com)
 
 Written by Geert Bevin (http://gbevin.com).
 
 This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
 or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
*/
#ifndef CONTROLAPPLICATION_H_INCLUDED
#define CONTROLAPPLICATION_H_INCLUDED

#include <JuceHeader.h>

#include "MainWindow.h"
#include "LinnStrumentSerial.h"

class ControlApplication : public JUCEApplication,
                           public MessageListener,
                           public Timer
{
public:
    ControlApplication();
    
    inline static ControlApplication &getApp()
    {
        ControlApplication* const app = dynamic_cast<ControlApplication*>(JUCEApplication::getInstance());
        jassert (app != nullptr);
        return *app;
    }
    
    const String getApplicationName() override;
    const String getApplicationVersion() override;
    bool moreThanOneInstanceAllowed() override;

    void initialise(const String& commandLine) override;
    void shutdown() override;
    void systemRequestedQuit() override;
    void anotherInstanceStarted (const String& commandLine) override;
    
    void handleMessage(const juce::Message &message) override;
    void timerCallback() override;
    
    LinnStrumentSerial &getLinnStrumentSerial();
    
    void detectLinnStrument();
    
    void setProgressText(const String& text);

private:

    ScopedPointer<MainWindow> mainWindow;
    ScopedPointer<LinnStrumentSerial> linnStrumentSerial;
};

#endif  // CONTROLAPPLICATION_H_INCLUDED
