// Fill out your copyright notice in the Description page of Project Settings.

#define UTF16
#include "UdpClient.h"
#include "Networking.h"
#include "Engine.h"
#include "MyBlueprintFunctionLibrary.h"
#include "RunnableThreadx.h"
#include "MobileUtilsBlueprintLibrary.h"
UdpClient::UdpClient()
{
	//SenderSocket = FUdpSocketBuilder(FString("normal"))
	//	.AsReusable()
	//	//.WithBroadcast()
	//	;
	
}
void UdpClient::exit()
{
	if (Sendreceivethread)
	{
		Sendreceivethread->StopThread();
	}
	if (Receivereceivethread)
	{
		Receivereceivethread->StopThread();
	}
	if (ReceiveSocket)
	{
		ReceiveSocket->Close();
	}
	if (SenderSocket)
	{
		SenderSocket->Close();
	}
	exitthread = true;
	delete this;
}
UdpClient::~UdpClient()
{
	if (Sendreceivethread)
	{
		delete Sendreceivethread;
	}
	if (Receivereceivethread)
	{
		delete Receivereceivethread;
	}
}
FString UdpClient::listen(int32 port)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////// only work on windows
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get();
	TSharedRef<FInternetAddr> Addr = SocketSubsystem->GetLocalBindAddr(*GLog);
	Addr->SetPort(port);
	ReceiveSocket = ISocketSubsystem::Get()->CreateSocket(NAME_DGram, TEXT("default receive"), Addr->GetProtocolType());
	validport = SocketSubsystem->BindNextPort(ReceiveSocket, *Addr, 1000, 1);
	//Bind to our listen port
   //if (!ReceiveSocket->Bind(*Addr))
   //{
   //	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::FromInt(1).Append(" bind failed"));
   //	ISocketSubsystem::Get()->DestroySocket(ReceiveSocket);
   //	ReceiveSocket = nullptr;
   //	UE_LOG(LogTemp, Warning, TEXT("Failed to bind to the listen port (%s) for LiveLink face AR receiving with error (%s)"),
   //		*Addr->ToString(true), ISocketSubsystem::Get()->GetSocketError());
   //	check(ReceiveSocket);
   //	return"";
   //}
   //else
   //{
   //	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::FromInt(1).Append(" bind ok"));
   //}
   //Async(EAsyncExecution::ThreadPool, [=]() {ReceiveReceiveWork(); }, nullptr);
	Receivereceivethread = new RunnableThreadx([=]() {
		uint32 BytesPending = 0;
		FPlatformProcess::Sleep(0.015);
		while (ReceiveSocket->HasPendingData(BytesPending))
		{
			ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get();
			TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();
			RecvBuffer.Empty();
			RecvBuffer.AddUninitialized(BytesPending);
			int32 BytesRead = 0;
			if (ReceiveSocket->RecvFrom(RecvBuffer.GetData(), BytesPending, BytesRead, *Sender))
			{
				OnUdpServerReceiveddata.ExecuteIfBound(RecvBuffer);
			}
		}
		});
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////only work on windows
	//bool bCanBindAll;
	//TSharedPtr<class FInternetAddr> Addr1 = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBindAll);
	//FString MyIP = Addr1->ToString(false);
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, MyIP);
	return UMobileUtilsBlueprintLibrary::Getlocalipaddress();

}
void UdpClient::setserveraddress(FString ipaddress, int32 port)
{
	bool bIsValid;
	RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	RemoteAddr->SetIp(*ipaddress, bIsValid);
	RemoteAddr->SetPort(port);
	//GEngine->AddOnScreenDebugMessage(-1, 51.0f, FColor::Yellow, ipaddress+ " : "+FString::FromInt(port));

	if (!bIsValid)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 51.0f, FColor::Yellow, TEXT("UDP Sender>> IP address was not valid!"));
		return;
	}
	protocoltype = RemoteAddr->GetProtocolType();
	SenderSocket = ISocketSubsystem::Get()->CreateSocket(NAME_DGram, TEXT("default send"), protocoltype /*FName("IPv4")*/);
	SenderSocket->SetReuseAddr();
	SenderSocket->SetNonBlocking();
	//SenderSocket->SetBroadcast();

	//int32 SendSize = 65507;
	//SenderSocket->SetSendBufferSize(SendSize, SendSize);
	//SenderSocket->SetReceiveBufferSize(SendSize, SendSize);
	//Async(EAsyncExecution::ThreadPool, [=]() {SendReceiveWork(); }, nullptr);
	Sendreceivethread = new RunnableThreadx([=]() {
		FPlatformProcess::Sleep(0.015);


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
			//SenderSocket->GetPeerAddress(*RemoteAddr.Get());
			SenderSocket->Recv(&datareceive[0], datasize, bytes, ESocketReceiveFlags::None);
			OnUdpClientReceiveddata.ExecuteIfBound(datareceive);

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
		});
}
//bool UdpClient::Send(FString& serialized)
//{
//	bool successful = false;
//	int32 sent = 0;
//	uint8* pointer;
//#ifdef UTF16
//	int64 outsize;
//	UMyBlueprintFunctionLibrary::FStringtoUTF16(serialized, pointer, outsize);
//	successful = Send(pointer, outsize);
//#else
//	TCHAR* serializedChar = serialized.GetCharArray().GetData();
//	int32 size = FCString::Strlen(serializedChar);
//	pointer = (uint8*)TCHAR_TO_UTF8(serializedChar);
//	successful = Send(pointer, size);
//#endif // SENDUTF16
//	return successful;
//}
bool UdpClient::Send(const uint8* content, const int32& size)
{

	int32 BytesSent = 0; 
	bool bs = SenderSocket->SendTo(content,size, BytesSent, *RemoteAddr);
	return bs;
}

SuperUdpClient::SuperUdpClient()
{
	header.Add(0xaa);
	header.Add(0x55);
	header.Add(0xaa);
	header.Add(0x55);
	header.Add(0xaa);
	header.Add(0x55);
	header.Add(0xaa);
	header.Add(0x55);
	//int realheadersize = header.Num();
	//header.SetNum(realheadersize + 4);
	//FMemory::Memcpy(header.GetData()+realheadersize,(uint8*)(&id), 4);//here is 2 byte valide data length
	udpclient = new UdpClient();
	udpclient->OnUdpClientReceiveddata.BindLambda([this](const TArray<uint8>& data) {
			if (isvalidedata(data))
			{
				int32 datasize = data.Num();
				int32 headersize = header.Num();
				uint8 un_reliablebyte = data[headersize];
				if (un_reliablebyte == UNRELIABLESIGN)//unreliable data
				{      
					uint8 normal_ackbyte = data[headersize + 1];
					if (normal_ackbyte == NORMALDATASIGN)
					{
						                // +unreliable + ack/normaldata
						int32 commandsize = headersize + 1 + 1;
						int32 playloadsize;// = datasize - commandsize;
						playloadsize = *(uint16*)(data.GetData() + commandsize);//here is 2 byte valide data length
						realdata.SetNum(playloadsize);
						FMemory::Memcpy(realdata.GetData(), data.GetData() + commandsize + 2, playloadsize);//here is 2 byte valide data length
						unreliabledatareceiveddelegate.ExecuteIfBound(realdata);
/*
#ifdef UTF16
						FString datatostring = FString(realdata.Num() >> 1, (TCHAR*)&realdata[0]);
						UMyBlueprintFunctionLibrary::CLogtofile(datatostring);					
#else
						FString datatostring = FString(UTF8_TO_TCHAR(&realdata[0])).Left(realdata.Num());
#endif // SENDUTF16
						unreliabledatareceiveddelegate.ExecuteIfBound(realdata, datatostring);
*/
					}
					else if (normal_ackbyte == ACKSIGN)
					{
						receiveackid = data[headersize + 2];
						GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString("receiveackid").Append(FString::FromInt(receiveackid)));

					}

				}
				else if (un_reliablebyte == RELIABLESIGN)
				{
					    
					    uint8 messageid = data[headersize + 1];
						bool bspecial = messageid == 0 && lastsendackid == 0xff;
						if (bspecial ||messageid > lastsendackid)
						{
							lastsendackid = messageid;
							//+ reliable  + id
							int32 commandsize = headersize + 1 + +1;
							int32 playloadsize;// = datasize - commandsize;
							playloadsize = *(uint16*)(data.GetData() + commandsize);//here is 2 byte valide data length
							realdata.SetNum(playloadsize);
							FMemory::Memcpy(realdata.GetData(), data.GetData() + commandsize + 2, playloadsize);//here is 2 byte valide data length
							reliabledatareceiveddelegate.ExecuteIfBound(realdata);

/*
#ifdef UTF16
							FString datatostring = FString(realdata.Num() >> 1, (TCHAR*)&realdata[0]);
							UMyBlueprintFunctionLibrary::CLogtofile(datatostring);
#else
							FString datatostring = FString(UTF8_TO_TCHAR(&realdata[0])).Left(realdata.Num());
#endif // SENDUTF16
							reliabledatareceiveddelegate.ExecuteIfBound(realdata, datatostring);
*/
						}
						sendack(messageid);

				}
			}
		});
	 Async(EAsyncExecution::ThreadPool, [=]() {
			reliabletick();				
		}, nullptr);
}

SuperUdpClient::~SuperUdpClient()
{
	udpclient->exitthread = true;
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString("receiveackid").Append(FString::FromInt(receiveackid)));

}
//void SuperUdpClient::sendunreliable(FString& serialized)
//{
//	int32 sent = 0;
//	uint8* pointer;
//#ifdef UTF16
//	int64 outsize;
//	UMyBlueprintFunctionLibrary::FStringtoUTF16(serialized, pointer, outsize);
//	sendunreliable(pointer, outsize);
//#else
//	TCHAR* serializedChar = serialized.GetCharArray().GetData();
//	int32 size = FCString::Strlen(serializedChar);
//	pointer = (uint8*)TCHAR_TO_UTF8(serializedChar);
//	sendunreliable(pointer, size);
//#endif // SENDUTF16
//}
void SuperUdpClient::sendack(const uint8 messageid)
{
	TArray<uint8> tempcontent;
	tempcontent.Append(header); 
	tempcontent.Add(UNRELIABLESIGN);//mean unreliable
	tempcontent.Add(ACKSIGN);//mean this is ack	
	tempcontent.Add(messageid);
	udpclient->Send(tempcontent.GetData(), tempcontent.Num());
}
void SuperUdpClient::sendunreliable(const uint8* content, const int32& size)
{
	TArray<uint8> tempcontent;
	tempcontent.Append(header);
	tempcontent.Add(UNRELIABLESIGN);//mean unreliable
	tempcontent.Add(NORMALDATASIGN);//mean this is normal data	
	int headersize = tempcontent.Num();
	tempcontent.SetNum(headersize + 2 + size);// 2 mean 2 byte valide data length
	FMemory::Memcpy(tempcontent.GetData() + headersize, (uint8*)&size, 2);
	FMemory::Memcpy(tempcontent.GetData() + headersize+2, content, size);
	udpclient->Send(tempcontent.GetData(), headersize + 2 + size);
}
void SuperUdpClient::sendreliable(const uint8* content, const int32& size)
{
	TArray<uint8> tempcontent;
	tempcontent.Append(header);
	tempcontent.Add(RELIABLESIGN);//mean reliable
	tempcontent.Add(reliabledataid++);//mean reliable message id
	int headersize = tempcontent.Num();
	tempcontent.SetNum(headersize + 2 + size);// 2 mean 2 byte valide data length
	FMemory::Memcpy(tempcontent.GetData() + headersize, (uint8*)&size, 2);
	FMemory::Memcpy(tempcontent.GetData() + headersize + 2, content, size);
	m_mutex.Lock();
	Queuedreliabledata.Enqueue(tempcontent);
	m_mutex.Unlock();
}
//void SuperUdpClient::sendreliable(FString& serialized)
//{
//	int32 sent = 0;
//	uint8* pointer;
//#ifdef UTF16
//	int64 outsize;
//	UMyBlueprintFunctionLibrary::FStringtoUTF16(serialized, pointer, outsize);
//	sendreliable(pointer, outsize);
//#else
//	TCHAR* serializedChar = serialized.GetCharArray().GetData();
//	int32 size = FCString::Strlen(serializedChar);
//	pointer = (uint8*)TCHAR_TO_UTF8(serializedChar);
//	sendreliable(pointer, size);
//#endif // SENDUTF16
//}
void SuperUdpClient::reliabletick()
{
	while (true)
	{
		if (exitthread)
		{
			delete this;
			break;
		}
		m_mutex.Lock();
		TArray<uint8> NewAudioBuffer;
		bool b = Queuedreliabledata.Peek(NewAudioBuffer);
		if (b)
		{
			int32 headersize = header.Num();
			int32 idindex = headersize + 1;
			if (NewAudioBuffer[idindex] == receiveackid)
			{
				Queuedreliabledata.Pop();
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString("Queuedreliabledata.Pop()").Append(FString::FromInt(receiveackid)));
			}
		}
		b = Queuedreliabledata.Peek(NewAudioBuffer);
		if (b)
		{
			udpclient->Send(NewAudioBuffer.GetData(), NewAudioBuffer.Num());
		}
		m_mutex.Unlock();
		FPlatformProcess::Sleep(0.1f);
	}


}
bool SuperUdpClient::isvalidedata(const TArray<uint8>&data)
{
	bool b  = data[0] == 0xaa;
	bool b1 = data[1] == 0x55;
	bool b2 = data[2] == 0xaa;
	bool b3 = data[3] == 0x55;
	bool b4 = data[4] == 0xaa;
	bool b5 = data[5] == 0x55;
	bool b6 = data[6] == 0xaa;
	bool b7 = data[7] == 0x55;
	return b && b1&& b2&& b3&& b4&& b5&& b6&& b7;
}
