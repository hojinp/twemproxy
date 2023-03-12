#include <nc_core.h>

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/utils/logging/LogLevel.h>
#include <aws/s3/S3Client.h>
#include <iostream>

#include "nc_aws.h"

#ifdef __cplusplus
extern "C" {
#endif

Aws::SDKOptions sdk_options;

void init_aws_sdk() {
    std::cout << "[InitAwsSdk] Initialize AWS SDK c++\n" << std::flush;
    sdk_options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info;
    Aws::InitAPI(sdk_options);
}

void deinit_aws_sdk() {
    std::cout << "[DeinitAwsSdk] Deinitialize AWS SDK c++\n" << std::flush;
    Aws::ShutdownAPI(sdk_options);
}

void get_aws_bucket_list() {
    std::cout << "[getAwsBucketList] Start\n" << std::flush;

    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.proxyHost = "proxy.pdl.cmu.edu";
    clientConfig.proxyPort = 3128;
    clientConfig.proxyScheme = Aws::Http::Scheme::HTTP;

    Aws::S3::S3Client aws_s3_client(clientConfig);
    auto outcome = aws_s3_client.ListBuckets();
    if (outcome.IsSuccess()) {
        std::cout << "Found " << outcome.GetResult().GetBuckets().size() << " buckets\n" << std::flush;
        for (auto&& b : outcome.GetResult().GetBuckets()) {
            std::cout << b.GetName() << std::endl << std::flush;
        }
    }
    else {
        std::cout << "Failed with error: " << outcome.GetError() << std::endl << std::flush;
    }
}

#ifdef __cplusplus
}
#endif