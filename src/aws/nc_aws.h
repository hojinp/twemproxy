#ifndef _NC_AWS_H_
#define _NC_AWS_H_

#ifdef __cplusplus
extern "C" {
#endif

void init_aws_sdk(void);

void deinit_aws_sdk(void);

void get_aws_bucket_list(void);

#ifdef __cplusplus
}
#endif

#endif /* __NC_AWS_H__ */