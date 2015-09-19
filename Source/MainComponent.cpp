/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.2.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
#include "ControlApplication.h"
//[/Headers]

#include "MainComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
MainComponent::MainComponent ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    addAndMakeVisible (progressLabel = new Label ("progress label",
                                                  String::empty));
    progressLabel->setFont (Font (15.00f, Font::plain));
    progressLabel->setJustificationType (Justification::centred);
    progressLabel->setEditable (false, false, false);
    progressLabel->setColour (TextEditor::textColourId, Colours::black);
    progressLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (linnstrumentLabel = new Label ("linnstrument label",
                                                      String::empty));
    linnstrumentLabel->setFont (Font (15.00f, Font::plain));
    linnstrumentLabel->setJustificationType (Justification::centred);
    linnstrumentLabel->setEditable (false, false, false);
    linnstrumentLabel->setColour (TextEditor::textColourId, Colours::black);
    linnstrumentLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (400, 218);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

MainComponent::~MainComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    progressLabel = nullptr;
    linnstrumentLabel = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    progressLabel->setBounds ((getWidth() / 2) - (328 / 2), 176, 328, 24);
    linnstrumentLabel->setBounds ((getWidth() / 2) - (328 / 2), 96, 328, 64);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void MainComponent::setLabelText(const String& text, bool enableButton)
{
    linnstrumentLabel->setText(text, NotificationType::sendNotificationAsync);
}

void MainComponent::setProgressText(const String& text)
{
    progressLabel->setText(text, NotificationType::sendNotificationAsync);
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MainComponent" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="400" initialHeight="218">
  <BACKGROUND backgroundColour="ffffffff"/>
  <LABEL name="progress label" id="25076abe0a4bd824" memberName="progressLabel"
         virtualName="" explicitFocusOrder="0" pos="0Cc 176 328 24" edTextCol="ff000000"
         edBkgCol="0" labelText="" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="36"/>
  <LABEL name="linnstrument label" id="62da816b2e6b995a" memberName="linnstrumentLabel"
         virtualName="" explicitFocusOrder="0" pos="0Cc 96 328 64" edTextCol="ff000000"
         edBkgCol="0" labelText="" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="36"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
