// Fill out your copyright notice in the Description page of Project Settings.


#include "TcpClient.h"
#include "Runtime/Sockets/Public/SocketSubsystem.h"
#include "Runtime/Networking/Public/Interfaces/IPv4/IPv4Address.h"
#include "Runtime/Sockets/Public/IPAddress.h"
#include "Runtime/Sockets/Public/Sockets.h"
#include "Engine.h"


void UTcpClient::BeginDestroy()
{
	Super::BeginDestroy();
	exitthread = true;
}

UTcpClient::UTcpClient()
{	
	Socket = ISocketSubsystem::Get()->CreateSocket(NAME_Stream, TEXT("default"), false);
}

bool UTcpClient::Connecttoserver(uint8 a, uint8 b, uint8 c, uint8 d, uint32 port)
{
	FIPv4Address ip(a, b, c,d);

	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get()->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(port);
	bool connected = Socket->Connect(*addr);
	if (connected)
	{
		Async(EAsyncExecution::ThreadPool, [=]() {ReceiveWork(); }, nullptr);
	}
	return connected;
}

bool UTcpClient::Send(FString & serialized)
{
	TCHAR *serializedChar = serialized.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar);
	int32 sent = 0;
	bool successful = Socket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
	return successful;
}

bool UTcpClient::Send(const uint8 * content, const int32 &size)
{
	int32 sent = 0;
	bool successful = Socket->Send(content, size, sent);
	return successful;
}

void UTcpClient::ReceiveWork()
{

	while (true)
	{
		////GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("UTcpClient threadworker"));
		FPlatformProcess::Sleep(period);
		if (Socket == nullptr)break;
		bool b = Socket->HasPendingData(datasize);

		int32 bytes;
		if (b)
		{
			period = 0.02;
			datareceive.AddUninitialized(datasize);
			Socket->Recv(&datareceive[(counter++) * 65536], datasize, bytes, ESocketReceiveFlags::None);
		}
		else
		{
			if (datareceive.Num() > 0)
			{                                                 //the string may should not be trust
				OnTcpClientReceiveddata.Broadcast(datareceive, FString(UTF8_TO_TCHAR(&datareceive[0])).Left(datareceive.Num()), extra);
				datareceive.Empty();
				period = 0.1;
			}
		}
		if (exitthread)
		{
			break;
		}
	}
}
