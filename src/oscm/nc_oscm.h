#ifndef _NC_OSCM_H_
#define _NC_OSCM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define OSC_NAME_SIZE 32
struct oscm_result {
    char block_id[OSC_NAME_SIZE];
    int offset;
    int size;
    unsigned exist : 1;
};

void init_oscm_lib(void);

void deinit_oscm_lib(void);

struct oscm_result* get_oscm_metadata(char* key);
void put_oscm_metadata(char* block_id, char* block_info);
void delete_oscm_metadata(char* key);

#ifdef __cplusplus
}
#endif

#endif /* _NC_OSCM_H_ */