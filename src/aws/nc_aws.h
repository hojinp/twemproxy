#ifndef _NC_AWS_H_
#define _NC_AWS_H_

#include "../macaron/macaron.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PACKING_BLOCK_SIZE 4 * 1024 * 1024                                          // XXX: Pakcing block size: 4 MB (manual)
#define MAX_PACKING_INFO_STR_LEN (40 + MACARON_MAX_KEY_LEN) * MAX_PACKING_ITEM_CNT  // max number of chars for the packing information
#define PACKING_BUFFER_CNT 100                                                      // how many number of packings can be pending
#define MAX_PACKING_ID_LEN 50                                                       // max number of chars for each packing id
#define MAX_PACKING_ITEM_CNT 64                                                     // max number of items that can be saved in a single buffer

struct packing_info {
    char* keys[MAX_PACKING_ITEM_CNT];
    int offsets[MAX_PACKING_ITEM_CNT];
    int sizes[MAX_PACKING_ITEM_CNT];
    int valids[MAX_PACKING_ITEM_CNT];
    int data_size;
    int item_cnt;
};

void aws_init_sdk(void);
void aws_deinit_sdk(void);

void aws_init_osc(void);
void aws_deinit_osc(void);
void aws_init_datalake(void);
void aws_deinit_datalake(void);
void aws_get_bucket_list(void);
void aws_get_data_from_osc(char* key, int offset, int size, char* data, int buffer_size);
void aws_put_data_to_osc(char* key, char* data, int data_size);
void aws_put_data_to_osc_packing(char* key, char* data, int data_size);
int aws_get_data_from_datalake(char* key, char** new_msg, int* new_msg_len, int* data_offset, int* data_size);
void aws_put_data_to_datalake(char* key, char* data, int data_size);

void* packing_worker(void* ti);

#ifdef __cplusplus
}
#endif

#endif /* _NC_AWS_H_ */