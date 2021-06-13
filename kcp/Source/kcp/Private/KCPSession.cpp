// Fill out your copyright notice in the Description page of Project Settings.

#if PLATFORM_WINDOWS

#include "KCPSession.h"
#include "UdpServer.h"

KCPSession::KCPSession(uint16 port, int& iResultp)
{
	server = new UdpServer(port, iResultp);
	server->onacceptchannel = [=](KChannel* channel) {
		if (onacceptchannel)
		{
			onacceptchannel(new KCPChannel(channel));
		}
	};
}

KCPSession::~KCPSession()
{
	delete server;
}

KCPChannel::KCPChannel(KChannel* pchannel)
{
	channel = pchannel;
}
void KCPChannel::setonUserLevelReceivedCompleted(TFunction<void(const uint8* data, const uint16& size)> onc)
{
	channel->onUserLevelReceivedCompleted = onc;
}

KCPChannel::~KCPChannel()
{
	delete channel;
}
void KCPChannel::send(const char* data, const uint16& size)
{
	channel->Send(data, size);
}
#endif
