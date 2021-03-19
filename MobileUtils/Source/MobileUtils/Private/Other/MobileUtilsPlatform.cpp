// Mobile Utils Plugin
// Created by Patryk Stepniewski
// Copyright (c) 2014-2019 gameDNA Ltd. All Rights Reserved.

#include "MobileUtilsPlatform.h"
#include "Networking.h"

#if !(PLATFORM_ANDROID || PLATFORM_IOS)
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
	bool bCanBindAll;
	TSharedPtr<class FInternetAddr> Addr1 = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBindAll);
	FString MyIP = Addr1->ToString(false);
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, MyIP);
	return MyIP;
}
void FMobileUtilsPlatform::DispatchMessage(int message, const JsonSharedPtr jsonObject)
{
}

FString FMobileUtilsPlatform::ConvertToAbsolutePath(const FString& path)
{
	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*path);
}
#endif
