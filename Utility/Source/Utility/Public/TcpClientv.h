// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#define TCPSENDBUFFERSIZE 65536
#include "CoreMinimal.h"
#include "TcpClient.h"
#include "Smartinterface.h"
DECLARE_DELEGATE_TwoParams(FOnTcpClientReceivedatav1, const TArray<uint8>&, const FString &);

/**
 * 
 */
class UTILITY_API TcpClientv : public ISmartinterface
{

    
	TArray<uint8> datareceive;
	uint32 datasize;
	long counter = 0;
public:
	class FSocket* Socket;
	TcpClientv();
	~TcpClientv();
public:
	bool exitthread = false;
	bool Connecttoserver(uint8 a, uint8 b, uint8 c, uint8 d, uint32 port);
	bool Send(FString& serialized);
	bool Send(const uint8*content, const int32& size);
	void ReceiveWork();
	FOnTcpClientvReceiveddata OnTcpClientReceiveddata;
	FOnTcpClientReceivedatav1 OnTcpClientReceiveddatav1;
};
