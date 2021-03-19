//
//  UE4Utils.mm
//  InfiniteLife1_0_Index
//
//  Created by Mac on 2019/10/22.
//  Copyright © 2019 Epic Games, Inc. All rights reserved.
//

#include "UE4Utils.h"
#include "IMobileUtils.h"

static void nativeOnMessageCallback(int message, const char *json_uc)
{
    if (FTaskGraphInterface::IsRunning())
    {
        if (json_uc != NULL)
        {
            FString json_fstr = FString(json_uc);

            JsonSharedPtr json_obj = MakeShareable(new FJsonObject);
            TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(json_fstr);
            if (FJsonSerializer::Deserialize(reader, json_obj))
            {
                FFunctionGraphTask::CreateAndDispatchWhenReady([json_obj, message]()
                {
                    MessageCallback callback = IMobileUtils::Get().GetPlatformInterface()->GetMessageCallback();
                    if (callback != NULL)
                        callback(message, json_obj);
                }, TStatId(), NULL, ENamedThreads::GameThread);
            }
            else
            {
                UE_LOG(LogEngine, Error, TEXT("\nNativeOnMessageCallback json deserialize fail.\n"));
            }
        }
        else
        {
            FFunctionGraphTask::CreateAndDispatchWhenReady([message]()
            {
                MessageCallback callback = IMobileUtils::Get().GetPlatformInterface()->GetMessageCallback();
                if (callback != NULL)
                    callback(message, NULL);
            }, TStatId(), NULL, ENamedThreads::GameThread);
        }
    }
}

@implementation UE4Utils

+ (void)messageCallback:(int)message jsonString:(const char *)json_uc
{
}

+ (void)dispatchMessage:(int)message jsonString:(const char *)json_uc
{
    nativeOnMessageCallback(message, json_uc);
}

@end
