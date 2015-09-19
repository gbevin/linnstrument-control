/*
 Copyright 2015 Uwyn SPRL (www.uwyn.com)
 
 Written by Geert Bevin (http://gbevin.com).
 
 This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
 or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
*/
#include "MainWindow.h"

#include "MainComponent.h"

MainWindow::MainWindow() : DocumentWindow(String(ProjectInfo::projectName)+" "+ProjectInfo::versionString, Colours::white, DocumentWindow::closeButton)
{
    setResizable(false, false);
    setUsingNativeTitleBar(true);
    
    setOpaque(true);
    addToDesktop(getDesktopWindowStyleFlags());
    setContentOwned(new MainComponent(), true);
    
    centreWithSize(getWidth(), getHeight());
    setVisible(true);
}

void MainWindow::closeButtonPressed()
{
    JUCEApplication::getInstance()->systemRequestedQuit();
}
