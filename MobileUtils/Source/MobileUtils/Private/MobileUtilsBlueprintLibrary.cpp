// Mobile Utils Plugin
// Created by Patryk Stepniewski
// Copyright (c) 2014-2019 gameDNA Ltd. All Rights Reserved.

#include "MobileUtilsBlueprintLibrary.h"
#include "IMobileUtils.h"

UMobileUtilsBlueprintLibrary::UMobileUtilsBlueprintLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMobileUtilsBlueprintLibrary::SetOrientation(int orientation)
{
#if PLATFORM_ANDROID || PLATFORM_IOS
	IMobileUtils::Get().GetPlatformInterface()->SetOrientation(orientation);
#endif
}

void UMobileUtilsBlueprintLibrary::StartActivity(const FString& activity)
{
#if PLATFORM_ANDROID || PLATFORM_IOS
	IMobileUtils::Get().GetPlatformInterface()->StartActivity(activity);
#endif
}
FString UMobileUtilsBlueprintLibrary::Getlocalipaddress()
{
	
	return IMobileUtils::Get().GetPlatformInterface()->getlocalipaddress();

}
FString UMobileUtilsBlueprintLibrary::ConvertToAbsolutePath(const FString& path)
{
	return IMobileUtils::Get().GetPlatformInterface()->ConvertToAbsolutePath(path);
}