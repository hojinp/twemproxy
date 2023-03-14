#ifndef _NC_AWS_H_
#define _NC_AWS_H_

#ifdef __cplusplus
extern "C" {
#endif

void aws_init_sdk(void);
void aws_deinit_sdk(void);

void aws_init_osc(void);
void aws_deinit_osc(void);
void aws_init_datalake(void);
void aws_deinit_datalake(void);
void aws_get_bucket_list(void);
void aws_get_data_from_osc(char* key, int offset, int size, char* data, int buffer_size);
void aws_put_data_to_osc(char* key, char* data, int data_size);
int aws_get_data_from_datalake(char* key, char** new_msg, int* new_msg_len, int* data_offset, int* data_size);

#ifdef __cplusplus
}
#endif

#endif /* _NC_AWS_H_ */