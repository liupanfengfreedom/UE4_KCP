// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TcpClient.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTcpClientReceiveddata, const TArray<uint8>&, data, const FString&, str, UObject*, extra);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTcpClientvReceiveddata, const TArray<uint8>&, data, const FString&, str);

/**
 * 
 */
UCLASS()
class UTILITY_API UTcpClient : public UObject
{
	GENERATED_BODY()
	bool exitthread = false;
	class FSocket* Socket;
	TArray<uint8> datareceive;
	uint32 datasize;
	long counter = 0;
	float period = 0.1;
protected:
	// Called when the game starts or when spawned
	virtual void BeginDestroy() override;
public:
	UObject* extra;
public:
	UTcpClient();
	bool Connecttoserver(uint8 a, uint8 b, uint8 c, uint8 d, uint32 port);
	bool Send(FString& content);
	bool Send(const uint8*content,const int32& size);
	void ReceiveWork();
	FOnTcpClientReceiveddata OnTcpClientReceiveddata;
};
