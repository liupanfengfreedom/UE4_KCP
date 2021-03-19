// Fill out your copyright notice in the Description page of Project Settings.

#include "HttpServicev.h"
#include "Engine.h"
UHttpServicev::UHttpServicev()
{
	Http = &FHttpModule::Get();

}
UHttpServicev* UHttpServicev::GetANewInstance()
{

	return NewObject<UHttpServicev>();

}
bool UHttpServicev::ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful) {
	if (!bWasSuccessful || !Response.IsValid()) return false;
	if (EHttpResponseCodes::IsOk(Response->GetResponseCode())) return true;
	else {
		UE_LOG(LogTemp, Warning, TEXT("Http Response returned error code: %d"), Response->GetResponseCode());
		return false;
	}
}
void UHttpServicev::SetRequestHeaders(TSharedRef<IHttpRequest>& Request) {
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accepts"), TEXT("application/json"));
}
void UHttpServicev::HttpGet(FString uri)
{
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetURL(uri);
	SetRequestHeaders(Request);
	Request->OnProcessRequestComplete().BindUObject(this, &UHttpServicev::HttpResponseComplete);
	Request->OnRequestProgress().BindUObject(this,&UHttpServicev::HttpResponseProgress);
	Request->ProcessRequest();
}
void UHttpServicev::HttpPost(FString uri, FString username, FString password, FString payload, FString content)
{
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetURL(uri);
	SetRequestHeaders(Request);
	Request->SetHeader(TEXT("UserName"), username);
	Request->SetHeader(TEXT("Password"), password);
	Request->SetHeader(TEXT("Palyload"), payload);
	Request->SetVerb("POST");
	Request->SetContentAsString(content);
	Request->OnProcessRequestComplete().BindUObject(this, &UHttpServicev::HttpResponseComplete);
	Request->OnRequestProgress().BindUObject(this, &UHttpServicev::HttpResponseProgress);
	Request->ProcessRequest();
}
void UHttpServicev::HttpPost(FString uri, FString username, FString password, FString payload, TArray<uint8>& content)
{
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetURL(uri);
	SetRequestHeaders(Request);
	Request->SetHeader(TEXT("UserName"), username);
	Request->SetHeader(TEXT("Password"), password); 
	Request->SetHeader(TEXT("Palyload"), payload);
	Request->SetVerb("POST");
	Request->SetContent(content);
	Request->OnProcessRequestComplete().BindUObject(this, &UHttpServicev::HttpResponseComplete);
	Request->OnRequestProgress().BindUObject(this, &UHttpServicev::HttpResponseProgress);
	Request->ProcessRequest();
}
void UHttpServicev::HttpResponseComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	////GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,TEXT("HttpResponseComplete"));
	if (!ResponseIsValid(Response, bWasSuccessful))
	{
		OnHtpResponseFailed.Broadcast();
		return;
	}
	OnHttpResponseComplete.Broadcast(Response->GetContentAsString(), Response->GetContent(), extra);
}
void UHttpServicev::HttpResponseProgress(FHttpRequestPtr Resquest, int32 a, int32 b)
{
	////GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::FromInt(b).Append(FString(": b")));
	////GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::FromInt(a).Append(FString(": a")));
	float percent = (1000.0 * b) / 730130476;

//	////GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::FromInt(percent).Append(FString(": percent")));
	if (OnHttpResponseProgress.IsBound())
	{
		OnHttpResponseProgress.Broadcast(a, b, extra);
	}
}



