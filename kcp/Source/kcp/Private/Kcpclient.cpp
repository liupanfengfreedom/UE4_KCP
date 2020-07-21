// Fill out your copyright notice in the Description page of Project Settings.


#include "Kcpclient.h"
#include "Networking/Public/Networking.h"
#include "Engine.h"
FOnRawsend Kcpclient::onrawsendevent;
Kcpclient::Kcpclient()
{
	SenderSocket = FUdpSocketBuilder(FString("normal"))
		.AsReusable()
		//.WithBroadcast()
		;
}

Kcpclient::~Kcpclient()
{
	if (kcp1)
	{
		ikcp_release(kcp1);
		kcp1 = nullptr;
	}
}
void Kcpclient::setserveraddress(FString ipaddress, int32 port)
{
	bool bIsValid;
	RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	RemoteAddr->SetIp(*ipaddress, bIsValid);
	RemoteAddr->SetPort(port);

	if (!bIsValid)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("UDP Sender>> IP address was not valid!"));
		return;
	}
	int32 SendSize = 65507;
	SenderSocket->SetSendBufferSize(SendSize, SendSize);
	SenderSocket->SetReceiveBufferSize(SendSize, SendSize);
	Async(EAsyncExecution::ThreadPool, [=]() {ReceiveWork(); }, nullptr);
	////////////////////////////////////////////////////////

	kcp1 = ikcp_create(5598781, (void*)0);

	int mode = 0;
	// 判断测试用例的模式
	if (mode == 0) {
		// 默认模式
		ikcp_nodelay(kcp1, 1, 20, 2, 1);
		ikcp_wndsize(kcp1, 64, 64);
		ikcp_setmtu(kcp1, 512);
	}
	else if (mode == 1) {
		// 普通模式，关闭流控等
		ikcp_nodelay(kcp1, 0, 10, 0, 1);
	}
	else {
		// 启动快速模式
		// 第二个参数 nodelay-启用以后若干常规加速将启动
		// 第三个参数 interval为内部处理时钟，默认设置为 10ms
		// 第四个参数 resend为快速重传指标，设置为2
		// 第五个参数 为是否禁用常规流控，这里禁止
		ikcp_nodelay(kcp1, 2, 10, 2, 1);
		kcp1->rx_minrto = 10;
		kcp1->fastresend = 1;
	}
	kcp1->output = [](const char* buf, int len, struct IKCPCB* kcp, void* user)->int {
		onrawsendevent.ExecuteIfBound(buf, len, kcp, user);
		return 0;
	};
	onrawsendevent.BindLambda([=](const char* buf, int len, struct IKCPCB* kcp, void* user) {
		int32 BytesSent = 0;
		bool bs = SenderSocket->SendTo((const uint8*)buf, len, BytesSent, *RemoteAddr);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("SenderSocket->SendTo"));
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,FString::FromInt(len));

		});
}
void Kcpclient::kcpsend(const uint8* buffer, int len)
{
	ikcp_send(kcp1, (const char*)buffer, len);
}
void Kcpclient::kcpsend( FString& msg)
{
	//uint8* pointer;
#ifdef UTF16
	int64 outsize;
	UMyBlueprintFunctionLibrary::FStringtoUTF16(msg, pointer, outsize);
	kcpsend((const uint8*)pointer, outsize);
#else
	TCHAR* serializedChar = msg.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar);
	//pointer = (uint8*)TCHAR_TO_UTF8(serializedChar);
	kcpsend((const uint8*)TCHAR_TO_UTF8(serializedChar), size);

#endif // SENDUTF16
}
#define TIMEINTERNAL  0.015f
#define TIMEINTERNALINMILLISECOND  TIMEINTERNAL*1000*1

void Kcpclient::ReceiveWork()
{
	//UMyBlueprintFunctionLibrary::CLogtofile(FString("ReceiveWork() ok"));
	IUINT32 nexttimecallupdate = 0;
	IUINT32 currenttime = 0;
	while (true)
	{
		static int tickcounter = 0;
		currenttime = FDateTime::UtcNow().ToUnixTimestamp()*1000;
		tickcounter += TIMEINTERNALINMILLISECOND;
		currenttime += tickcounter;
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::FromInt(currenttime).Append(" :currenttime"));
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::FromInt(nexttimecallupdate).Append(" :nexttimecallupdate"));
		if (currenttime >= nexttimecallupdate)
		{
			ikcp_update(kcp1, currenttime);
			nexttimecallupdate = ikcp_check(kcp1, currenttime);
			tickcounter = 0;
		}
		//ikcp_update(kcp1, FDateTime::UtcNow().ToUnixTimestamp() * 100);
		int hr = ikcp_recv(kcp1, (char*)kcpreceive, sizeof(kcpreceive));
		if (hr > 0)
		{
  			OnkcpReceiveddata.ExecuteIfBound(kcpreceive, hr);
		}
		if (exitthread)
		{
			//UMyBlueprintFunctionLibrary::CLogtofile(FString("exitthread"));
			SenderSocket->Close();
			delete this;
			break;
		}
		if (SenderSocket == nullptr)
		{
			//UMyBlueprintFunctionLibrary::CLogtofile(FString("Socket == nullptr"));
			delete this;
			break;
		}
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("UTcpClient threadworker"));
		FPlatformProcess::Sleep(TIMEINTERNAL);


		bool b = false;
		b = SenderSocket->HasPendingData(datasize);

		int32 bytes;
		if (b)
		{
			//UMyBlueprintFunctionLibrary::CLogtofile(FString(" Socket->HasPendingData(datasize) is ture"));
			// u need to ensure single package not bigger than 65536 byte
			// and the time gap between two packages is needed and at least 30ms 
			datareceive.Empty();
			datareceive.AddUninitialized(datasize);

			SenderSocket->Recv(&datareceive[0], datasize, bytes, ESocketReceiveFlags::None);
			//OnUdpClientReceiveddata.ExecuteIfBound(datareceive);
			ikcp_input(kcp1, (const char*)&datareceive[0], datasize);

			/*
			#ifdef UTF16
						FString datatostring = FString(datareceive.Num() >> 1, (TCHAR*)&datareceive[0]);
						UMyBlueprintFunctionLibrary::CLogtofile(datatostring);
						OnUdpClientReceiveddata.ExecuteIfBound(datareceive, datatostring);
			#else
						FString datatostring = FString(UTF8_TO_TCHAR(&datareceive[0])).Left(datareceive.Num());
						OnUdpClientReceiveddata.ExecuteIfBound(datareceive, datatostring);
			#endif // SENDUTF16
			*/
		}
	}
}