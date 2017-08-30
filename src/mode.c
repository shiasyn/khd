#include "mode.h"
#include "hotkey.h"

#include <stdbool.h>
#include <string.h>

#define internal static

internal inline unsigned long
pjw_hash(char *key)
{
    unsigned long hash = 0, high;
    while(*key) {
        hash = (hash << 4) + *key++;
        high = hash & 0xF0000000;
        if(high) {
            hash ^= (high >> 24);
        }
        hash &= ~high;
    }
    return hash;
}

bool same_mode(struct mode *a, struct mode *b)
{
    return (strcmp(a->name, b->name) == 0);
}

unsigned long hash_mode(char *name)
{
    return pjw_hash(name);
}

internal inline char *
copy_string(char *s)
{
    int length = strlen(s);
    char *result = malloc(length + 1);
    memcpy(result, s, length);
    result[length] = '\0';
    return result;
}

struct mode *
create_mode(char *name)
{
    struct mode *mode = malloc(sizeof(struct mode));
    memset(mode, 0, sizeof(struct mode));
    mode->name = copy_string(name);
    table_init(&mode->hotkey_map,
               131,
               (table_hash_func) hash_hotkey,
               (table_compare_func) same_hotkey);
    return mode;
}
