#ifndef __linncontrol__OtherActions__
#define __linncontrol__OtherActions__

#include "JuceHeader.h"

namespace linncontrol
{
#if JUCE_WINDOWS
	enum WindowsOtherActions
	{
		APPCOMMAND_BROWSER_BACKWARD    =  1,
		APPCOMMAND_BROWSER_FORWARD     =  2,
		APPCOMMAND_BROWSER_REFRESH     =  3,
		APPCOMMAND_BROWSER_STOP        =  4,
		APPCOMMAND_BROWSER_SEARCH      =  5,
		APPCOMMAND_BROWSER_FAVORITES   =  6,
		APPCOMMAND_BROWSER_HOME        =  7,
		APPCOMMAND_MEDIA_NEXTTRACK     =  11,
		APPCOMMAND_MEDIA_PREVIOUSTRACK =  12,
		APPCOMMAND_MEDIA_STOP          =  13,
		APPCOMMAND_MEDIA_PLAY_PAUSE    =  14,
		APPCOMMAND_LAUNCH_MAIL         =  15,
		APPCOMMAND_LAUNCH_MEDIA_SELECT =  16,
		APPCOMMAND_LAUNCH_APP1         =  17,
		APPCOMMAND_LAUNCH_APP2         =  18,
		APPCOMMAND_BASS_DOWN           =  19,
		APPCOMMAND_BASS_BOOST          =  20,
		APPCOMMAND_BASS_UP             =  21,
		APPCOMMAND_TREBLE_DOWN         =  22,
		APPCOMMAND_TREBLE_UP           =  23,
		APPCOMMAND_MICROPHONE_VOLUME_MUTE = 24,
		APPCOMMAND_MICROPHONE_VOLUME_DOWN = 25,
		APPCOMMAND_MICROPHONE_VOLUME_UP =  26,
		APPCOMMAND_HELP                =  27,
		APPCOMMAND_FIND                =  28,
		APPCOMMAND_NEW                 =  29,
		APPCOMMAND_OPEN                =  30,
		APPCOMMAND_CLOSE               =  31,
		APPCOMMAND_SAVE                =  32,
		APPCOMMAND_PRINT               =  33,
		APPCOMMAND_UNDO                =  34,
		APPCOMMAND_REDO                =  35,
		APPCOMMAND_COPY                =  36,
		APPCOMMAND_CUT                 =  37,
		APPCOMMAND_PASTE               =  38,
		APPCOMMAND_REPLY_TO_MAIL       =  39,
		APPCOMMAND_FORWARD_MAIL        =  40,
		APPCOMMAND_SEND_MAIL           =  41,
		APPCOMMAND_SPELL_CHECK         =  42,
		APPCOMMAND_DICTATE_OR_COMMAND_CONTROL_TOGGLE  = 43,
		APPCOMMAND_MIC_ON_OFF_TOGGLE   =  44,
		APPCOMMAND_CORRECTION_LIST     =  45,
		APPCOMMAND_MEDIA_PLAY          =  46,
		APPCOMMAND_MEDIA_PAUSE         =  47,
		APPCOMMAND_MEDIA_RECORD        =  48,
		APPCOMMAND_MEDIA_FAST_FORWARD  =  49,
		APPCOMMAND_MEDIA_REWIND        =  50,
		APPCOMMAND_MEDIA_CHANNEL_UP    =  51,
		APPCOMMAND_MEDIA_CHANNEL_DOWN  =  52
	};
#endif
#if JUCE_MAC
	enum MacOtherActions
	{
        NX_KEYTYPE_BRIGHTNESS_UP	   = 2,
        NX_KEYTYPE_BRIGHTNESS_DOWN	   = 3,
//        NX_KEYTYPE_HELP			       = 5,
//        NX_POWER_KEY			       = 6,
//        NX_KEYTYPE_CONTRAST_UP		   = 11,
//        NX_KEYTYPE_CONTRAST_DOWN	   = 12,
//        NX_KEYTYPE_LAUNCH_PANEL		   = 13,
//        NX_KEYTYPE_EJECT		       = 14,
        NX_KEYTYPE_VIDMIRROR		   = 15,
        NX_KEYTYPE_NEXT                = 17,
        NX_KEYTYPE_PREVIOUS            = 18,
        NX_KEYTYPE_FAST			       = 19,
        NX_KEYTYPE_REWIND		       = 20,
        NX_KEYTYPE_ILLUMINATION_UP	   = 21,
        NX_KEYTYPE_ILLUMINATION_DOWN   = 22,
        NX_KEYTYPE_ILLUMINATION_TOGGLE = 23,
        SHOW_APPLICATION_WINDOWS       = 100,
        SHOW_DASHBOARD                 = 101,
        SHOW_DESKTOP                   = 102,
        SHOW_LAUNCHPAD                 = 103,
        SHOW_MISSION_CONTROL           = 104
	};
#endif
}

#endif