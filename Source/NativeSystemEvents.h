#ifndef __linncontrol_NativeSystemEvents__
#define __linncontrol_NativeSystemEvents__

#include "MediaKeys.h"
#include "SystemCommands.h"
#include "Modifiers.h"

namespace linncontrol
{
    class NativeSystemEvents
    {
    public:
        NativeSystemEvents();
        ~NativeSystemEvents();
        
        void keyDown(unsigned int, LCModifiers);
        void keyUp(unsigned int, LCModifiers);
        void capsLockDown(LCModifiers);
        void capsLockUp(LCModifiers);
        void modifierDown(LCModifiers);
        void modifierUp(LCModifiers);
        void mediaDown(MediaKey);
        void mediaUp(MediaKey);
		void appCommandDown(int);
		void appCommandUp(int);
        void systemCommand(SystemCommand);

        void moveMouse(int, int);
        void mouseDown(int, LCModifiers, bool);
        void mouseUp(int, LCModifiers);
        void mouseDoubleClick(LCModifiers);
        void moveScrollWheel(float, float, LCModifiers);
        
        void resetOffset();
        void clearOffset();
        
        void prepareNotificationHandle(void *);
        
        unsigned int getNativeCode(const char k);
        
        class impl_t;
        
    private:
        bool isMouseDownContinuous(int);
        
        bool mouseButton1Down_;
        bool mouseButton2Down_;
        bool mouseButton3Down_;
        int offsetX_;
        int offsetY_;
        int mouseDownPosition_[3][2];
        
        impl_t *impl_;
    };
}

#endif
