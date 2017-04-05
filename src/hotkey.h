#ifndef KHD_HOTKEY_H
#define KHD_HOTKEY_H

#include <stdint.h>
#include <Carbon/Carbon.h>

#define internal static

#define Modifier_Keycode_CapsLock     0x39
#define Modifier_Keycode_Left_Cmd     0x37
#define Modifier_Keycode_Right_Cmd    0x36
#define Modifier_Keycode_Left_Shift   0x38
#define Modifier_Keycode_Right_Shift  0x3C
#define Modifier_Keycode_Left_Alt     0x3A
#define Modifier_Keycode_Right_Alt    0x3D
#define Modifier_Keycode_Left_Ctrl    0x3B
#define Modifier_Keycode_Right_Ctrl   0x3E
#define Modifier_Keycode_Fn           0x3F

enum config_option
{
    Config_Kwm_Border = (1 << 0),
    Config_Void_Bind = (1 << 1),
};

enum osx_event_mask
{
    Event_Mask_CapsLock = 0x00010000,

    Event_Mask_Alt = 0x00080000,
    Event_Mask_LAlt = 0x00000020,
    Event_Mask_RAlt = 0x00000040,

    Event_Mask_Shift = 0x00020000,
    Event_Mask_LShift = 0x00000002,
    Event_Mask_RShift = 0x00000004,

    Event_Mask_Cmd = 0x00100000,
    Event_Mask_LCmd = 0x00000008,
    Event_Mask_RCmd = 0x00000010,

    Event_Mask_Control = 0x00040000,
    Event_Mask_LControl = 0x00000001,
    Event_Mask_RControl = 0x00002000,
};

enum hotkey_flag
{
    Hotkey_Flag_Alt = (1 << 0),
    Hotkey_Flag_LAlt = (1 << 1),
    Hotkey_Flag_RAlt = (1 << 2),

    Hotkey_Flag_Shift = (1 << 3),
    Hotkey_Flag_LShift = (1 << 4),
    Hotkey_Flag_RShift = (1 << 5),

    Hotkey_Flag_Cmd = (1 << 6),
    Hotkey_Flag_LCmd = (1 << 7),
    Hotkey_Flag_RCmd = (1 << 8),

    Hotkey_Flag_Control = (1 << 9),
    Hotkey_Flag_LControl = (1 << 10),
    Hotkey_Flag_RControl = (1 << 11),

    Hotkey_Flag_Passthrough = (1 << 12),
    Hotkey_Flag_Literal = (1 << 13),
    Hotkey_Flag_MouseButton = (1 << 14),
};

enum hotkey_type
{
    Hotkey_Default,
    Hotkey_Include,
    Hotkey_Exclude,
};

struct hotkey;

struct mode
{
    char *Name;
    char *Color;

    bool Prefix;
    double Timeout;
    char *Restore;
    long long Time;

    struct hotkey *Hotkey;
    struct mode *Next;
};

struct hotkey
{
    char *Mode;

    enum hotkey_type Type;
    uint32_t Flags;

    /* NOTE(koekeishiya):
     * CGKeyCode        -> typedef uint16_t
     * CGMouseButton    -> typedef uint32_t
     * */
    uint32_t Value;

    char *Command;
    char **App;

    struct hotkey *Next;
};

struct modifier_state
{
    uint32_t Flags;

    long long Time;
    double Timeout;
    bool Valid;
};

internal inline void
AddFlags(struct hotkey *Hotkey, uint32_t Flag)
{
    Hotkey->Flags |= Flag;
}

internal inline bool
HasFlags(struct hotkey *Hotkey, uint32_t Flag)
{
    bool Result = Hotkey->Flags & Flag;
    return Result;
}

internal inline void
ClearFlags(struct hotkey *Hotkey, uint32_t Flag)
{
    Hotkey->Flags &= ~Flag;
}

struct hotkey CreateHotkeyFromCGEvent(CGEventFlags Flags, uint32_t Value);
bool FindAndExecuteHotkey(struct hotkey *Eventkey);
void RefreshModifierState(CGEventFlags Flags, CGKeyCode Key);

struct mode *CreateBindingMode(const char *Mode);
struct mode *GetBindingMode(const char *Mode);
struct mode *GetLastBindingMode();
void ActivateMode(const char *Mode);

#endif
