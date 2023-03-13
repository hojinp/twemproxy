#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <nc_core.h>

#include <cstdlib>
#include <iostream>

#include "nc_aws.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CACHE_BUCKET_NAME "macaron-osc"

Aws::SDKOptions sdk_options;

Aws::S3::S3Client* aws_s3_client = NULL;

void aws_init_sdk() {
    std::cout << "[aws_init_sdk] Initialize AWS SDK c++\n"
              << std::flush;
    Aws::InitAPI(sdk_options);

    std::cout << "[aws_init_sdk] Create OSC bucket: " << CACHE_BUCKET_NAME << "\n"
              << std::flush;
    Aws::Auth::AWSCredentials credentials;
    credentials.SetAWSAccessKeyId(std::getenv("AWS_ACCESS_KEY_ID"));
    credentials.SetAWSSecretKey(std::getenv("AWS_SECRET_ACCESS_KEY"));
    Aws::Client::ClientConfiguration configurations;
    aws_s3_client = new Aws::S3::S3Client(credentials, configurations);

    Aws::S3::Model::CreateBucketRequest request;
    request.SetBucket(CACHE_BUCKET_NAME);
    Aws::S3::Model::CreateBucketOutcome outcome = aws_s3_client->CreateBucket(request);
    if (!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cout << "Error: CreateBucket: " << err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Created bucket " << CACHE_BUCKET_NAME << " in the specified AWS Region." << std::endl;
    }
}

void aws_deinit_sdk() {
    std::cout << "[aws_deinit_sdk] Deinitialize AWS SDK c++\n"
              << std::flush;
    free(aws_s3_client);
    Aws::ShutdownAPI(sdk_options);
}

void aws_get_bucket_list() {
    std::cout << "[aws_get_bucket_list] Start\n"
              << std::flush;

    auto outcome = aws_s3_client->ListBuckets();
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

void aws_get_data_from_osc(char* object_name, int offset, int size, char* data) {
    std::cout << "[aws_get_data_from_osc] Start\n"
              << std::flush;
    Aws::S3::Model::GetObjectRequest request;
    request.SetBucket(CACHE_BUCKET_NAME);
    request.SetKey(object_name);
    request.SetRange("bytes=" + std::to_string(offset) + "-" + std::to_string(offset + size - 1));
    Aws::S3::Model::GetObjectOutcome outcome = aws_s3_client->GetObject(request);
    if (outcome.IsSuccess()) {
        int n = outcome.GetResult().GetContentLength();
        std::stringstream ss;
        ss << outcome.GetResult().GetBody().rdbuf();
        snprintf(data, (size_t)n, "%s", ss.str().c_str());
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

#ifdef __cplusplus
}
#endif