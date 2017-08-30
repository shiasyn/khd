#ifndef KHD_MODE_H
#define KHD_MODE_H

#include "hashtable.h"
#include <stdbool.h>

struct mode
{
    char *name;
    char *on_enter;

    long long time;
    bool is_prefix;
    float timeout;

    struct table hotkey_map;
};

bool same_mode(struct mode *a, struct mode *b);
unsigned long hash_mode(struct mode *a);

#endif
