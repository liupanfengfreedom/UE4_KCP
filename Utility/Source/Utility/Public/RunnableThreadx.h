// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Runtime/Core/Public/HAL/ThreadSafeBool.h"
#include "CoreMinimal.h"

/**
 * 
 */
class UTILITY_API RunnableThreadx : public FRunnable
{
public:
	TFunction<void(void)> worker;
	RunnableThreadx(const TFunction<void(void)>& workerp);
	~RunnableThreadx();
	//暂停线程
	void PauseThread();
	//继续线程
	void ContinueThread();
	//停止线程
	void StopThread();

	bool IsThreadPaused();
	bool IsThreadKilled();
	FCriticalSection m_mutex;
private:
	FRunnableThread* Thread;
	FThreadSafeCounter StopTaskCounter;

	//override Frunnable Function
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

private:
	FThreadSafeBool m_Kill;
	FThreadSafeBool m_Pause;
};
