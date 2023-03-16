#include "macaron.h"

#include <stdlib.h>
#include <string.h>

#include <cassert>

#ifdef __cplusplus
extern "C" {
#endif

char *macaron_proxy_name;
void init_macaron_proxy_name(const char *name) {
    macaron_proxy_name = (char *)malloc(strlen(name) + 1);
    strncpy(macaron_proxy_name, name, strlen(name));
    macaron_proxy_name[strlen(name)] = '\0';
}

char *get_macaron_proxy_name() {
    assert(macaron_proxy_name != NULL);
    return macaron_proxy_name;
}

void deinit_macaron_proxy_name() {
    free(macaron_proxy_name);
}

#ifdef __cplusplus
}
#endif