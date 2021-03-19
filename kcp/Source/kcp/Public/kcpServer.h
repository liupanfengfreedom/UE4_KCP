// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ikcp.h"
DECLARE_DELEGATE_TwoParams(FOnKcpReceivedata, const uint8*, uint32);
DECLARE_DELEGATE_FourParams(FOnRawsend, const char*, int, struct IKCPCB*, void*);
/**
 * 
 */
class KCP_API kcpServer
{
	TSharedPtr<class UdpServerv1, ESPMode::ThreadSafe> us;
	ikcpcb* kcp1;
	static TMap<void*, FOnRawsend> onrawsendevents;
	void ReceiveWork();
	class RunnableThreadx* receivethread = nullptr;

	IUINT32 nexttimecallupdate = 0;
	IUINT32 currenttime = 0;
	uint8 kcpreceive[0xffff] = { 0 };

public:
	kcpServer(UINT16 port, int& iResultp);
	~kcpServer();
	FOnKcpReceivedata OnkcpReceiveddata;
	void kcpsend(const uint8* buffer, int len);
	void kcpsend(const FString& msg);
	void close();

};
