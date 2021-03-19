// Fill out your copyright notice in the Description page of Project Settings.
//some string change to utf8 will enconter error
#define UTF16
#include "TcpClientv.h"
#include "Runtime/Sockets/Public/SocketSubsystem.h"
#include "Runtime/Networking/Public/Interfaces/IPv4/IPv4Address.h"
#include "Runtime/Sockets/Public/IPAddress.h"
#include "Runtime/Sockets/Public/Sockets.h"
#include "Engine.h"
#include "MyBlueprintFunctionLibrary.h"
TcpClientv::TcpClientv()
{
	Socket = ISocketSubsystem::Get()->CreateSocket(NAME_Stream, TEXT("default"), false);
	int newsize;
	bool b = Socket->SetSendBufferSize(TCPSENDBUFFERSIZE, newsize);
	if (b)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString("sendbuffersize  for this os ok "));
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString("sendbuffersize  for this os failed "));
	}
	b = Socket->SetReceiveBufferSize(65536,newsize);
	if (b)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString("Receivebuffersize  for this os ok "));
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString("Receivebuffersize  for this os failed "));
	}
}

TcpClientv::~TcpClientv()
{
	UMyBlueprintFunctionLibrary::CLogtofile(FString("~TcpClientv()"));
}

bool TcpClientv::Connecttoserver(uint8 a, uint8 b, uint8 c, uint8 d, uint32 port)
{
	FIPv4Address ip(a, b, c, d);

	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get()->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(port);
	bool connected = Socket->Connect(*addr);
	if (connected)
	{
		UMyBlueprintFunctionLibrary::CLogtofile(FString("connected ok"));
		Async(EAsyncExecution::ThreadPool, [=]() {ReceiveWork(); }, nullptr);
	}
	return connected;
}

bool TcpClientv::Send(FString & serialized)
{
	bool successful = false;
	int32 sent = 0;
	uint8*pointer;
#ifdef UTF16
	int64 outsize;
	UMyBlueprintFunctionLibrary::FStringtoUTF16(serialized, pointer, outsize);
	successful = Socket->Send(pointer, outsize, sent);
#else
	TCHAR *serializedChar = serialized.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar);
	pointer = (uint8*)TCHAR_TO_UTF8(serializedChar);
	successful = Socket->Send(pointer, size, sent);
#endif // SENDUTF16
	return successful;
}
bool TcpClientv::Send(const uint8 * content, const int32 & size)
{
	int32 sent = 0;
	bool successful = Socket->Send(content, size, sent);
	return successful;
}

void TcpClientv::ReceiveWork()
{
	UMyBlueprintFunctionLibrary::CLogtofile(FString("ReceiveWork() ok"));

	while (true)
	{
		if (exitthread)
		{
			UMyBlueprintFunctionLibrary::CLogtofile(FString("exitthread"));
			Socket->Close();
			delete this;
			break;
		}
		if (Socket == nullptr)
		{
			UMyBlueprintFunctionLibrary::CLogtofile(FString("Socket == nullptr"));
			delete this;
			break;
		}
		////GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("UTcpClient threadworker"));
		FPlatformProcess::Sleep(0.03);
		ESocketConnectionState state = Socket->GetConnectionState();
		if (state != ESocketConnectionState::SCS_Connected)
		{
			UMyBlueprintFunctionLibrary::CLogtofile(FString("SCS_ConnectionError"));
			Socket->Close();
			delete this;
			break;
		}
	
		bool b = false;
		if (state == ESocketConnectionState::SCS_Connected)
		{
			b = Socket->HasPendingData(datasize);
		}
		int32 bytes;
		if (b)
		{
			UMyBlueprintFunctionLibrary::CLogtofile(FString(" Socket->HasPendingData(datasize) is ture"));
			// u need to ensure single package not bigger than 65536 byte
			// and the time gap between two packages is needed and at least 30ms 
			datareceive.Empty();
			datareceive.AddUninitialized(datasize);
			UMyBlueprintFunctionLibrary::CLogtofile(FString(" datasize：").Append(FString::FromInt(datasize)));
			//Socket->Recv(&datareceive[(counter++) * 65536], datasize, bytes, ESocketReceiveFlags::None);
			//UMyBlueprintFunctionLibrary::CLogtofile(FString(UTF8_TO_TCHAR(&datareceive[0])).Left(datareceive.Num()));
			Socket->Recv(&datareceive[0], datasize, bytes, ESocketReceiveFlags::None);
			//UMyBlueprintFunctionLibrary::CLogtofile(FString(UTF8_TO_TCHAR(&datareceive[0])).Left(datareceive.Num()));
#ifdef UTF16
			FString datatostring = FString(datareceive.Num() >> 1, (TCHAR*)&datareceive[0]);
			UMyBlueprintFunctionLibrary::CLogtofile(datatostring);
			//OnTcpClientReceiveddata.Broadcast(datareceive, FString(datareceive.Num() >> 1, (TCHAR*)&datareceive[0]));
			OnTcpClientReceiveddata.Broadcast(datareceive, datatostring);
			OnTcpClientReceiveddatav1.ExecuteIfBound(datareceive, datatostring);
#else
			FString datatostring = FString(UTF8_TO_TCHAR(&datareceive[0])).Left(datareceive.Num());
			//OnTcpClientReceiveddata.Broadcast(datareceive, FString(UTF8_TO_TCHAR(&datareceive[0])).Left(datareceive.Num()));
			OnTcpClientReceiveddata.Broadcast(datareceive,datatostring);
			OnTcpClientReceiveddatav1.ExecuteIfBound(datareceive,datatostring);
#endif // SENDUTF16
		}
		//else
		//{
		//	UMyBlueprintFunctionLibrary::CLogtofile(FString("if (datareceive.Num() > 0) before"));

		//	if (datareceive.Num() > 0)
		//	{             
		//		UMyBlueprintFunctionLibrary::CLogtofile(FString(UTF8_TO_TCHAR(&datareceive[0])).Left(datareceive.Num()));
		//	                                             //the string may should not be trust
		//		OnTcpClientReceiveddata.Broadcast(datareceive, FString(UTF8_TO_TCHAR(&datareceive[0])).Left(datareceive.Num()));
		//		datareceive.Empty();	
		//      counter = 0;
		//	}
		//	UMyBlueprintFunctionLibrary::CLogtofile(FString("if (datareceive.Num() > 0) after"));

		//}
	}
}
