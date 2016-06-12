#include "LCModifiers.h"

using namespace linncontrol;

LCModifiers::LCModifiers() : flags_(0)
{
}

LCModifiers::LCModifiers(const int flags) : flags_ (flags)
{
}

LCModifiers::LCModifiers(const LCModifiers &other) : flags_(other.flags_)
{
}

LCModifiers &LCModifiers::operator=(const LCModifiers other)
{
    flags_ = other.flags_;
    return *this;
}

bool LCModifiers::operator==(const LCModifiers &other) const
{
    return flags_ == other.flags_;
}

bool LCModifiers::operator!=(const LCModifiers &other) const
{
    return flags_ != other.flags_;
}

bool LCModifiers::operator<(const LCModifiers &other) const
{
    return flags_ < other.flags_;
}

bool LCModifiers::operator>(const LCModifiers &other) const
{
    return flags_ > other.flags_;
}

int LCModifiers::getRawFlags() const
{
    return flags_;
}

LCModifiers LCModifiers::withoutFlags(int rawFlagsToClear) const
{
    return LCModifiers(flags_ & ~rawFlagsToClear);
}

LCModifiers LCModifiers::withFlags(int rawFlagsToSet) const
{
    return LCModifiers(flags_ | rawFlagsToSet);
}