// Mobile Utils Plugin
// Created by Patryk Stepniewski
// Copyright (c) 2014-2019 gameDNA Ltd. All Rights Reserved.

#include "IMobileUtils.h"
#include "MobileUtilsBlueprintLibrary.h"

#if !(PLATFORM_ANDROID || PLATFORM_IOS)
#include "Other/MobileUtilsPlatform.h"
#else
#include "MobileUtilsPlatform.h"
#endif

DEFINE_LOG_CATEGORY(LogMobileUtils);

#define LOCTEXT_NAMESPACE "MobileUtils"

class FMobileUtils : public IMobileUtils
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FMobileUtils, MobileUtils)

// Startup Module
void FMobileUtils::StartupModule()
{	
	PlatformInterface = MakeShareable(new FMobileUtilsPlatform());
}

// Shutdown Module
void FMobileUtils::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
