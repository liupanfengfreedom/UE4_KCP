// Fill out your copyright notice in the Description page of Project Settings.


#include "RunnableThreadx.h"
#include "Engine.h"

RunnableThreadx::RunnableThreadx(const TFunction<void(void)>& workerp)
{
	worker = workerp;

	m_Kill = false;
	m_Pause = false;
	Thread = FRunnableThread::Create(this, *FString::FromInt((uint64)this), 0, TPri_BelowNormal);
}

RunnableThreadx::~RunnableThreadx()
{
	if (Thread)
	{
		Stop();
		Thread->WaitForCompletion();
		//Thread->Kill();
		delete Thread;
		Thread = nullptr;
	}
}
void RunnableThreadx::PauseThread()
{
	m_Pause = true;
}
void RunnableThreadx::ContinueThread()
{
	m_Pause = false;
}
void RunnableThreadx::StopThread()
{
	Stop();
	if (Thread)
	{
		Thread->WaitForCompletion();
		//Thread->Kill();
	}

}
bool  RunnableThreadx::IsThreadPaused()
{
	return (bool)m_Pause;
}
bool  RunnableThreadx::IsThreadKilled()
{
	return (bool)m_Kill;
}

bool  RunnableThreadx::Init()
{
	////GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("RunnableThreadxInit"));
	return true;
}
uint32  RunnableThreadx::Run()
{
	FPlatformProcess::Sleep(0.03);

	while (StopTaskCounter.GetValue() == 0 && !m_Kill)
	{
		if (m_Pause)
		{
			FPlatformProcess::Sleep(0.1);
			////GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("AudioRecordThreadPause"));
			if (m_Kill)
			{
				return 0;
			}
		}
		else
		{
			////GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("AudioRecordThreadLoop"));

			//m_mutex.Lock();
			////需要同步处理的内容

			//m_mutex.Unlock();
			worker();
			//threadworker.ExecuteIfBound();
			//FPlatformProcess::Sleep(0.03);
		}
	}
	return 0;

}
void RunnableThreadx::Stop()
{
	StopTaskCounter.Increment();
	m_Kill = true;
	m_Pause = false;
	////GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("AudioRecordThreadStop"));

}

void RunnableThreadx::Exit()
{
	////GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("AudioRecordThreadExit"));
}