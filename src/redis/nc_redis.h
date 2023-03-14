#ifndef _NC_REDIS_H_
#define _NC_REDIS_H_

#ifdef __cplusplus
extern "C" {
#endif

void redis_init();
void redis_deinit();

void redis_put_data(char *key, char *data, int len);

#ifdef __cplusplus
}
#endif

#endif /* _NC_REDIS_H_ */