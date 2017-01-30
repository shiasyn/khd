#ifndef KHD_TOKENIZE_H
#define KHD_TOKENIZE_H

#define internal static

enum token_type
{
    Token_Identifier,
    Token_Command,
    Token_Literal,
    Token_List,
    Token_Negate,
    Token_Passthrough,

    Token_Plus,
    Token_OpenBrace,
    Token_CloseBrace,

    Token_Hex,
    Token_Digit,
    Token_Comment,

    Token_Unknown,
    Token_EndOfStream,
};

struct token
{
    char *Text;
    unsigned Length;
    enum token_type Type;
};

struct tokenizer
{
    char *At;
    unsigned Line;
};

internal inline bool
IsDot(char C)
{
    bool Result = ((C == '.') ||
                   (C == ','));
    return Result;
}

internal inline bool
IsEndOfLine(char C)
{
    bool Result = ((C == '\n') ||
                   (C == '\r'));

    return Result;
}

internal inline bool
IsWhiteSpace(char C)
{
    bool Result = ((C == ' ') ||
                   (C == '\t') ||
                   IsEndOfLine(C));

    return Result;
}

internal inline bool
IsAlpha(char C)
{
    bool Result = (((C >= 'a') && (C <= 'z')) ||
                   ((C >= 'A') && (C <= 'Z')) ||
                   ((C == '_')));

    return Result;
}

internal inline bool
IsNumeric(char C)
{
    bool Result = ((C >= '0') && (C <= '9'));
    return Result;
}

internal inline bool
IsHexadecimal(char C)
{
    bool Result = (((C >= 'a') && (C <= 'f')) ||
                   ((C >= 'A') && (C <= 'F')) ||
                   (IsNumeric(C)));
    return Result;
}


struct token GetToken(struct tokenizer *Tokenizer);
bool RequireToken(struct tokenizer *Tokenizer, enum token_type DesiredType);
bool TokenEquals(struct token Token, const char *Match);

#endif
