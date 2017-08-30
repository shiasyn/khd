#include "locale.h"
#include "hotkey.h"

#include <Carbon/Carbon.h>
#include <IOKit/hidsystem/ev_keymap.h>

#define internal static
#define local_persist static

internal CFStringRef
CFStringFromKeycode(CGKeyCode Keycode)
{
    TISInputSourceRef Keyboard = TISCopyCurrentASCIICapableKeyboardLayoutInputSource();
    CFDataRef Uchr = (CFDataRef) TISGetInputSourceProperty(Keyboard, kTISPropertyUnicodeKeyLayoutData);
    CFRelease(Keyboard);

    UCKeyboardLayout *KeyboardLayout = (UCKeyboardLayout *) CFDataGetBytePtr(Uchr);
    if(KeyboardLayout)
    {
        UInt32 DeadKeyState = 0;
        UniCharCount MaxStringLength = 255;
        UniCharCount StringLength = 0;
        UniChar UnicodeString[MaxStringLength];

        OSStatus Status = UCKeyTranslate(KeyboardLayout, Keycode,
                                         kUCKeyActionDown, 0,
                                         LMGetKbdType(), 0,
                                         &DeadKeyState,
                                         MaxStringLength,
                                         &StringLength,
                                         UnicodeString);

        if(StringLength == 0 && DeadKeyState)
        {
            Status = UCKeyTranslate(KeyboardLayout, kVK_Space,
                                    kUCKeyActionDown, 0,
                                    LMGetKbdType(), 0,
                                    &DeadKeyState,
                                    MaxStringLength,
                                    &StringLength,
                                    UnicodeString);
        }

        if(StringLength > 0 && Status == noErr)
            return CFStringCreateWithCharacters(NULL, UnicodeString, StringLength);
    }

    return NULL;
}

bool KeycodeFromChar(char Key, struct hotkey *Hotkey)
{
    local_persist CFMutableDictionaryRef CharToKeycode = NULL;

    if(!CharToKeycode)
    {
        CharToKeycode = CFDictionaryCreateMutable(kCFAllocatorDefault, 128,
                                                  &kCFCopyStringDictionaryKeyCallBacks, NULL);
        if(!CharToKeycode)
            return false;

        for(size_t KeyIndex = 0; KeyIndex < 128; ++KeyIndex)
        {
            CFStringRef KeyString = CFStringFromKeycode(KeyIndex);
            if(KeyString)
            {
                CFDictionaryAddValue(CharToKeycode, KeyString, (const void *)KeyIndex);
                CFRelease(KeyString);
            }
        }
    }

    bool Result = true;
    UniChar Character = Key;
    CFStringRef CharStr = CFStringCreateWithCharacters(kCFAllocatorDefault, &Character, 1);
    if(!CFDictionaryGetValueIfPresent(CharToKeycode, CharStr, (const void **)&Hotkey->Value))
        Result = false;

    CFRelease(CharStr);
    return Result;
}

// NOTE(koekeishiya): The value 0, 1 and 2 is mapped to the left, right and middle mouse-button.
bool OtherMouseButtonFromString(char *Temp, struct hotkey *Hotkey)
{
    bool Result = false;

    uint32_t Button = strtoul(Temp, 0, 0);
    if(Button >= 3)
    {
        AddFlags(Hotkey, Hotkey_Flag_MouseButton);
        Hotkey->Value = Button;
        Result = true;
    }

    return Result;
}

bool LayoutIndependentKeycode(char *Key, struct hotkey *Hotkey)
{
    bool Result = true;

    if(StringsAreEqual(Key, "return"))               { Hotkey->Value = kVK_Return; }
    else if(StringsAreEqual(Key, "tab"))             { Hotkey->Value = kVK_Tab; }
    else if(StringsAreEqual(Key, "space"))           { Hotkey->Value = kVK_Space; }
    else if(StringsAreEqual(Key, "backspace"))       { Hotkey->Value = kVK_Delete; }
    else if(StringsAreEqual(Key, "capslock"))        { Hotkey->Value = Modifier_Keycode_CapsLock; }
    else if(StringsAreEqual(Key, "delete"))          { Hotkey->Value = kVK_ForwardDelete; }
    else if(StringsAreEqual(Key, "escape"))          { Hotkey->Value =  kVK_Escape; }
    else if(StringsAreEqual(Key, "home"))            { Hotkey->Value =  kVK_Home; }
    else if(StringsAreEqual(Key, "end"))             { Hotkey->Value =  kVK_End; }
    else if(StringsAreEqual(Key, "pageup"))          { Hotkey->Value =  kVK_PageUp; }
    else if(StringsAreEqual(Key, "pagedown"))        { Hotkey->Value =  kVK_PageDown; }
    else if(StringsAreEqual(Key, "help"))            { Hotkey->Value =  kVK_Help; }
    else if(StringsAreEqual(Key, "left"))            { Hotkey->Value =  kVK_LeftArrow; }
    else if(StringsAreEqual(Key, "right"))           { Hotkey->Value =  kVK_RightArrow; }
    else if(StringsAreEqual(Key, "up"))              { Hotkey->Value = kVK_UpArrow; }
    else if(StringsAreEqual(Key, "down"))            { Hotkey->Value = kVK_DownArrow; }
    else if(StringsAreEqual(Key, "f1"))              { Hotkey->Value = kVK_F1; }
    else if(StringsAreEqual(Key, "f2"))              { Hotkey->Value = kVK_F2; }
    else if(StringsAreEqual(Key, "f3"))              { Hotkey->Value = kVK_F3; }
    else if(StringsAreEqual(Key, "f4"))              { Hotkey->Value = kVK_F4; }
    else if(StringsAreEqual(Key, "f5"))              { Hotkey->Value = kVK_F5; }
    else if(StringsAreEqual(Key, "f6"))              { Hotkey->Value = kVK_F6; }
    else if(StringsAreEqual(Key, "f7"))              { Hotkey->Value = kVK_F7; }
    else if(StringsAreEqual(Key, "f8"))              { Hotkey->Value = kVK_F8; }
    else if(StringsAreEqual(Key, "f9"))              { Hotkey->Value = kVK_F9; }
    else if(StringsAreEqual(Key, "f10"))             { Hotkey->Value = kVK_F10; }
    else if(StringsAreEqual(Key, "f11"))             { Hotkey->Value = kVK_F11; }
    else if(StringsAreEqual(Key, "f12"))             { Hotkey->Value = kVK_F12; }
    else if(StringsAreEqual(Key, "f13"))             { Hotkey->Value = kVK_F13; }
    else if(StringsAreEqual(Key, "f14"))             { Hotkey->Value = kVK_F14; }
    else if(StringsAreEqual(Key, "f15"))             { Hotkey->Value = kVK_F15; }
    else if(StringsAreEqual(Key, "f16"))             { Hotkey->Value = kVK_F16; }
    else if(StringsAreEqual(Key, "f17"))             { Hotkey->Value = kVK_F17; }
    else if(StringsAreEqual(Key, "f18"))             { Hotkey->Value = kVK_F18; }
    else if(StringsAreEqual(Key, "f19"))             { Hotkey->Value = kVK_F19; }
    else if(StringsAreEqual(Key, "f20"))             { Hotkey->Value = kVK_F20; }
    else if(StringsAreEqual(Key, "volume_up"))       { Hotkey->Value = NX_KEYTYPE_SOUND_UP; AddFlags(Hotkey, Hotkey_Flag_SystemDefined); }
    else if(StringsAreEqual(Key, "volume_down"))     { Hotkey->Value = NX_KEYTYPE_SOUND_DOWN; AddFlags(Hotkey, Hotkey_Flag_SystemDefined); }
    else if(StringsAreEqual(Key, "mute"))            { Hotkey->Value = NX_KEYTYPE_MUTE; AddFlags(Hotkey, Hotkey_Flag_SystemDefined); }
    else if(StringsAreEqual(Key, "play"))            { Hotkey->Value = NX_KEYTYPE_PLAY; AddFlags(Hotkey, Hotkey_Flag_SystemDefined); }
    else if(StringsAreEqual(Key, "previous"))        { Hotkey->Value = NX_KEYTYPE_PREVIOUS; AddFlags(Hotkey, Hotkey_Flag_SystemDefined); }
    else if(StringsAreEqual(Key, "next"))            { Hotkey->Value = NX_KEYTYPE_NEXT; AddFlags(Hotkey, Hotkey_Flag_SystemDefined); }
    else if(StringsAreEqual(Key, "brightness_up"))   { Hotkey->Value = NX_KEYTYPE_BRIGHTNESS_UP; AddFlags(Hotkey, Hotkey_Flag_SystemDefined); }
    else if(StringsAreEqual(Key, "brightness_down")) { Hotkey->Value = NX_KEYTYPE_BRIGHTNESS_DOWN; AddFlags(Hotkey, Hotkey_Flag_SystemDefined); }
    else                                             { Result = false; }

    return Result;
}

bool StringsAreEqual(const char *A, const char *B)
{
    return strcmp(A, B) == 0;
}

bool StringPrefix(const char *String, const char *Prefix)
{
    size_t StringLength = strlen(String);
    size_t PrefixLength = strlen(Prefix);

    bool Result = false;
    if(StringLength >= PrefixLength)
    {
        Result = (strncmp(String, Prefix, PrefixLength) == 0);
    }

    return Result;
}
