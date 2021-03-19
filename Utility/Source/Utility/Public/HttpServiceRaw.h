// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsonUtilities.h"
#include "Json.h"
#include "Runtime/Online/HTTP/Public/Http.h"

/**
 * 
 */

class LoadFileFromLocalOrCloud;
class UTILITY_API HttpServiceRaw
{
	static TArray<TSharedPtr<HttpServiceRaw, ESPMode::ThreadSafe>> httppool;
	bool busy=false;
	int64 filesizeinserver=0;
public:
	HttpServiceRaw();
	~HttpServiceRaw();
	FHttpModule* Http;
	void SetRequestHeaders(TSharedRef<IHttpRequest>& Request);
	bool ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful);
	TSharedPtr<class LoadFileFromLocalOrCloud, ESPMode::ThreadSafe> mloadfilefromlocalorcloud;
	//static UHttpServicev1* _instance;
public:
	DECLARE_DELEGATE(FOnHtpResponseFailedRaw);
	DECLARE_DELEGATE_ThreeParams(FOnHttpResponseCompleteRaw, const FString&, const TArray<uint8>&, UObject*);
	DECLARE_DELEGATE_ThreeParams(FOnHttpResponseProgressRaw, float, float, UObject*);
	FOnHttpResponseCompleteRaw OnHttpResponseComplete;
	FOnHttpResponseProgressRaw OnHttpResponseProgress;
	FOnHtpResponseFailedRaw    OnHtpResponseFailed;
	UObject* extra;
public:
	void HttpGet(FString uri);
	void HttpGet(FString uri, FString username, FString password);
	void HttpPost(FString uri, FString username, FString password, FString payload, FString content);
	void HttpPost(FString uri, FString username, FString password, FString payload, TArray<uint8> & content);
	void HttpResponseHeaderReceived(FHttpRequestPtr Request, const FString& headername, const FString& headervalue);
	void HttpResponseComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HttpResponseProgress(FHttpRequestPtr Resquest, int32 a, int32 b);
	static TSharedPtr<HttpServiceRaw,ESPMode::ThreadSafe>  GetANewInstance();
};
