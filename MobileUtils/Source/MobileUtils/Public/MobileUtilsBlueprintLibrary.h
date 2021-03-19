// Mobile Utils Plugin
// Created by Patryk Stepniewski
// Copyright (c) 2014-2019 gameDNA Ltd. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MobileUtilsBlueprintLibrary.generated.h"

UCLASS()
class MOBILEUTILS_API UMobileUtilsBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = MobileUtils)
	static void SetOrientation(int orientation);
	
	UFUNCTION(BlueprintCallable, Category = MobileUtils)
	static void StartActivity(const FString& activity);
	UFUNCTION(BlueprintPure, Category = MobileUtils)
		static FString Getlocalipaddress();
	UFUNCTION(BlueprintCallable, Category = MobileUtils)
	static FString ConvertToAbsolutePath(const FString& path);
};
