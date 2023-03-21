#include "macaron.h"

#include <stdlib.h>
#include <string.h>

#include <cassert>

#ifdef __cplusplus
extern "C" {
#endif

char *macaron_proxy_name;
char *macaron_oscm_ip;

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

void init_macaron_oscm_ip(const char *ip) {
    macaron_oscm_ip = (char *)malloc(strlen(ip) + 1);
    strncpy(macaron_oscm_ip, ip, strlen(ip));
    macaron_oscm_ip[strlen(ip)] = '\0';
}

char *get_macaron_oscm_ip() {
    assert(macaron_oscm_ip != NULL);
    return macaron_oscm_ip;
}

void deinit_macaron_oscm_ip() {
    free(macaron_oscm_ip);
}

#ifdef __cplusplus
}
#endif