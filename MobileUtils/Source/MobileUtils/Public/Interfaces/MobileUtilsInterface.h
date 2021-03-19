// Mobile Utils Plugin
// Created by Patryk Stepniewski
// Copyright (c) 2014-2019 gameDNA Ltd. All Rights Reserved.

#pragma once

#include "EngineMinimal.h"
#include "Core.h"

typedef TSharedPtr<class FJsonObject> JsonSharedPtr;
typedef void (*MessageCallback)(int message, const JsonSharedPtr jsonObject);

class IMobileUtilsInterface
{
public:
	IMobileUtilsInterface() : messageCallback(NULL) {}

	virtual void SetOrientation(int orientation) = 0;
	virtual void StartActivity(const FString& activity) = 0;
	virtual void DispatchMessage(int message, const JsonSharedPtr jsonObject) = 0;
	virtual FString ConvertToAbsolutePath(const FString& path) = 0;
	virtual FString getlocalipaddress() { return ""; };
	void SetMessageCallback(MessageCallback callback)
	{
		messageCallback = callback;
	}
	MessageCallback GetMessageCallback()
	{
		return messageCallback;
	}
	void CallMessageCallback(int message, const JsonSharedPtr jsonObject)
	{
		MessageCallback callback = messageCallback;
		FFunctionGraphTask::CreateAndDispatchWhenReady([callback, message, jsonObject]()
		{
			if (callback != NULL)
				callback(message, jsonObject);
		}, TStatId(), NULL, ENamedThreads::GameThread);
	}

private:
	MessageCallback messageCallback;
};
