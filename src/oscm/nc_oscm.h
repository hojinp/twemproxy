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

void oscm_init_lib(void);
void oscm_deinit_lib(void);

struct oscm_result* oscm_get_metadata(char* key);
void oscm_put_metadata(char* block_id, char* block_info);
void oscm_delete_metadata(char* key);

#ifdef __cplusplus
}
#endif

#endif /* _NC_OSCM_H_ */