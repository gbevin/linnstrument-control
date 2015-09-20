#ifndef __linncontrol__Modifiers__
#define __linncontrol__Modifiers__

namespace linncontrol
{
    class LCModifiers
    {
    public:
        
        LCModifiers();
        LCModifiers(int flags);
        LCModifiers(const LCModifiers &other);
        LCModifiers &operator=(const LCModifiers other);
        
        bool operator==(const LCModifiers &other) const;
        bool operator!=(const LCModifiers &other) const;
        bool operator<(const LCModifiers &other) const;
        bool operator>(const LCModifiers &other) const;
        
        int getRawFlags() const;
        
        LCModifiers withoutFlags(int rawFlagsToClear) const;
        LCModifiers withFlags(int rawFlagsToSet) const;
        
        inline bool testFlags(const int flagsToTest) const { return (flags_ & flagsToTest) != 0; }

        inline bool isAnyModifierKeyDown() const   { return testFlags((shiftModifier | ctrlModifier | altModifier |  windowsModifier)); }
        inline bool isShiftDown() const            { return testFlags(shiftModifier); }
        inline bool isCtrlDown() const             { return testFlags(ctrlModifier); }
        inline bool isAltDown() const              { return testFlags(altModifier); }
        inline bool isWindowsDown() const          { return testFlags(windowsModifier); }
        
        enum Flags
        {
            noModifiers      = 0,
            shiftModifier    = 1,
            ctrlModifier     = 2,
            altModifier      = 4,
            windowsModifier  = 8
        };
        
    private:
        
        volatile int flags_;
    };
}
#endif
