// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
DECLARE_DELEGATE_TwoParams(FOnchannelReceivedata, const TArray<uint8>&, const FString&);

/**
 * 
 */
typedef uint8 channelidtype;
class UTILITY_API UdpChannel
{
	friend class UdpChannelMannger;
	channelidtype channelid;
	TSharedPtr<class UdpChannelMannger, ESPMode::ThreadSafe> udpchannelmanager;
public:
	UdpChannel(channelidtype id);
	~UdpChannel();
	FOnchannelReceivedata unreliabledatareceiveddelegate;
	FOnchannelReceivedata reliabledatareceiveddelegate;
	void sendunreliable(const uint8* content, const int32& size);
	void sendreliable(const uint8* content, const int32& size);
	void sendunreliable(FString& serialized);
	void sendreliable(FString& serialized);

};
class UTILITY_API UdpChannelMannger
{
	friend class UdpChannel;
	TArray<uint8> realdata;

	class SuperUdpClient * superudpclient;
	static TSharedPtr<UdpChannelMannger, ESPMode::ThreadSafe> _msingleston;
	TMap<channelidtype, UdpChannel*> OnchannelReceivedatacallbackmap;
	//TMap<channelidtype, TSharedPtr<UdpChannel, ESPMode::ThreadSafe>> OnchannelReceivedatacallbackmap;
public:
	UdpChannelMannger();
	~UdpChannelMannger();
	static TSharedPtr<UdpChannelMannger, ESPMode::ThreadSafe> Getsingleston();
	bool DestoryChannel(channelidtype channelid);
};
