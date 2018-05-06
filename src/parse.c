#include "parse.h"
#include "tokenize.h"
#include "locale.h"
#include "hotkey.h"
#include "daemon.h"
#include "misc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define internal static
#define MOUSE_BUTTON_PREFIX "button"
#define MOUSE_BUTTON_PREFIX_LENGTH (sizeof(MOUSE_BUTTON_PREFIX) - 1)

extern struct modifier_state ModifierState;
extern struct mode DefaultBindingMode;
extern struct mode *ActiveBindingMode;
extern char *ConfigFile;
extern uint32_t ConfigFlags;

internal inline unsigned
HexToInt(char *Hex)
{
    uint32_t Result;
    sscanf(Hex, "%x", &Result);
    return Result;
}

internal inline double
StringToDouble(char *String)
{
    double Result;
    sscanf(String, "%lf", &Result);
    return Result;
}

internal char *
AllocAndCopyString(char *Text, int Length)
{
    char *Result = (char *) malloc(Length + 1);
    strncpy(Result, Text, Length);
    Result[Length] = '\0';

    return Result;
}

internal char **
AllocAndCopyList(char *Text, int Length)
{
    int Count = 1;
    for(int Index = 0; Index < Length; ++Index)
    {
        if(Text[Index] == ',')
            ++Count;
    }

    char *Temp = AllocAndCopyString(Text, Length);
    char **Result = (char **) malloc(Count * sizeof(char *) + 1);
    Result[Count] = NULL;

    char *Token = strtok(Temp, ",");
    while(Token)
    {
        while(isspace(*Token))
            ++Token;

        char *End = Token + strlen(Token) - 1;
        while(End > Token && isspace(*End))
            --End;

        *(End + 1) = '\0';
        Result[--Count] = strdup(Token);
        Token = strtok(NULL, ",");
    }

    free(Temp);
    return Result;
}

internal inline struct hotkey *
AllocHotkey()
{
    struct hotkey *Hotkey = (struct hotkey *) malloc(sizeof(struct hotkey));
    memset(Hotkey, 0, sizeof(struct hotkey));
    Hotkey->Mode = strdup("default");

    return Hotkey;
}

internal inline void
AppendHotkey(struct hotkey *Hotkey)
{
    struct mode *BindingMode = GetBindingMode(Hotkey->Mode);
    if(!BindingMode)
    {
        BindingMode = CreateBindingMode(Hotkey->Mode);
    }

    if(BindingMode->Hotkey)
    {
        struct hotkey *Last = BindingMode->Hotkey;
        while(Last->Next)
            Last = Last->Next;

        Last->Next = Hotkey;
    }
    else
    {
        BindingMode->Hotkey = Hotkey;
    }
}

internal inline void
DestroyHotkey(struct hotkey *Hotkey)
{
    if(Hotkey->Next)
        DestroyHotkey(Hotkey->Next);

    if(Hotkey->Mode)
        free(Hotkey->Mode);

    if(Hotkey->Command)
        free(Hotkey->Command);

    if(Hotkey->App)
    {
        char **App = Hotkey->App;
        while(*App)
        {
            free(*App++);
        }

        free(Hotkey->App);
    }

    free(Hotkey);
}

internal inline void
DestroyBindingMode(struct mode *BindingMode)
{
    if(BindingMode->Next)
        DestroyBindingMode(BindingMode->Next);

    if(BindingMode->Hotkey)
        DestroyHotkey(BindingMode->Hotkey);

    if(BindingMode->Name)
        free(BindingMode->Name);

    if(BindingMode->OnEnterCommand)
        free(BindingMode->OnEnterCommand);

    if(BindingMode->Restore)
        free(BindingMode->Restore);

    free(BindingMode);
}

internal void
ReloadConfig()
{
    char *Contents = ReadFile(ConfigFile);
    if(Contents)
    {
        printf("Khd: Reloading config '%s'\n", ConfigFile);
        if(DefaultBindingMode.Next)
        {
            DestroyBindingMode(DefaultBindingMode.Next);
            DefaultBindingMode.Next = NULL;
        }

        if(DefaultBindingMode.Hotkey)
        {
            DestroyHotkey(DefaultBindingMode.Hotkey);
            DefaultBindingMode.Hotkey = NULL;
        }

        if(DefaultBindingMode.OnEnterCommand)
        {
            free(DefaultBindingMode.OnEnterCommand);
            DefaultBindingMode.OnEnterCommand = NULL;
        }

        if(DefaultBindingMode.Restore)
        {
            free(DefaultBindingMode.Restore);
            DefaultBindingMode.Restore = NULL;
        }

        ParseConfig(Contents);
        ActivateMode("default");
    }
}

internal void
StripTrailingWhiteSpace(struct token *Token)
{
    while(IsWhiteSpace(Token->Text[Token->Length-1]))
        --Token->Length;
}

internal void
ParseCommand(struct tokenizer *Tokenizer, struct hotkey *Hotkey)
{
    struct token Command = GetToken(Tokenizer);
    switch(Command.Type)
    {
        case Token_Passthrough:
        {
            AddFlags(Hotkey, Hotkey_Flag_Passthrough);
            printf("Token_Passthrough: %.*s\n", Command.Length, Command.Text);
            ParseCommand(Tokenizer, Hotkey);
        } break;
        case Token_Negate:
        {
            Hotkey->Type = Hotkey_Exclude;
            printf("Token_Negate: %.*s\n", Command.Length, Command.Text);
            ParseCommand(Tokenizer, Hotkey);
        } break;
        case Token_List:
        {
            printf("Token_List: [\n");
            Hotkey->App = AllocAndCopyList(Command.Text, Command.Length);
            if(Hotkey->Type == Hotkey_Default)
                Hotkey->Type = Hotkey_Include;

            char **List = Hotkey->App;
            while(*List)
            {
                printf("    %s\n", *List++);
            }
            printf("]\n");

            ParseCommand(Tokenizer, Hotkey);
        } break;
        case Token_Command:
        {
            StripTrailingWhiteSpace(&Command);
            Hotkey->Command = AllocAndCopyString(Command.Text, Command.Length);
            AppendHotkey(Hotkey);
            printf("Token_Command: %s\n", Hotkey->Command);
        } break;
        default:
        {
            Error("Line#%d: Expected token of type 'Token_Command' after symbols!\n", Tokenizer->Line);
        } break;
    }
}

internal inline void
SetHotkeyMode(struct hotkey *Hotkey, char *Modifier)
{
    if(Hotkey->Mode)
        free(Hotkey->Mode);

    Hotkey->Mode = strdup(Modifier);
}

internal void
AddHotkeyModifier(char *Mod, int Length, struct hotkey *Hotkey)
{
    char *Modifier = AllocAndCopyString(Mod, Length);
    printf("Token_Modifier: %s\n", Modifier);

    if     (StringsAreEqual(Modifier, "cmd"))    AddFlags(Hotkey, Hotkey_Flag_Cmd);
    else if(StringsAreEqual(Modifier, "lcmd"))   AddFlags(Hotkey, Hotkey_Flag_LCmd);
    else if(StringsAreEqual(Modifier, "rcmd"))   AddFlags(Hotkey, Hotkey_Flag_RCmd);
    else if(StringsAreEqual(Modifier, "alt"))    AddFlags(Hotkey, Hotkey_Flag_Alt);
    else if(StringsAreEqual(Modifier, "lalt"))   AddFlags(Hotkey, Hotkey_Flag_LAlt);
    else if(StringsAreEqual(Modifier, "ralt"))   AddFlags(Hotkey, Hotkey_Flag_RAlt);
    else if(StringsAreEqual(Modifier, "shift"))  AddFlags(Hotkey, Hotkey_Flag_Shift);
    else if(StringsAreEqual(Modifier, "lshift")) AddFlags(Hotkey, Hotkey_Flag_LShift);
    else if(StringsAreEqual(Modifier, "rshift")) AddFlags(Hotkey, Hotkey_Flag_RShift);
    else if(StringsAreEqual(Modifier, "ctrl"))   AddFlags(Hotkey, Hotkey_Flag_Control);
    else if(StringsAreEqual(Modifier, "lctrl"))  AddFlags(Hotkey, Hotkey_Flag_LControl);
    else if(StringsAreEqual(Modifier, "rctrl"))  AddFlags(Hotkey, Hotkey_Flag_RControl);
    else if(StringsAreEqual(Modifier, "fn"))     AddFlags(Hotkey, Hotkey_Flag_Fn);
    else                                         SetHotkeyMode(Hotkey, Modifier);

    free(Modifier);
}

internal void
ParseKeyHexadecimal(struct tokenizer *Tokenizer, struct token *Token,
                    struct hotkey *Hotkey, bool ExpectCommand)
{
    char *Temp = AllocAndCopyString(Token->Text, Token->Length);
    Hotkey->Value = HexToInt(Temp);
    free(Temp);

    if(ExpectCommand)
    {
        AddFlags(Hotkey, Hotkey_Flag_Literal);
        ParseCommand(Tokenizer, Hotkey);
    }
}

internal void
ParseKeyLiteral(struct tokenizer *Tokenizer, struct token *Token,
                struct hotkey *Hotkey, bool ExpectCommand)
{
    char *Temp = AllocAndCopyString(Token->Text, Token->Length);
    bool Result = false;

    if(Token->Length > 1)
    {
        if(StringPrefix(Temp, MOUSE_BUTTON_PREFIX))
        {
            Result = OtherMouseButtonFromString(Temp + MOUSE_BUTTON_PREFIX_LENGTH, Hotkey);
        }
        else
        {
            Result = LayoutIndependentKeycode(Temp, Hotkey);
        }
    }
    else
    {
        Result = KeycodeFromChar(Temp[0], Hotkey);
    }

    if(Result && ExpectCommand)
    {
        AddFlags(Hotkey, Hotkey_Flag_Literal);
        ParseCommand(Tokenizer, Hotkey);
    }
    else if(!Result)
    {
        Error("Line#%d: Invalid format for key literal: %.*s\n", Tokenizer->Line, Token->Length, Token->Text);
    }

    free(Temp);
}

internal void
ParseKeySym(struct tokenizer *Tokenizer, struct token *Token,
            struct hotkey *Hotkey, bool ExpectCommand)
{
    AddHotkeyModifier(Token->Text, Token->Length, Hotkey);

    char *At = Tokenizer->At;
    struct token Symbol = GetToken(Tokenizer);
    switch(Symbol.Type)
    {
        case Token_Plus:
        {
            struct token Symbol = GetToken(Tokenizer);
            ParseKeySym(Tokenizer, &Symbol, Hotkey, ExpectCommand);
        } break;
        case Token_Hex:
        {
            printf("Token_Hex: %.*s\n", Symbol.Length, Symbol.Text);
            ParseKeyHexadecimal(Tokenizer, &Symbol, Hotkey, ExpectCommand);
        } break;
        case Token_Literal:
        {
            printf("Token_Literal: %.*s\n", Symbol.Length, Symbol.Text);
            ParseKeyLiteral(Tokenizer, &Symbol, Hotkey, ExpectCommand);
        } break;
        default:
        {
            if(ExpectCommand)
            {
                Tokenizer->At = At;
                ParseCommand(Tokenizer, Hotkey);
            }
            else
            {
                Error("Line#%d: Invalid format for keysym: %.*s\n", Tokenizer->Line, Symbol.Length, Symbol.Text);
            }
        } break;
    }
}

internal void
ParseKhdModeActivate(struct tokenizer *Tokenizer)
{
    struct token TokenMode = GetToken(Tokenizer);
    char *Mode = AllocAndCopyString(TokenMode.Text, TokenMode.Length);
    ActivateMode(Mode);

    free(Mode);
}

internal void
ParseKhdModeProperties(struct token *TokenMode, struct tokenizer *Tokenizer)
{
    char *Mode = AllocAndCopyString(TokenMode->Text, TokenMode->Length);
    struct mode *BindingMode = GetBindingMode(Mode);
    if(!BindingMode)
        BindingMode = CreateBindingMode(Mode);

    struct token Token = GetToken(Tokenizer);
    if(TokenEquals(Token, "prefix"))
    {
        struct token Token = GetToken(Tokenizer);
        if(TokenEquals(Token, "on"))
        {
            BindingMode->Prefix = true;
        }
        else if(TokenEquals(Token, "off"))
        {
            BindingMode->Prefix = false;
        }

        printf("Prefix State: %d\n", BindingMode->Prefix);
    }
    else if(TokenEquals(Token, "timeout"))
    {
        struct token Token = GetToken(Tokenizer);
        switch(Token.Type)
        {
            case Token_Digit:
            {
                char *Temp = AllocAndCopyString(Token.Text, Token.Length);
                BindingMode->Timeout = StringToDouble(Temp);
                printf("Prefix Timeout: %f\n", BindingMode->Timeout);
                free(Temp);
            } break;
            default:
            {
                Error("Line#%d: Expected token of type 'Token_Digit': %.*s\n", Tokenizer->Line, Token.Length, Token.Text);
            };
        }
    }
    else if(TokenEquals(Token, "on_enter"))
    {
        struct token Token = ReadTilEndOfLine(Tokenizer);
        BindingMode->OnEnterCommand = AllocAndCopyString(Token.Text, Token.Length);
        printf("Prefix OnEnterCommand: %s\n", BindingMode->OnEnterCommand);
    }
    else if(TokenEquals(Token, "restore"))
    {
        struct token Token = GetToken(Tokenizer);
        BindingMode->Restore = AllocAndCopyString(Token.Text, Token.Length);
        printf("Prefix Restore: %s\n", BindingMode->Restore);
    }

    free(Mode);
}

internal void
ParseKhdVoidUnlistedBind(struct tokenizer *Tokenizer)
{
    struct token Token = GetToken(Tokenizer);
    if(TokenEquals(Token, "on"))
    {
        ConfigFlags |= Config_Void_Bind;
    }
    else if(TokenEquals(Token, "off"))
    {
        ConfigFlags &= ~Config_Void_Bind;
    }
    else
    {
        Error("Line#%d: Unexpected token '%.*s'\n", Tokenizer->Line, Token.Length, Token.Text);
    }
}

internal void
ParseKhdModTriggerTimeout(struct tokenizer *Tokenizer)
{
    struct token Token = GetToken(Tokenizer);
    switch(Token.Type)
    {
        case Token_Digit:
        {
            char *Temp = AllocAndCopyString(Token.Text, Token.Length);
            ModifierState.Timeout = StringToDouble(Temp);
            free(Temp);
        } break;
        default:
        {
            Error("Line#%d: Unexpected token '%.*s'\n", Tokenizer->Line, Token.Length, Token.Text);
        } break;
    }
}

internal void
ParseKhdMode(struct tokenizer *Tokenizer)
{
    struct token Token = GetToken(Tokenizer);
    switch(Token.Type)
    {
        case Token_EndOfStream:
        {
            Error("Line#%d: Unexpected end of stream while parsing khd command!\n", Tokenizer->Line);
        } break;
        case Token_Identifier:
        {
            if(TokenEquals(Token, "activate"))
            {
                ParseKhdModeActivate(Tokenizer);
            }
            else
            {
                ParseKhdModeProperties(&Token, Tokenizer);
            }
        } break;
        default:
        {
            Error("Line#%d: Unexpected token '%.*s'\n", Tokenizer->Line, Token.Length, Token.Text);
        } break;
    }
}

internal void
ParseKhd(struct tokenizer *Tokenizer, int SockFD)
{
    struct token Token = GetToken(Tokenizer);
    switch(Token.Type)
    {
        case Token_EndOfStream:
        {
            Error("Line#%d: Unexpected end of stream while parsing khd command!\n", Tokenizer->Line);
        } break;
        case Token_Identifier:
        {
            if(TokenEquals(Token, "reload"))
            {
                ReloadConfig();
            }
            else if(TokenEquals(Token, "void_unlisted_bind"))
            {
                ParseKhdVoidUnlistedBind(Tokenizer);
            }
            else if(TokenEquals(Token, "mod_trigger_timeout"))
            {
                ParseKhdModTriggerTimeout(Tokenizer);
            }
            else if(TokenEquals(Token, "mode"))
            {
                ParseKhdMode(Tokenizer);
            }
            else if(TokenEquals(Token, "print"))
            {
                struct token Query = GetToken(Tokenizer);
                if(TokenEquals(Query, "mode"))
                {
                    if((ActiveBindingMode) &&
                       (ActiveBindingMode->Name))
                    {
                        WriteToSocket(ActiveBindingMode->Name, SockFD);
                    }
                    else
                    {
                        WriteToSocket("<Unknown Mode>", SockFD);
                    }
                }
            }
        } break;
        default:
        {
            Error("Line#%d: Unexpected token '%.*s'\n", Tokenizer->Line, Token.Length, Token.Text);
        } break;
    }
}

void ParseKhdEmit(char *Contents, int SockFD)
{
    struct tokenizer Tokenizer = { Contents };
    ParseKhd(&Tokenizer, SockFD);
}

void ParseKeySymEmit(char *KeySym, struct hotkey *Hotkey)
{
    struct tokenizer Tokenizer = { KeySym };
    struct token Token = GetToken(&Tokenizer);
    switch(Token.Type)
    {
        case Token_Hex: { ParseKeyHexadecimal(&Tokenizer, &Token, Hotkey, false); } break;
        case Token_Literal: { ParseKeyLiteral(&Tokenizer, &Token, Hotkey, false); } break;
        case Token_Identifier: { ParseKeySym(&Tokenizer, &Token, Hotkey, false); } break;
        default: { Error("Invalid format for keysym: %.*s\n", Token.Length, Token.Text); } break;
    }
}

void ParseConfig(char *Contents)
{
    struct tokenizer Tokenizer = { Contents, 0 };
    bool Parsing = true;
    while(Parsing)
    {
        struct token Token = GetToken(&Tokenizer);
        switch(Token.Type)
        {
            case Token_EndOfStream:
            {
                Parsing = false;
            } break;
            case Token_Comment:
            {
                printf("Token_Comment: %.*s\n", Token.Length, Token.Text);
            } break;
            case Token_Hex:
            {
                printf("Token_Hex: %.*s\n", Token.Length, Token.Text);
                ParseKeyHexadecimal(&Tokenizer, &Token, AllocHotkey(), true);
            } break;
            case Token_Literal:
            {
                printf("Token_Literal: %.*s\n", Token.Length, Token.Text);
                ParseKeyLiteral(&Tokenizer, &Token, AllocHotkey(), true);
            } break;
            case Token_Identifier:
            {
                if(TokenEquals(Token, "khd"))
                {
                    ParseKhd(&Tokenizer, 0);
                }
                else
                {
                    ParseKeySym(&Tokenizer, &Token, AllocHotkey(), true);
                }
            } break;
            default:
            {
                printf("%d: %.*s\n", Token.Type, Token.Length, Token.Text);
            } break;
        }
    }

    printf("Total lines#%d\n", Tokenizer.Line);
    free(Contents);
}

char *ReadFile(const char *File)
{
    char *Contents = NULL;
    FILE *Descriptor = fopen(File, "r");

    if(Descriptor)
    {
        fseek(Descriptor, 0, SEEK_END);
        unsigned Length = ftell(Descriptor);
        fseek(Descriptor, 0, SEEK_SET);

        Contents = (char *) malloc(Length + 1);
        fread(Contents, Length, 1, Descriptor);
        Contents[Length] = '\0';

        fclose(Descriptor);
    }

    return Contents;
}
