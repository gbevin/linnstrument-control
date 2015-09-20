#include "NativeSystemEvents.h"

#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#include <Quartz/Quartz.h>
#import <Cocoa/Cocoa.h>

#include <iostream>

#include <JuceHeader.h>

#include "OtherActions.h"

// NSEvent.h
#define NSSystemDefined 14

// hidsystem/ev_keymap.h
#define NX_KEYTYPE_SOUND_UP		0
#define NX_KEYTYPE_SOUND_DOWN		1
#define NX_KEYTYPE_BRIGHTNESS_UP	2
#define NX_KEYTYPE_BRIGHTNESS_DOWN	3
#define NX_KEYTYPE_CAPS_LOCK		4
#define NX_KEYTYPE_HELP			5
#define NX_POWER_KEY			6
#define	NX_KEYTYPE_MUTE			7
#define NX_UP_ARROW_KEY			8
#define NX_DOWN_ARROW_KEY		9
#define NX_KEYTYPE_NUM_LOCK		10

#define NX_KEYTYPE_CONTRAST_UP		11
#define NX_KEYTYPE_CONTRAST_DOWN	12
#define NX_KEYTYPE_LAUNCH_PANEL		13
#define NX_KEYTYPE_EJECT		14
#define NX_KEYTYPE_VIDMIRROR		15

#define NX_KEYTYPE_PLAY			16
#define NX_KEYTYPE_NEXT			17
#define NX_KEYTYPE_PREVIOUS		18
#define NX_KEYTYPE_FAST			19
#define NX_KEYTYPE_REWIND		20

#define NX_KEYTYPE_ILLUMINATION_UP	21
#define NX_KEYTYPE_ILLUMINATION_DOWN	22
#define NX_KEYTYPE_ILLUMINATION_TOGGLE	23

CG_EXTERN void CoreDockSendNotification(CFStringRef, void *);

namespace
{
    Rectangle<int> getTotalDisplayArea(Point<int> &position)
    {
        MessageManagerLock lock;
        return Desktop::getInstance().getDisplays().getDisplayContaining(position).totalArea;
    }

    
    // The two methods below come from http://stackoverflow.com/questions/1918841/how-to-convert-ascii-character-to-cgkeycode
    
    /* Returns string representation of key, if it is printable.
     * Ownership follows the Create Rule; that is, it is the caller's
     * responsibility to release the returned object. */
    CFStringRef createStringForKey(CGKeyCode keyCode)
    {
        TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
        CFDataRef layoutData = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
        const UCKeyboardLayout *keyboardLayout =
        (const UCKeyboardLayout *)CFDataGetBytePtr(layoutData);
        
        UInt32 keysDown = 0;
        UniChar chars[4];
        UniCharCount realLength;
        
        UCKeyTranslate(keyboardLayout,
                       keyCode,
                       kUCKeyActionDisplay,
                       0,
                       LMGetKbdType(),
                       kUCKeyTranslateNoDeadKeysBit,
                       &keysDown,
                       sizeof(chars) / sizeof(chars[0]),
                       &realLength,
                       chars);
        CFRelease(currentKeyboard);
        
        return CFStringCreateWithCharacters(kCFAllocatorDefault, chars, 1);
    }
    
    /* Returns key code for given character via the above function, or UINT16_MAX
     * on error. */
    CGKeyCode keyCodeForChar(UniChar character)
    {
        static CFMutableDictionaryRef charToCodeDict = NULL;
        CGKeyCode code;
        CFStringRef charStr = NULL;
        
        /* Generate table of keycodes and characters. */
        if (charToCodeDict == NULL)
        {
            size_t i;
            charToCodeDict = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                       128,
                                                       &kCFCopyStringDictionaryKeyCallBacks,
                                                       NULL);
            if (charToCodeDict == NULL) return UINT16_MAX;
            
            /* Loop through every keycode (0 - 127) to find its current mapping. */
            for (i = 0; i < 128; ++i)
            {
                CFStringRef string = createStringForKey((CGKeyCode)i);
                if (string != NULL)
                {
                    CFDictionaryAddValue(charToCodeDict, string, (const void *)i);
                    CFRelease(string);
                }
            }
        }
        
        charStr = CFStringCreateWithCharacters(kCFAllocatorDefault, &character, 1);
        
        /* Our values may be NULL (0), so we need to use this function. */
        if (!CFDictionaryGetValueIfPresent(charToCodeDict, charStr, (const void **)&code))
        {
            code = UINT16_MAX;
        }
        
        CFRelease(charStr);
        return code;
    }
    
    CGKeyCode make_keycode(const char k)
    {
        CGKeyCode c = keyCodeForChar(k);
        return c;
    }
}

using namespace linncontrol;

class NativeSystemEvents::impl_t
{
public:
    impl_t() : latestFlags_((CGEventFlags)0), mouseEventNumber_(0x4000)
    {
        source_ = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
        CGEventSourceSetUserData(source_, (intptr_t)this);
        
        CFMachPortRef eventTap = CGEventTapCreate(kCGHIDEventTap, kCGTailAppendEventTap, kCGEventTapOptionListenOnly, kCGEventMaskForAllEvents, NativeSystemEvents::impl_t::flagsEventTapCallBack, this);
        CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
        CFRunLoopAddSource(CFRunLoopGetMain(), runLoopSource, kCFRunLoopCommonModes);
        CGEventTapEnable(eventTap, true);
    }
    
    static CGEventRef flagsEventTapCallBack(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
    {
        if (type != kCGEventFlagsChanged)
        {
            return event;
        }
        
        NativeSystemEvents::impl_t *i = (NativeSystemEvents::impl_t *)refcon;
        if (CGEventGetIntegerValueField(event, kCGEventSourceUserData) == (intptr_t)i)
        {
            return event;
        }
        
        i->latestFlags_ = CGEventGetFlags(event);
        return event;
    }
    
    ~impl_t()
    {
        CFRelease(source_);
    }
    
    CGEventFlags getModifierFlags(const LCModifiers &m)
    {
        CGEventFlags existingFlags = latestFlags_.get();
        int flags = 0;
        if (m.isShiftDown() || existingFlags & kCGEventFlagMaskShift)
        {
            flags |= kCGEventFlagMaskShift;
        }
        if (m.isCtrlDown() || existingFlags & kCGEventFlagMaskControl)
        {
            flags |= kCGEventFlagMaskControl;
        }
        if (m.isAltDown() || existingFlags & kCGEventFlagMaskAlternate)
        {
            flags |= kCGEventFlagMaskAlternate;
        }
        if (m.isWindowsDown() || existingFlags & kCGEventFlagMaskCommand)
        {
            flags |= kCGEventFlagMaskCommand;
        }
        return (CGEventFlags)flags;
    }

    void sendKeyEvent(unsigned int code, const LCModifiers &m, bool down)
    {
        CGEventRef e = CGEventCreateKeyboardEvent(source_, (CGKeyCode)code, down);
        if (e)
        {
//            std::cout << "sendKeyEvent code=" << code << " modifiers=" << m.getRawFlags() << " down=" << down << " flag=" << getModifierFlags(m) << std::endl;
            CGEventSetFlags(e, getModifierFlags(m));
            CGEventPost(kCGHIDEventTap, e);
            CFRelease(e);
        }
    }
    
    void keyEvent(unsigned int code, const LCModifiers &m, bool down)
    {
//        std::cout << "keyEvent code=" << code << " modifiers=" << m.getRawFlags() << " down=" << down << std::endl;
        sendKeyEvent(code, m, down);
    }
    
    void mediaKeyEvent(MediaKey k, bool down)
    {
//        std::cout << "mediaKeyEvent key=" << k << " down=" << down << std::endl;
        int key;
        switch (k)
        {
            case MediaPlay:
                key = NX_KEYTYPE_PLAY;
                break;
            case MediaStop:
                return;
            case MediaPrevious:
                key = NX_KEYTYPE_REWIND;
                break;
            case MediaNext:
                key = NX_KEYTYPE_FAST;
                break;
            case MediaMute:
                key = NX_KEYTYPE_MUTE;
                break;
            case MediaQuieter:
                key = NX_KEYTYPE_SOUND_DOWN;
                break;
            case MediaLouder:
                key = NX_KEYTYPE_SOUND_UP;
                break;
        }
        
        appCommandEvent(key, down);
    }
    
    void appCommandEvent(int command, bool down)
    {
        ScopedAutoReleasePool();
        
        JUCE_AUTORELEASEPOOL
        {
            NSPoint point;
            point.x = 0;
            point.y = 0;
            NSEvent *ev = [NSEvent otherEventWithType:NSEventType(NSSystemDefined)
                                             location:point
                                        modifierFlags:(down ? 0xa00 : 0xb00)
                                            timestamp:0
                                         windowNumber:0
                                              context:0
                                              subtype:8
                                                data1:(command << 16) | ((down ? 0xa : 0xb) << 8)
                                                data2:-1];
            CGEventRef e = [ev CGEvent];
            if (e)
            {
                CGEventPost(kCGHIDEventTap, e);
            }
        }
    }
    
    OSStatus SendAppleEventToSystemProcess(AEEventID EventToSend)
    {
        AEAddressDesc targetDesc;
        static const ProcessSerialNumber kPSNOfSystemProcess = { 0, kSystemProcess };
        AppleEvent eventReply = {typeNull, NULL};
        AppleEvent appleEventToSend = {typeNull, NULL};
        
        OSStatus error = noErr;
        
        error = AECreateDesc(typeProcessSerialNumber, &kPSNOfSystemProcess,
                             sizeof(kPSNOfSystemProcess), &targetDesc);
        
        if (error != noErr)
        {
            return(error);
        }
        
        error = AECreateAppleEvent(kCoreEventClass, EventToSend, &targetDesc,
                                   kAutoGenerateReturnID, kAnyTransactionID, &appleEventToSend);
        
        AEDisposeDesc(&targetDesc);
        if (error != noErr)
        {
            return(error);
        }
        
        error = AESend(&appleEventToSend, &eventReply, kAENoReply,
                       kAENormalPriority, kAEDefaultTimeout, NULL, NULL);
        
        AEDisposeDesc(&appleEventToSend);
        if (error != noErr)
        {
            return(error);
        }
        
        AEDisposeDesc(&eventReply);
        
        return(error); 
    }
    
    void systemCommand(SystemCommand command)
    {
        switch (command)
        {
            case SystemLock:
            {
                @try {
                    NSString *p = @"/System/Library/CoreServices/Menu Extras/User.menu/Contents/Resources/CGSession";
                    NSString *a = @"-suspend";
                    
                    NSTask *t = [[NSTask alloc] init];
                    [t setLaunchPath:p];
                    [t setArguments:[NSArray arrayWithObjects:a, nil]];
                    [t setStandardInput:[NSPipe pipe]];
                    
                    [t launch];
                    [t waitUntilExit];
                    
                    [t release];
                }
                @catch (NSException *e)
                {
                    // ignore exception
                }
                break;
            }
            case SystemSleep:
                SendAppleEventToSystemProcess(kAESleep);
                break;
            case SystemLogout:
                SendAppleEventToSystemProcess(kAEReallyLogOut);
                break;
            case SystemRestart:
                SendAppleEventToSystemProcess(kAERestart);
                break;
            case SystemHalt:
                SendAppleEventToSystemProcess(kAEShutDown);
                break;
        }
    }
    
    void modifierEvent(const LCModifiers &m, bool down)
    {
//        std::cout << "modifierEvent m=" << m.getRawFlags() << " down=" << down << std::endl;
        if (m.isAnyModifierKeyDown())
        {
            if (m.isShiftDown())
            {
                sendKeyEvent(kVK_Shift, down ? m : 0, down);
            }
            if (m.isCtrlDown())
            {
                sendKeyEvent(kVK_Control, down ? m : 0, down);
            }
            if (m.isAltDown())
            {
                sendKeyEvent(kVK_Option, down ? m : 0, down);
            }
            if (m.isWindowsDown())
            {
                sendKeyEvent(kVK_Command, down ? m : 0, down);
            }
        }
    }
    
    void mouseEvent(CGEventType type, CGPoint position, int dx, int dy, CGMouseButton button, LCModifiers m)
    {
//        std::cout << "mouseEvent type=" << type << " position=" << position.x << "," << position.y << " d=" << dx << "," << dy << " button=" << button << std::endl;
        CGEventRef e = CGEventCreateMouseEvent(source_, type, position, button);
        if (e)
        {
            CGEventSetFlags(e, getModifierFlags(m));
            CGEventSetIntegerValueField(e, kCGMouseEventNumber, mouseEventNumber_);
            CGEventSetIntegerValueField(e, kCGMouseEventButtonNumber, button);
            CGEventSetIntegerValueField(e, kCGMouseEventDeltaX, dx);
            CGEventSetIntegerValueField(e, kCGMouseEventDeltaY, dy);
            CGEventPost(kCGHIDEventTap, e);
            CFRelease(e);
        }
    }
    
    void mouseDoubleClick(LCModifiers m)
    {
//        std::cout << "mouseDoubleClick" << std::endl;
        CGEventRef reference_event = CGEventCreate(NULL);
        CGPoint position = CGEventGetLocation(reference_event);
        
        CGEventRef e = CGEventCreateMouseEvent(source_, kCGEventLeftMouseDown, position, kCGMouseButtonLeft);
        if (e)
        {
            CGEventSetFlags(e, getModifierFlags(m));
            CGEventSetIntegerValueField(e, kCGMouseEventClickState, 2);
            CGEventPost(kCGHIDEventTap, e);
            CGEventSetType(e, kCGEventLeftMouseUp);
            CGEventPost(kCGHIDEventTap, e);
            CGEventSetType(e, kCGEventLeftMouseDown);
            CGEventPost(kCGHIDEventTap, e);
            CGEventSetType(e, kCGEventLeftMouseUp);
            CGEventPost(kCGHIDEventTap, e);
            CFRelease(e);
        }
    }
    
    void scrollwheel(float dx, float dy, LCModifiers m)
    {
//        std::cout << "scrollwheel d=" << dx << "," << dy << std::endl;
        CGEventRef e = CGEventCreateScrollWheelEvent(source_, kCGScrollEventUnitPixel, 2, int(dx), int(dy));
        if (e)
        {
            CGEventSetFlags(e, getModifierFlags(m));
            CGEventPost(kCGHIDEventTap, e);
            CFRelease(e);
        }
    }
    
    void increaseEventNumber()
    {
        mouseEventNumber_++;
    }

private:
    
    CGEventSourceRef source_;
    Atomic<CGEventFlags> latestFlags_;
    int mouseEventNumber_;
};

NativeSystemEvents::NativeSystemEvents() : mouseButton1Down_(false),  mouseButton2Down_(false), offsetX_(0), offsetY_(0), impl_(new impl_t())
{
    for (int i = 0; i < 3 ; ++i)
    {
        mouseDownPosition_[i][0] = -1;
        mouseDownPosition_[i][1] = -1;
    }
}

NativeSystemEvents::~NativeSystemEvents()
{
    delete impl_;
}

unsigned int NativeSystemEvents::getNativeCode(const char ch)
{
    return make_keycode(ch);
}

void NativeSystemEvents::keyDown(unsigned int nativecode, LCModifiers modifiers)
{
    impl_->keyEvent(nativecode, modifiers, true);
}

void NativeSystemEvents::keyUp(unsigned int nativecode, LCModifiers modifiers)
{
    impl_->keyEvent(nativecode, modifiers, false);
}

void NativeSystemEvents::capsLockDown(LCModifiers m)
{
    impl_->keyEvent(kVK_CapsLock, m, true);
}

void NativeSystemEvents::capsLockUp(LCModifiers m)
{
    impl_->keyEvent(kVK_CapsLock, m, false);
}

void NativeSystemEvents::modifierDown(LCModifiers m)
{
    impl_->modifierEvent(m, true);
}

void NativeSystemEvents::modifierUp(LCModifiers m)
{
    impl_->modifierEvent(m, false);
}

void NativeSystemEvents::mediaDown(MediaKey k)
{
    impl_->mediaKeyEvent(k, true);
}

void NativeSystemEvents::mediaUp(MediaKey k)
{
    impl_->mediaKeyEvent(k, false);
}

void NativeSystemEvents::appCommandDown(int command)
{
    if (command < 100)
    {
        impl_->appCommandEvent(command, true);
    }
    else
    {
        switch (command)
        {
            case MacOtherActions::SHOW_MISSION_CONTROL:
                ::CoreDockSendNotification(CFSTR("com.apple.expose.awake"), NULL);
                break;
                
            case MacOtherActions::SHOW_APPLICATION_WINDOWS:
                ::CoreDockSendNotification(CFSTR("com.apple.expose.front.awake"), NULL);
                break;
                
            case MacOtherActions::SHOW_DESKTOP:
                ::CoreDockSendNotification(CFSTR("com.apple.showdesktop.awake"), NULL);
                break;
                
            case MacOtherActions::SHOW_DASHBOARD:
                ::CoreDockSendNotification(CFSTR("com.apple.dashboard.awake"), NULL);
                break;
                
            case MacOtherActions::SHOW_LAUNCHPAD:
                ::CoreDockSendNotification(CFSTR("com.apple.launchpad.toggle"), NULL);
                break;
                
            default:
                break;
        }
    }
}

void NativeSystemEvents::appCommandUp(int command)
{
    if (command < 100)
    {
        impl_->appCommandEvent(command, false);
    }
}

void NativeSystemEvents::systemCommand(SystemCommand command)
{
    impl_->systemCommand(command);
}

bool NativeSystemEvents::isMouseDownContinuous(int button)
{
    if (button >= 0 && button < 3)
    {
        return mouseDownPosition_[button][0] == -1 && mouseDownPosition_[button][1] == -1;
    }
    return false;
}

void NativeSystemEvents::moveMouse(int dx, int dy)
{
    CGEventRef reference_event = CGEventCreate(NULL);
    CGPoint position;

    if (mouseButton1Down_ && !isMouseDownContinuous(0))
    {
        position = CGPointMake(mouseDownPosition_[0][0], mouseDownPosition_[0][1]);
        dx = 0;
        dy = 0;
    }
    else if (mouseButton2Down_ && !isMouseDownContinuous(1))
    {
        position = CGPointMake(mouseDownPosition_[1][0], mouseDownPosition_[1][1]);
        dx = 0;
        dy = 0;
    }
    else if (mouseButton3Down_ && !isMouseDownContinuous(2))
    {
        position = CGPointMake(mouseDownPosition_[2][0], mouseDownPosition_[2][1]);
        dx = 0;
        dy = 0;
    }
    else
    {
        CGPoint ref = CGEventGetLocation(reference_event);
        Point<int> pos((int) (ref.x+dx), (int) (ref.y+dy));
        Rectangle<int> area = getTotalDisplayArea(pos);
        area.setSize(area.getWidth()-1, area.getHeight()-1);
        Point<int> constr = area.getConstrainedPoint(pos);
        position = CGPointMake(constr.x, constr.y);
    }
    
    if (mouseButton1Down_ && isMouseDownContinuous(0))
    {
        impl_->mouseEvent(kCGEventLeftMouseDragged, position, dx, dy, kCGMouseButtonLeft, LCModifiers());
    }
    else if (mouseButton2Down_ && isMouseDownContinuous(1))
    {
        impl_->mouseEvent(kCGEventRightMouseDragged, position, dx, dy, kCGMouseButtonRight, LCModifiers());
    }
    else if (mouseButton3Down_ &&  isMouseDownContinuous(2))
    {
        impl_->mouseEvent(kCGEventOtherMouseDragged, position, dx, dy, kCGMouseButtonCenter, LCModifiers());
    }
    else
    {
        impl_->mouseEvent(kCGEventMouseMoved, position, dx, dy, (CGMouseButton)0, LCModifiers());
    }
}

void NativeSystemEvents::mouseDown(int button, LCModifiers m, bool continuous)
{
    CGEventRef reference_event = CGEventCreate(NULL);
    CGPoint position = CGEventGetLocation(reference_event);

    if (button >= 0 && button < 3)
    {
        if (continuous)
        {
            mouseDownPosition_[button][0] = -1;
            mouseDownPosition_[button][1] = -1;
        }
        else
        {
            mouseDownPosition_[button][0] = (int) position.x;
            mouseDownPosition_[button][1] = (int) position.y;
        }
    }

    if (0 == button && !mouseButton1Down_)
    {
        impl_->increaseEventNumber();
        impl_->modifierEvent(m, true);
        impl_->mouseEvent(kCGEventLeftMouseDown, position, 0, 0, kCGMouseButtonLeft, m);
        mouseButton1Down_ = true;
    }
    if (1 == button && !mouseButton2Down_)
    {
        impl_->increaseEventNumber();
        impl_->modifierEvent(m, true);
        impl_->mouseEvent(kCGEventRightMouseDown, position, 0, 0, kCGMouseButtonRight, m);
        mouseButton2Down_ = true;
    }
    if (2 == button && !mouseButton3Down_)
    {
        impl_->increaseEventNumber();
        impl_->modifierEvent(m, true);
        impl_->mouseEvent(kCGEventOtherMouseDown, position, 0, 0, kCGMouseButtonCenter, m);
        mouseButton3Down_ = true;
    }
}

void NativeSystemEvents::mouseUp(int button, LCModifiers m)
{
    CGEventRef reference_event = CGEventCreate(NULL);
    CGPoint position = CGEventGetLocation(reference_event);

    if (button >= 0 && button < 3)
    {
        if (mouseDownPosition_[button][0] != -1 &&
            mouseDownPosition_[button][1] != -1)
        {
            position = CGPointMake(mouseDownPosition_[button][0], mouseDownPosition_[button][1]);
        }
        
        mouseDownPosition_[button][0] = -1;
        mouseDownPosition_[button][1] = -1;
    }
    
    if (0 == button && mouseButton1Down_)
    {
        impl_->mouseEvent(kCGEventLeftMouseUp, position, 0, 0, kCGMouseButtonLeft, m);
        impl_->modifierEvent(m, false);
        mouseButton1Down_ = false;
    }
    if (1 == button && mouseButton2Down_)
    {
        impl_->mouseEvent(kCGEventRightMouseUp, position, 0, 0, kCGMouseButtonRight, m);
        impl_->modifierEvent(m, false);
        mouseButton2Down_ = false;
    }
    if (2 == button && mouseButton3Down_)
    {
        impl_->mouseEvent(kCGEventOtherMouseUp, position, 0, 0, kCGMouseButtonCenter, m);
        impl_->modifierEvent(m, false);
        mouseButton3Down_ = false;
    }
}

void NativeSystemEvents::mouseDoubleClick(LCModifiers m)
{
    if (!mouseButton1Down_)
    {
        impl_->increaseEventNumber();
        impl_->modifierEvent(m, true);
        impl_->mouseDoubleClick(m);
        impl_->modifierEvent(m, false);
    }
}

void NativeSystemEvents::moveScrollWheel(float dx, float dy, LCModifiers m)
{
    impl_->modifierEvent(m, true);
    impl_->scrollwheel(dx, dy, m);
    impl_->modifierEvent(m, false);
}

void NativeSystemEvents::resetOffset()
{
    CGEventRef reference_event = CGEventCreate(NULL);
    CGPoint position = CGEventGetLocation(reference_event);
    MessageManagerLock lock;
    Point<int> pos((int) position.x, (int) position.y);
    Rectangle<int> area = getTotalDisplayArea(pos);
    offsetX_ = (int) (position.x + area.getX() - area.getWidth()/2);
    offsetY_ = (int) (position.y + area.getY() - area.getHeight()/2);
}

void NativeSystemEvents::clearOffset()
{
    offsetX_ = 0;
    offsetY_ = 0;
}

void NativeSystemEvents::prepareNotificationHandle(void *handle)
{
    NSView *view = (NSView *)handle;
    NSWindow *window = [view window];
    [window setLevel:CGShieldingWindowLevel()];
    [window setCollectionBehavior:NSWindowCollectionBehaviorStationary|NSWindowCollectionBehaviorCanJoinAllSpaces|NSWindowCollectionBehaviorFullScreenAuxiliary];
    [window setIgnoresMouseEvents:YES];
    [window orderFront:view];
    [window orderFrontRegardless];
    [window makeKeyAndOrderFront:view];
}
