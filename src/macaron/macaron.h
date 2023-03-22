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

extern char *macaron_oscm_ip;
void init_macaron_oscm_ip(const char *ip);
char *get_macaron_oscm_ip();
void deinit_macaron_oscm_ip();

extern char *datalake_region;
extern int datalake_region_initalized = 0;
void init_datalake_region(const char *region);
char *get_datalake_region();
void deinit_datalake_region();
int get_datalake_region_initialized();

#ifdef __cplusplus
}
#endif

#endif /* _MACARON_H_ */