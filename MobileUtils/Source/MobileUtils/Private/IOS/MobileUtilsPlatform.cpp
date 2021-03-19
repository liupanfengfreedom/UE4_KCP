// Mobile Utils Plugin
// Created by Patryk Stepniewski
// Copyright (c) 2014-2019 gameDNA Ltd. All Rights Reserved.

#include "MobileUtilsPlatform.h"
#include "UE4Utils.h"
#include"slv.h"

FMobileUtilsPlatform::FMobileUtilsPlatform()
{
}

FMobileUtilsPlatform::~FMobileUtilsPlatform()
{
}

void FMobileUtilsPlatform::SetOrientation(int orientation)
{
}

void FMobileUtilsPlatform::StartActivity(const FString& activity)
{
}
FString FMobileUtilsPlatform::getlocalipaddress()
{
    char* cp = nullptr;
    NSString* ts = [slv getIPAddress : true];
    cp = (char*)[ts UTF8String];
    return FString(cp);
}
void FMobileUtilsPlatform::DispatchMessage(int message, const JsonSharedPtr jsonObject)
{
    if (jsonObject.IsValid())
    {
        FString ostr;
        TSharedRef<TJsonWriter<>> write = TJsonWriterFactory<>::Create(&ostr);
        if (FJsonSerializer::Serialize(jsonObject.ToSharedRef(), write))
            [UE4Utils messageCallback:message jsonString:TCHAR_TO_UTF8(*ostr)];
        else
            UE_LOG(LogEngine, Error, TEXT("\nDispatchMessage json serializer fail.\n"));
    }
    else
    {
        [UE4Utils messageCallback:message jsonString:NULL];
    }
}

FString FMobileUtilsPlatform::ConvertToAbsolutePath(const FString& path)
{
	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*path);
}