// Mobile Utils Plugin
// Created by Patryk Stepniewski
// Copyright (c) 2014-2019 gameDNA Ltd. All Rights Reserved.

#pragma once

#include "Interfaces/MobileUtilsInterface.h"
#include "EngineMinimal.h"
#include "Core.h"

class FMobileUtilsPlatform : public IMobileUtilsInterface
{
public:
	FMobileUtilsPlatform();
	virtual ~FMobileUtilsPlatform();

	virtual void SetOrientation(int orientation) override;
	virtual void StartActivity(const FString& activity) override;
	virtual void DispatchMessage(int message, const JsonSharedPtr jsonObject) override;
	virtual FString getlocalipaddress() override;
	virtual FString ConvertToAbsolutePath(const FString& path) override;
};
