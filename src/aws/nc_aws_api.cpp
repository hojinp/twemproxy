#include <../macaron/macaron.h>
#include <../oscm/nc_oscm.h>
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <math.h>
#include <nc_core.h>
#include <pthread.h>

#include <chrono>
#include <cstdlib>
#include <iostream>

#include "nc_aws.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CACHE_BUCKET_NAME "macaron-osc"
#define DATALAKE_BUCKET_NAME "macaron-datalake"

Aws::SDKOptions sdk_options;

Aws::S3::S3Client* aws_s3_client_cache = NULL;
Aws::S3::S3Client* aws_s3_client_dl = NULL;

pthread_t packing_thread;            // thread for packing
int packing_thread_flag = 1;         // whether to keep runnig this packing worker thread or not
int packing_ptr = 0;                 // packing idx that should be processed now
int packing_end = 0;                 // the end of the packing index that is pending
int packing_idx = 0;                 // the packing id that will be assigned for the next packing
char** packing_ids;                  // packing id for each pending packings
char** packing_buffers;              // packing buffer for each pending packings
int packing_buffer_offset;           // offset of the packing buffer for the next data
struct packing_info* packing_infos;  // metadata of each pending packings
int* packing_buffer_sizes;           // how much data is filled in for each packing buffer
std::mutex packing_idx_mutex;
std::mutex packing_mutex;

void aws_init_sdk() {
    std::cout << "[aws_init_sdk] Initialize AWS SDK c++\n"
              << std::flush;
    Aws::InitAPI(sdk_options);
}

void aws_init_osc() {
    std::cout << "[aws_init_osc] Create OSC bucket: " << CACHE_BUCKET_NAME << "\n"
              << std::flush;

    // Initialize AWS Client for cache bucket
    Aws::Auth::AWSCredentials credentials;
    credentials.SetAWSAccessKeyId(std::getenv("AWS_ACCESS_KEY_ID"));
    credentials.SetAWSSecretKey(std::getenv("AWS_SECRET_ACCESS_KEY"));
    Aws::Client::ClientConfiguration configurations;
    aws_s3_client_cache = new Aws::S3::S3Client(credentials, configurations);

    // Create a cache bucket
    Aws::S3::Model::CreateBucketRequest request;
    request.SetBucket(CACHE_BUCKET_NAME);
    Aws::S3::Model::CreateBucketOutcome outcome = aws_s3_client_cache->CreateBucket(request);
    if (!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cout << "Error: CreateCacheBucket: " << err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Created bucket " << CACHE_BUCKET_NAME << " in the specified AWS Region." << std::endl;
    }

    // Start the OSC packing background thread
    int ti = 100;
    int ret = pthread_create(&packing_thread, NULL, &packing_worker, &ti);
    if (ret != 0) {
        std::cout << "Error: pthread_create() failed\n"
                  << std::flush;
        exit(EXIT_FAILURE);
    }
}

/**
 *  Packing related start
 */

void get_next_packing_id(char** packing_id) {
    std::lock_guard<std::mutex> lk(packing_idx_mutex);
    std::string name = proxy_name;
    name += ("-" + std::to_string(packing_idx++));
    strncpy(*packing_id, name.c_str(), name.length());
    (*packing_id)[name.length()] = '\0';
}

void packing_info_to_str(struct packing_info& p_info, char** p_info_str) {
    std::string ret = "";
    for (int i = 0; i < p_info.item_cnt; i++) {
        if (i != 0) {
            ret += ",";
        }
        std::string key_str = p_info.keys[i];
        ret += (key_str + "," + std::to_string(p_info.offsets[i]) + "," + std::to_string(p_info.sizes[i]) + "," + std::to_string(p_info.valids[i]));
    }
    *p_info_str = new char[MAX_PACKING_INFO_STR_LEN];
    strncpy(*p_info_str, ret.c_str(), ret.length());
    (*p_info_str)[ret.length()] = '\0';
}

void* packing_worker(void* ti) {
    std::cout << "[packing_worker] Started\n"
              << std::flush;

    // initalize packing related resources
    packing_ids = new char*[PACKING_BUFFER_CNT];
    packing_buffers = new char*[PACKING_BUFFER_CNT];
    packing_buffer_offset = 0;
    packing_infos = new struct packing_info[PACKING_BUFFER_CNT];
    for (int i = 0; i < PACKING_BUFFER_CNT; i++) {
        packing_ids[i] = new char[MAX_PACKING_ID_LEN];
        packing_buffers[i] = new char[PACKING_BLOCK_SIZE];
        for (int j = 0; j < MAX_PACKING_ITEM_CNT; j++) {
            packing_infos[i].keys[j] = new char[MACARON_MAX_KEY_LEN + 1];
        }
        packing_infos[i].data_size = 0;
        packing_infos[i].item_cnt = 0;
    }
    get_next_packing_id(&packing_ids[0]);

    // run packing iterations
    while (packing_thread_flag) {
        while (packing_ptr != packing_end) {
            aws_put_data_to_osc(&packing_ids[packing_ptr][0], packing_buffers[packing_ptr], packing_buffer_sizes[packing_ptr]);
            char* packing_info_str;
            packing_info_to_str(packing_infos[packing_ptr], &packing_info_str);
            oscm_put_metadata(&packing_ids[packing_ptr][0], packing_info_str);
            free(packing_info_str);

            std::lock_guard<std::mutex> lk(packing_mutex);
            packing_infos[packing_ptr].data_size = 0;
            packing_infos[packing_ptr].item_cnt = 0;
            packing_ptr = (packing_ptr + 1) % PACKING_BUFFER_CNT;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(*((int*)ti)));
    }

    // clean up the memory used for packing
    for (int i = 0; i < PACKING_BUFFER_CNT; i++) {
        free(packing_buffers[i]);
    }
    free(packing_ids);
    free(packing_buffers);
    free(packing_infos);

    return NULL;
}

bool check_current_packing(int data_size) {
    return packing_infos[packing_end].item_cnt + 1 < MAX_PACKING_ITEM_CNT && packing_infos[packing_end].data_size + data_size < PACKING_BLOCK_SIZE;
}

void add_data_to_packing(char* key, char* data, int data_size) {
    strncpy(&packing_buffers[packing_end][packing_buffer_offset], data, data_size);
    packing_buffer_offset += data_size;

    int idx = packing_infos[packing_end].item_cnt;
    size_t key_len = strlen(key);
    strncpy(packing_infos[packing_end].keys[idx], key, key_len);
    packing_infos[packing_end].keys[idx][(int)key_len] = '\0';
    packing_infos[packing_end].offsets[idx] = packing_infos[packing_end].data_size;
    packing_infos[packing_end].sizes[idx] = data_size;
    packing_infos[packing_end].valids[idx] = 1;
    packing_infos[packing_end].data_size += data_size;
    packing_infos[packing_end].item_cnt += 1;
}

void update_packing_buffer() {
    packing_end = (packing_end + 1) % PACKING_BUFFER_CNT;
    get_next_packing_id(&packing_ids[packing_end]);
}

/**
 * Packing related ends
 */

void aws_deinit_osc() {
    free(aws_s3_client_cache);
    packing_thread_flag = 0;
    pthread_join(packing_thread, NULL);
    free(packing_ids);
}

void aws_init_datalake() {
    std::cout << "[aws_init_datalake] Create DATALAKE bucket: " << DATALAKE_BUCKET_NAME << "\n"
              << std::flush;
    Aws::Auth::AWSCredentials credentials;
    credentials.SetAWSAccessKeyId(std::getenv("AWS_ACCESS_KEY_ID"));
    credentials.SetAWSSecretKey(std::getenv("AWS_SECRET_ACCESS_KEY"));
    Aws::Client::ClientConfiguration configurations;
    aws_s3_client_dl = new Aws::S3::S3Client(credentials, configurations);

    Aws::S3::Model::CreateBucketRequest request;
    request.SetBucket(DATALAKE_BUCKET_NAME);
    Aws::S3::Model::CreateBucketOutcome outcome = aws_s3_client_dl->CreateBucket(request);
    if (!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cout << "Error: CreateDatalakeBucket: " << err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Created bucket " << DATALAKE_BUCKET_NAME << " in the specified AWS Region." << std::endl;
    }
}

void aws_deinit_datalake() {
    free(aws_s3_client_dl);
}

void aws_deinit_sdk() {
    std::cout << "[aws_deinit_sdk] Deinitialize AWS SDK c++\n"
              << std::flush;
    Aws::ShutdownAPI(sdk_options);
}

void aws_get_bucket_list() {
    std::cout << "[aws_get_bucket_list] Start\n"
              << std::flush;

    auto outcome = aws_s3_client_cache->ListBuckets();
    if (outcome.IsSuccess()) {
        std::cout << "Found " << outcome.GetResult().GetBuckets().size() << " buckets\n"
                  << std::flush;
        for (auto&& b : outcome.GetResult().GetBuckets()) {
            std::cout << b.GetName() << std::endl
                      << std::flush;
        }
    } else {
        std::cout << "Failed with error: " << outcome.GetError() << std::endl
                  << std::flush;
    }
}

void aws_get_data_from_osc(char* object_name, int offset, int size, char* data, int buffer_size) {
    std::cout << "[aws_get_data_from_osc] Start\n"
              << std::flush;
    Aws::S3::Model::GetObjectRequest request;
    request.SetBucket(CACHE_BUCKET_NAME);
    request.SetKey(object_name);
    request.SetRange("bytes=" + std::to_string(offset) + "-" + std::to_string(offset + size));
    Aws::S3::Model::GetObjectOutcome outcome = aws_s3_client_cache->GetObject(request);
    if (outcome.IsSuccess()) {
        int n = outcome.GetResult().GetContentLength();
        std::stringstream ss;
        ss << outcome.GetResult().GetBody().rdbuf();
        snprintf(data, (size_t)buffer_size, "%s", ss.str().c_str());
        std::cout << "Size retrieved from S3: " << n << " bytes\n"
                  << std::flush;
        std::cout << "Retrieved data from S3: " << ss.str() << "\n"
                  << std::flush;
    } else {
        auto err = outcome.GetError();
        std::cout << "ERROR: GetObject: " << err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void aws_put_data_to_osc(char* key, char* data, int data_size) {
    //std::cout << "[aws_put_data_to_osc] Data: " << data << "\n" << std::flush;
    //std::cout << "[aws_put_data_to_osc] Key: " << key << "\n" << std::flush;
    //std::cout << "[aws_put_data_to_osc] Data size: " << data_size << "\n" << std::flush;
    Aws::S3::Model::PutObjectRequest request;
    request.SetBucket(CACHE_BUCKET_NAME);
    request.SetKey(key);
    const std::shared_ptr<Aws::IOStream> inputData = Aws::MakeShared<Aws::StringStream>("");
    *inputData << data;
    request.SetBody(inputData);
    Aws::S3::Model::PutObjectOutcome outcome = aws_s3_client_cache->PutObject(request);
    if (!outcome.IsSuccess()) {
        std::cerr << "Error: PutObjectBuffer: " << outcome.GetError().GetMessage() << "\n"
                  << std::flush;
    } else {
        std::cout << "Success: Object '" << key << "' uploaded to bucket '" << CACHE_BUCKET_NAME << "'.\n"
                  << std::flush;
    }
}

void aws_put_data_to_osc_packing(char* key, char* data, int data_size) {
    std::cout << "[aws_put_data_to_osc_packing] Key: " << key << "\n"
              << std::flush;
    if (data_size >= PACKING_BLOCK_SIZE) {  // save the data to OSC right now, instead of adding to the packing buffer
        char *packing_id = new char[MAX_PACKING_ID_LEN];
        get_next_packing_id(&packing_id);
        std::cout << "[aws_put_data_to_osc_packing] new packing id: " << packing_id << "\n"
                  << std::flush;
        struct packing_info p_info;
        size_t key_len = strlen(key);
        std::cout << "Key length: " << key_len << "\n" << std::flush;
        strncpy(p_info.keys[0], key, key_len);
        p_info.keys[0][key_len] = '\0';
        p_info.offsets[0] = 0;
        p_info.sizes[0] = data_size;
        p_info.valids[0] = 1;
        p_info.item_cnt = 1;
        aws_put_data_to_osc(packing_id, data, data_size);
        char* packing_info_str;
        packing_info_to_str(p_info, &packing_info_str);
        oscm_put_metadata(packing_id, packing_info_str);
        std::cout << "[aws_put_data_to_osc_packing] Metadata in string: " << packing_info_str << "\n"
                  << std::flush;
        free(packing_info_str);
        free(packing_id);
    } else {
        std::lock_guard<std::mutex> lk(packing_mutex);
        bool ok = check_current_packing(data_size);
        if (!ok) {  // create a new packing buffer to add data
            update_packing_buffer();
        }
        add_data_to_packing(key, data, data_size);
    }
}

int aws_get_data_from_datalake(char* key, char** new_msg, int* new_msg_len, int* data_offset, int* data_size) {
    std::cout << "[aws_get_data_from_datalake] Start key: " << key << std::endl;
    Aws::S3::Model::GetObjectRequest request;
    request.SetBucket(DATALAKE_BUCKET_NAME);
    request.SetKey(key);
    Aws::S3::Model::GetObjectOutcome outcome = aws_s3_client_dl->GetObject(request);
    if (outcome.IsSuccess()) {
        std::stringstream ss;
        ss << outcome.GetResult().GetBody().rdbuf();
        int n = outcome.GetResult().GetContentLength();
        *data_size = n;

        int value_str_len = (int)log10(abs(n)) + 1;
        *new_msg_len = 5 + n + value_str_len;

        int offset = 0;
        *new_msg = (char*)malloc(*new_msg_len + 1);
        (*new_msg)[0] = '$';
        (*new_msg)[*new_msg_len] = '\0';
        offset += 1;
        snprintf((*new_msg) + offset, *new_msg_len, "%d", n);
        offset += value_str_len;
        (*new_msg)[offset] = '\r';
        (*new_msg)[offset + 1] = '\n';
        offset += 2;
        *data_offset = offset;
        snprintf((*new_msg) + offset, *new_msg_len, "%s", ss.str().c_str());
        offset += n;
        (*new_msg)[offset] = '\r';
        (*new_msg)[offset + 1] = '\n';
        offset += 2;
        assert(offset == *new_msg_len);
        // std::cout << "[aws_get_data_from_datalake] new msg: " << *new_msg << "\n" << std::flush;
        // std::cout << "[aws_get_data_from_datalake] Retrieved data from S3:\n"
        //           << ss.str() << "\n"
        //           << std::flush;
        std::cout << "[aws_get_data_from_datalake] Size retrieved from S3: " << n << " bytes\n"
                  << std::flush;
        return 1;
    } else {
        std::cout << "[aws_get_data_from_datalake] No such data for the key: " << key << std::endl;
        return 0;
    }
}

#ifdef __cplusplus
}
#endif