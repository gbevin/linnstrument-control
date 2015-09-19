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
        findFirmware,
        connectionWarning,
        detectLinnStrument,
        readSettings,
        prepareDevice,
        updateDevice,
        restoreSettings
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
    findFirmware();
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
        case ApplicationMessageType::findFirmware:
        {
            if (linnStrumentSerial->findFirmwareFile())
            {
                connectionWarning();
            }
            else
            {
                ((MainComponent *)mainWindow->getContentComponent())->setLabelText("No firmware file could be found in the same directory as the updater tool.", false);
                startTimer(300);
            }
            break;
        }
            
        case ApplicationMessageType::connectionWarning:
        {
            ((MainComponent *)mainWindow->getContentComponent())->setLabelText("Please connect LinnStrument's USB cable.\nDO NOT use a USB hub!\n\nThen activate Update OS mode in Global Settings.", false);
            break;
        }
            
        case ApplicationMessageType::detectLinnStrument:
        {
            if (linnStrumentSerial->isDetected() || linnStrumentSerial->detect())
            {
                ((MainComponent *)mainWindow->getContentComponent())->setLabelText("Found LinnStrument ready for OS Update.", true);
            }
            else
            {
                ((MainComponent *)mainWindow->getContentComponent())->setLabelText("No LinnStrument found!\nPlease make sure Update OS mode is selected in Global Settings.", false);
                startTimer(300);
            }
            break;
        }
            
        case ApplicationMessageType::readSettings:
        {
            ((MainComponent *)mainWindow->getContentComponent())->setLabelText("Retrieving LinnStrument's settings.", false);
            
            if (linnStrumentSerial->readSettings())
            {
                ((MainComponent *)mainWindow->getContentComponent())->setLabelText("LinnStrument's settings have been retrieved.", false);
                MessageManager::getInstance()->runDispatchLoopUntil(2000);
            }
            else
            {
                ((MainComponent *)mainWindow->getContentComponent())->setLabelText("Couldn't retrieve LinnStrument's settings,\nprobably due to older firmware.\n\nPerforming upgrade with default settings.", false);
                MessageManager::getInstance()->runDispatchLoopUntil(6000);
            }
            postMessage(new ApplicationMessage(ApplicationMessageType::prepareDevice, (void *)nullptr));
            break;
        }
        
        case ApplicationMessageType::prepareDevice:
        {
            if (linnStrumentSerial->prepareDevice())
            {
                ((MainComponent *)mainWindow->getContentComponent())->setLabelText("LinnStrument has been prepared for OS Update.", false);
				MessageManager::getInstance()->runDispatchLoopUntil(2000);
				postMessage(new ApplicationMessage(ApplicationMessageType::updateDevice, (void *)nullptr));
            }
            else
            {
                ((MainComponent *)mainWindow->getContentComponent())->setLabelText("An unexpected error occurred.\nPlease reconnect LinnStrument, quit and restart the updater.", false);
            }
            break;
        }
            
        case ApplicationMessageType::updateDevice:
        {
            if (linnStrumentSerial->performUpgrade())
            {
                ((MainComponent *)mainWindow->getContentComponent())->setLabelText("Performing LinnStrument firmware update.\nDO NOT disconnect LinnStrument.\nDO NOT quit the updater.", false);
            }
            break;
        }
            
        case ApplicationMessageType::restoreSettings:
        {
            ((MainComponent *)mainWindow->getContentComponent())->setLabelText("Restoring LinnStrument's settings.", false);
            ((MainComponent *)mainWindow->getContentComponent())->setProgressText("");

            if (linnStrumentSerial->restoreSettings())
            {
                ((MainComponent *)mainWindow->getContentComponent())->setLabelText("LinnStrument's settings have been restored.", false);
                setUpgradeDone();
            }
            else
            {
                ((MainComponent *)mainWindow->getContentComponent())->setLabelText("The LinnStrument firmware has been upgraded.\n\nPlease perform the calibration by carefully sliding over the light guides.", false);
            }
            break;
        }
    }
}

void ControlApplication::timerCallback()
{
    if (linnStrumentSerial->hasFirmwareFile())
    {
        detectLinnStrument();
    }
    else
    {
        findFirmware();
    }
    stopTimer();
}

LinnStrumentSerial &ControlApplication::getLinnStrumentSerial()
{
    return *linnStrumentSerial;
}

void ControlApplication::findFirmware()
{
    postMessage(new ApplicationMessage(ApplicationMessageType::findFirmware, (void *)nullptr));
}

void ControlApplication::connectionWarning()
{
    postMessage(new ApplicationMessage(ApplicationMessageType::connectionWarning, (void *)nullptr));
}

void ControlApplication::detectLinnStrument()
{
    postMessage(new ApplicationMessage(ApplicationMessageType::detectLinnStrument, (void *)nullptr));
}

void ControlApplication::upgradeLinnStrument()
{
    postMessage(new ApplicationMessage(ApplicationMessageType::readSettings, (void *)nullptr));
}

void ControlApplication::restoreSettings()
{
    postMessage(new ApplicationMessage(ApplicationMessageType::restoreSettings, (void *)nullptr));
}

void ControlApplication::setUpgradeDone()
{
    ((MainComponent *)mainWindow->getContentComponent())->setLabelText("All done!", false);
    ((MainComponent *)mainWindow->getContentComponent())->setProgressText("");
}

void ControlApplication::setUpgradeFailed()
{
    ((MainComponent *)mainWindow->getContentComponent())->setLabelText("The firmware upgrade failed.\nPlease reconnect LinnStrument, quit and restart the updater.", false);
    ((MainComponent *)mainWindow->getContentComponent())->setProgressText("");
}

void ControlApplication::setProgressText(const String& text)
{
    ((MainComponent *)mainWindow->getContentComponent())->setProgressText(text);
}
