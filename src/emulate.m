#include "emulate.h"
#include "hotkey.h"

void SendKeySequence(const char *Sequence)
{
    CFStringRef SequenceRef = CFStringCreateWithCString(NULL, Sequence, kCFStringEncodingUTF8);
    CFIndex SequenceLength = CFStringGetLength(SequenceRef);

    CGEventRef KeyDownEvent = CGEventCreateKeyboardEvent(NULL, 0, true);
    CGEventRef KeyUpEvent = CGEventCreateKeyboardEvent(NULL, 0, false);

    UniChar Character;
    for(CFIndex Index = 0;
        Index < SequenceLength;
        ++Index)
    {
        Character = CFStringGetCharacterAtIndex(SequenceRef, Index);

        CGEventSetFlags(KeyDownEvent, 0);
        CGEventKeyboardSetUnicodeString(KeyDownEvent, 1, &Character);
        CGEventPost(kCGHIDEventTap, KeyDownEvent);

        usleep(100);

        CGEventSetFlags(KeyUpEvent, 0);
        CGEventKeyboardSetUnicodeString(KeyUpEvent, 1, &Character);
        CGEventPost(kCGHIDEventTap, KeyUpEvent);
    }

    CFRelease(KeyUpEvent);
    CFRelease(KeyDownEvent);
    CFRelease(SequenceRef);
}

internal inline CGEventFlags
ClearModifierFlags(CGEventFlags Flags)
{
    Flags &= ~Event_Mask_Alt;
    Flags &= ~Event_Mask_LAlt;
    Flags &= ~Event_Mask_RAlt;

    Flags &= ~Event_Mask_Shift;
    Flags &= ~Event_Mask_LShift;
    Flags &= ~Event_Mask_RShift;

    Flags &= ~Event_Mask_Cmd;
    Flags &= ~Event_Mask_LCmd;
    Flags &= ~Event_Mask_RCmd;

    Flags &= ~Event_Mask_Control;
    Flags &= ~Event_Mask_LControl;
    Flags &= ~Event_Mask_RControl;

    return Flags;
}

internal inline void
CreateAndPostSystemKeyEvent(CGEventFlags Flags, CGKeyCode Key, bool Pressed)
{
    int32_t KeyState = Pressed ?  NX_KEYDOWN : NX_KEYUP;
    NSEvent *KeyEvent = [NSEvent otherEventWithType:NSEventTypeSystemDefined
                                           location:NSPointFromCGPoint(CGPointZero)
                                      modifierFlags:KeyState
                                          timestamp:0
                                       windowNumber:0
                                            context:0
                                            subtype:8
                                              data1:((Key << 16) | (KeyState << 8))
                                              data2:-1];


    CGEventPost(kCGHIDEventTap, [KeyEvent CGEvent]);
    usleep(7000);
}

internal inline void
CreateAndPostKeyEvent(CGEventFlags Flags, CGKeyCode Key, bool Pressed)
{
    CGEventRef KeyEvent = CGEventCreateKeyboardEvent(NULL, Key, Pressed);

    Flags |= ClearModifierFlags(CGEventGetFlags(KeyEvent));
    CGEventSetFlags(KeyEvent, Flags);
    CGEventPost(kCGHIDEventTap, KeyEvent);

    CFRelease(KeyEvent);
    usleep(7000);
}

void SendKeyPress(char *KeySym)
{
    struct hotkey Hotkey = {};
    ParseKeySymEmit(KeySym, &Hotkey);
    CGEventFlags Flags = CreateCGEventFlagsFromHotkeyFlags(Hotkey.Flags);

    if(IsSystemDefinedKeycode(Hotkey.Value))
    {
        CreateAndPostSystemKeyEvent(Flags, Hotkey.Value, true);
        CreateAndPostSystemKeyEvent(Flags, Hotkey.Value, false);
    }
    else
    {
        CreateAndPostKeyEvent(Flags, Hotkey.Value, true);
        CreateAndPostKeyEvent(Flags, Hotkey.Value, false);
    }
}
