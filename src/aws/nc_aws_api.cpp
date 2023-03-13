#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <math.h>
#include <nc_core.h>

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

void aws_init_sdk() {
    std::cout << "[aws_init_sdk] Initialize AWS SDK c++\n"
              << std::flush;
    Aws::InitAPI(sdk_options);
}

void aws_init_osc() {
    std::cout << "[aws_init_osc] Create OSC bucket: " << CACHE_BUCKET_NAME << "\n"
              << std::flush;
    Aws::Auth::AWSCredentials credentials;
    credentials.SetAWSAccessKeyId(std::getenv("AWS_ACCESS_KEY_ID"));
    credentials.SetAWSSecretKey(std::getenv("AWS_SECRET_ACCESS_KEY"));
    Aws::Client::ClientConfiguration configurations;
    aws_s3_client_cache = new Aws::S3::S3Client(credentials, configurations);

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

void aws_deinit_sdk() {
    std::cout << "[aws_deinit_sdk] Deinitialize AWS SDK c++\n"
              << std::flush;
    free(aws_s3_client_cache);
    free(aws_s3_client_dl);
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

int aws_get_data_from_datalake(char* key, char** new_msg, int* new_msg_len) {
    std::cout << "[aws_get_data_from_datalake] Start key: " << key << std::endl;
    Aws::S3::Model::GetObjectRequest request;
    request.SetBucket(DATALAKE_BUCKET_NAME);
    request.SetKey(key);
    Aws::S3::Model::GetObjectOutcome outcome = aws_s3_client_dl->GetObject(request);
    if (outcome.IsSuccess()) {
        std::stringstream ss;
        ss << outcome.GetResult().GetBody().rdbuf();
        int n = outcome.GetResult().GetContentLength();

        int value_str_len = (int)log10(abs(n)) + 1;
        *new_msg_len = 5 + n + value_str_len;

        int offset = 0;
        *new_msg = (char*)malloc(*new_msg_len + 1);
        (*new_msg)[0] = '$';
        (*new_msg)[*new_msg_len] = '\0';
        offset += 1;
        snprintf((*new_msg) + offset, *new_msg_len, "%d", value_str_len);
        offset += value_str_len;
        (*new_msg)[offset] = '\r';
        (*new_msg)[offset + 1] = '\n';
        offset += 2;
        snprintf((*new_msg) + offset, *new_msg_len, "%s", ss.str().c_str());
        offset += n;
        (*new_msg)[offset] = '\r';
        (*new_msg)[offset + 1] = '\n';
        offset += 2;
        assert(offset == *new_msg_len);
        std::cout << "[aws_get_data_from_datalake] new msg: " << *new_msg << std::flush;
        std::cout << "[aws_get_data_from_datalake] Size retrieved from S3: " << n << " bytes\n"
                  << std::flush;
        std::cout << "[aws_get_data_from_datalake] Retrieved data from S3:\n"
                  << ss.str() << "\n"
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