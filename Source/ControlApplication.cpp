/*
 Copyright 2015 Uwyn SPRL (www.uwyn.com)
 
 Written by Geert Bevin (http://gbevin.com).
 
 This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
 or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
*/
#include "ControlApplication.h"

#include "MainComponent.h"
#if JUCE_MAC
    #include "LinnStrumentSerialMac.h"
#elif JUCE_WINDOWS
	#include "LinnStrumentSerialWindows.h"
#endif

namespace
{
    enum ApplicationMessageType
    {
        detectLinnStrument
    };
    
    struct ApplicationMessage: juce::Message
    {
        ApplicationMessage(const ApplicationMessageType type, void* const payload): type_ (type), payload_ (payload) {}
        ~ApplicationMessage() {}
        
        const ApplicationMessageType type_;
        void* const payload_;
    };
}


ControlApplication::ControlApplication() :
#if JUCE_MAC
    linnStrumentSerial(new LinnStrumentSerialMac())
#elif JUCE_WINDOWS
	linnStrumentSerial(new LinnStrumentSerialWindows())
#endif
{
};
    
const String ControlApplication::getApplicationName()
{
    return ProjectInfo::projectName;
}

const String ControlApplication::getApplicationVersion()
{
    return ProjectInfo::versionString;
}

bool ControlApplication::moreThanOneInstanceAllowed()
{
    return false;
}
    
void ControlApplication::initialise(const String&)
{
    mainWindow = new MainWindow();
    detectLinnStrument();
}

void ControlApplication::shutdown()
{
    mainWindow = nullptr;
}

void ControlApplication::systemRequestedQuit()
{
    quit();
}

void ControlApplication::anotherInstanceStarted(const String&)
{
}

void ControlApplication::handleMessage(const juce::Message &message)
{
    ApplicationMessage *msg = (ApplicationMessage *)&message;
    switch(msg->type_)
    {
        case ApplicationMessageType::detectLinnStrument:
        {
            if (linnStrumentSerial->detect())
            {
                ((MainComponent *)mainWindow->getContentComponent())->setLabelText("Found LinnStrument.", true);
            }
            else
            {
                ((MainComponent *)mainWindow->getContentComponent())->setLabelText("No LinnStrument found!\nPlease make sure Update OS mode is selected in Global Settings.", false);
            }
            startTimer(500);
            break;
        }
    }
}

void ControlApplication::timerCallback()
{
    stopTimer();
    detectLinnStrument();
}

LinnStrumentSerial &ControlApplication::getLinnStrumentSerial()
{
    return *linnStrumentSerial;
}

void ControlApplication::detectLinnStrument()
{
    postMessage(new ApplicationMessage(ApplicationMessageType::detectLinnStrument, (void *)nullptr));
}

void ControlApplication::setProgressText(const String& text)
{
    ((MainComponent *)mainWindow->getContentComponent())->setProgressText(text);
}
