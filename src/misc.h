#ifndef KHD_MISC_H
#define KHD_MISC_H

#include <stdio.h>
#include <stdarg.h>

#define internal static

internal inline void
Error(const char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    vfprintf(stderr, Format, Args);
    va_end(Args);

    exit(EXIT_FAILURE);
}

#endif
