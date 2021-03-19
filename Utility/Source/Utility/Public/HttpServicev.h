// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsonUtilities.h"
#include "Json.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "UObject/NoExportTypes.h"
#include "HttpServicev.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHtpResponseFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHttpResponseComplete, const FString&, Responsestring, const TArray<uint8>&, Responsecontent, UObject*, extra);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHttpResponseProgress, int32, a, int32, b, UObject*, extra);

UCLASS()
class UTILITY_API UHttpServicev : public UObject
{
	GENERATED_BODY()
	FHttpModule* Http;
	void SetRequestHeaders(TSharedRef<IHttpRequest>& Request);
	bool ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful);
	//static UHttpServicev1* _instance;
public:
	UHttpServicev();
	FOnHttpResponseComplete OnHttpResponseComplete;
	FOnHttpResponseProgress OnHttpResponseProgress;
	FOnHtpResponseFailed    OnHtpResponseFailed;
	UObject* extra;
public:
	void HttpGet(FString uri);
	void HttpPost(FString uri,FString username,FString password,FString payload, FString content);
	void HttpPost(FString uri, FString username, FString password, FString payload, TArray<uint8> & content);
	void HttpResponseComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HttpResponseProgress(FHttpRequestPtr Resquest, int32 a, int32 b);
	static  UHttpServicev*  GetANewInstance();
	
	
	
};
