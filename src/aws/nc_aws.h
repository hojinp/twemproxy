#ifndef _NC_AWS_H_
#define _NC_AWS_H_

#ifdef __cplusplus
extern "C" {
#endif

void aws_init_sdk(void);

void aws_deinit_sdk(void);

void aws_get_bucket_list(void);

void aws_get_data_from_osc(char *object_name, int offset, int size, char* data);

#ifdef __cplusplus
}
#endif

#endif /* _NC_AWS_H_ */