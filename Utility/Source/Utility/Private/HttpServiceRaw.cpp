// Fill out your copyright notice in the Description page of Project Settings.


#include "HttpServiceRaw.h"
#include "Engine.h"
#include "MyBlueprintFunctionLibrary.h"

TArray<TSharedPtr<HttpServiceRaw, ESPMode::ThreadSafe>> HttpServiceRaw::httppool;
HttpServiceRaw::HttpServiceRaw()
{
	Http = &FHttpModule::Get();
}
TSharedPtr<HttpServiceRaw, ESPMode::ThreadSafe> HttpServiceRaw::GetANewInstance()
{
	for (auto var : httppool)
	{
		if (!var->busy)
		{
			var->OnHtpResponseFailed.Unbind();
			var->OnHttpResponseProgress.Unbind();
			var->OnHttpResponseComplete.Unbind();
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("recycle HttpServiceRaw"));
			return var;
		}
	}
	TSharedPtr<HttpServiceRaw, ESPMode::ThreadSafe> temp = MakeShareable(new HttpServiceRaw());
	httppool.Add(temp);
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("create a new one HttpServiceRaw"));
	return temp;
}
HttpServiceRaw::~HttpServiceRaw()
{

}
bool HttpServiceRaw::ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful) {
	if (!bWasSuccessful || !Response.IsValid()) return false;
	if (EHttpResponseCodes::IsOk(Response->GetResponseCode())) return true;
	else {
		UE_LOG(LogTemp, Warning, TEXT("Http Response returned error code: %d"), Response->GetResponseCode());
		return false;
	}
}
void HttpServiceRaw::SetRequestHeaders(TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& Request) {
	//Request->SetHeader(TEXT("Accept"), TEXT("text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3"));
	//Request->SetHeader(TEXT("Accept-Encoding"), TEXT("gzip, deflate, br"));
	//Request->SetHeader(TEXT("Accept-Language"), TEXT("zh-CN,zh;q=0.9"));
	//Request->SetHeader(TEXT("Connection"), TEXT("keep-alive"));
	//Request->SetHeader(TEXT("Sec-Fetch-Mode"), TEXT("navigate"));
	//Request->SetHeader(TEXT("Sec-Fetch-Site"), TEXT("none"));
	//Request->SetHeader(TEXT("Upgrade-Insecure-Requests"), TEXT("1"));
	//Request->SetHeader(TEXT("User-Agent"), TEXT("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36"));

}
void HttpServiceRaw::HttpGet(FString uri)
{
	busy = true;
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->SetURL(uri);
	SetRequestHeaders(Request);
	Request->SetVerb("GET");
	Request->OnHeaderReceived().BindRaw(this, &HttpServiceRaw::HttpResponseHeaderReceived);
	Request->OnProcessRequestComplete().BindRaw(this, &HttpServiceRaw::HttpResponseComplete);
	Request->OnRequestProgress().BindRaw(this, &HttpServiceRaw::HttpResponseProgress);
	Request->ProcessRequest();
}
void HttpServiceRaw::HttpGet(FString uri, FString username, FString password)
{
	busy = true;
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->SetURL(uri);
	SetRequestHeaders(Request);
	Request->SetHeader(TEXT("UserName"), username);
	Request->SetHeader(TEXT("Password"), password);
	Request->SetVerb("GET");
	Request->OnHeaderReceived().BindRaw(this, &HttpServiceRaw::HttpResponseHeaderReceived);
	Request->OnProcessRequestComplete().BindRaw(this, &HttpServiceRaw::HttpResponseComplete);
	Request->OnRequestProgress().BindRaw(this, &HttpServiceRaw::HttpResponseProgress);
	Request->ProcessRequest();
}
void HttpServiceRaw::HttpPost(FString uri, FString username, FString password, FString payload, FString content)
{
	busy = true;
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->SetURL(uri);
	SetRequestHeaders(Request);
	Request->SetHeader(TEXT("UserName"), username);
	Request->SetHeader(TEXT("Password"), password);
	Request->SetHeader(TEXT("Palyload"), payload);
	Request->SetVerb("POST");
	Request->SetContentAsString(content);
	Request->OnProcessRequestComplete().BindRaw(this, &HttpServiceRaw::HttpResponseComplete);
	Request->OnRequestProgress().BindRaw(this, &HttpServiceRaw::HttpResponseProgress);
	Request->ProcessRequest();
}
void HttpServiceRaw::HttpPost(FString uri, FString username, FString password, FString payload, TArray<uint8>& content)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->SetURL(uri);
	SetRequestHeaders(Request);
	Request->SetHeader(TEXT("UserName"), username);
	Request->SetHeader(TEXT("Password"), password);
	Request->SetHeader(TEXT("Palyload"), payload);
	Request->SetVerb("POST");
	Request->SetContent(content);
	Request->OnProcessRequestComplete().BindRaw(this, &HttpServiceRaw::HttpResponseComplete);
	Request->OnRequestProgress().BindRaw(this, &HttpServiceRaw::HttpResponseProgress);
	Request->ProcessRequest();
}
void HttpServiceRaw::HttpResponseHeaderReceived(FHttpRequestPtr Request, const FString& headername, const FString& headervalue)
{
	if (headername.Equals("Content-Length"))
	{
		filesizeinserver = FCString::Atoi(*headervalue);
	}

}
void HttpServiceRaw::HttpResponseComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	////GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,TEXT("HttpResponseComplete"));
	if (!ResponseIsValid(Response, bWasSuccessful))
	{
		OnHtpResponseFailed.ExecuteIfBound();
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("DOWNLOAD FAILED"));
		return;
	}
	OnHttpResponseComplete.ExecuteIfBound(Response->GetContentAsString(), Response->GetContent(), extra);
	busy = false;
}
void HttpServiceRaw::HttpResponseProgress(FHttpRequestPtr Resquest, int32 a, int32 b)
{
//	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::FromInt(b).Append(FString(": b")));
	////GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, Resquest->GetAllHeaders());
	if (filesizeinserver <= 0)return;
	float percent_f = ((float)b*1000) / filesizeinserver;
	UMyBlueprintFunctionLibrary::CLogtofile(FString::FromInt(b).Append("b  ").Append(FString::FromInt(filesizeinserver).Append("filesizeinserver")));
	percent_f /= 10;
	FString thefloat = FString::SanitizeFloat(percent_f);
	UMyBlueprintFunctionLibrary::CLogtofile(thefloat.Append("thefloat"));
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, thefloat);
	if (OnHttpResponseProgress.IsBound())
	{

		while (percent_f > 100)
		{
			percent_f -= 100;
		}
		OnHttpResponseProgress.ExecuteIfBound(a, percent_f, extra);
	}
}