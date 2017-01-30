#ifndef KHD_LOCALE_H
#define KHD_LOCALE_H

struct hotkey;
bool KeycodeFromChar(char Key, struct hotkey *Hotkey);
bool OtherMouseButtonFromString(char *Temp, struct hotkey *Hotkey);
bool LayoutIndependentKeycode(char *Key, struct hotkey *Hotkey);
bool StringsAreEqual(const char *A, const char *B);
bool StringPrefix(const char *String, const char *Prefix);

#endif
