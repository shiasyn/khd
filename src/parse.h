#ifndef KHD_PARSE_H
#define KHD_PARSE_H

char *ReadFile(const char *File);
void ParseConfig(char *Contents);
void ParseKhdEmit(char *Contents, int SockFD);

struct hotkey;
void ParseKeySymEmit(char *KeySym, struct hotkey *Hotkey);

#endif
