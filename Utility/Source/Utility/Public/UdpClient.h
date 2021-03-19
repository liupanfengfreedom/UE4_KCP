// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
/**
 * 
 */
DECLARE_DELEGATE_OneParam(FOnUdpClientReceivedata, const TArray<uint8>&);
class UTILITY_API UdpClient
{
	class RunnableThreadx* Sendreceivethread = nullptr;
	class RunnableThreadx* Receivereceivethread = nullptr;
	TSharedPtr<class FInternetAddr>	RemoteAddr;
	class FSocket* SenderSocket = nullptr;
	class FSocket* ReceiveSocket = nullptr;
	//void SendReceiveWork();
	//void ReceiveReceiveWork();
	uint32 datasize;
	TArray<uint8> datareceive;
	TArray<uint8> RecvBuffer;
	FName protocoltype;
public:
	bool exitthread = false;
	int32 validport = 0;
	void exit();
	UdpClient();
	~UdpClient();
	void setserveraddress(FString ipaddress, int32 port);
	FString listen(int32 port);
	bool Send(const FString& serialized);
	bool Send(const uint8* content, const int32& size);
	FOnUdpClientReceivedata OnUdpClientReceiveddata;
	FOnUdpClientReceivedata OnUdpServerReceiveddata;
};
class UTILITY_API SuperUdpClient
{
	uint8 UNRELIABLESIGN = 0x33;
	uint8 RELIABLESIGN = 0xee;
	uint8 ACKSIGN = 0x0e;
	uint8 NORMALDATASIGN   = 0xdd;
	TArray<uint8> realdata;
	TArray<uint8> header;
	TQueue<TArray<uint8>> Queuedreliabledata;
	uint8 reliabledataid=0;
	bool isvalidedata(const TArray<uint8>&data);
	uint8 receiveackid = 0xff;
	uint8 lastsendackid= 0xff;
	FCriticalSection m_mutex;
	void sendack(const uint8 messageid);

public:
	class UdpClient* udpclient;
	uint32 id;
public:
	SuperUdpClient();
	~SuperUdpClient();
	//void sendunreliable(FString& serialized);
	void sendunreliable(const uint8* content, const int32& size);
	void sendreliable(const uint8* content, const int32& size);
	//void sendreliable(FString& serialized);

	void reliabletick();
	FOnUdpClientReceivedata unreliabledatareceiveddelegate;
	FOnUdpClientReceivedata reliabledatareceiveddelegate;
public:
	bool exitthread = false;

};
