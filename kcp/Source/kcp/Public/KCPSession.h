// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class KCP_API KCPSession
{
	class UdpServer* server;
public:
	KCPSession(UINT16 port, int& iResultp);
	~KCPSession();
	TFunction<void(class KCPChannel* channel)> onacceptchannel;
};
class KCP_API KCPChannel
{
	class KChannel* channel;
public:
	KCPChannel(class KChannel* pchannel);
	~KCPChannel();
	void send(const char* data, const UINT16& size);
	void setonUserLevelReceivedCompleted(TFunction<void(const uint8* data, const UINT16& size)> onc);
};
