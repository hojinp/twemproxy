#ifndef _MACARON_H_
#define _MACARON_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MACARON_MAX_KEY_LEN 32

extern char *macaron_proxy_name;
void init_macaron_proxy_name(const char *name);
char *get_macaron_proxy_name();
void deinit_macaron_proxy_name();

#ifdef __cplusplus
}
#endif

#endif /* _MACARON_H_ */