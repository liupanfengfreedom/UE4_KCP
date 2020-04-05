// Fill out your copyright notice in the Description page of Project Settings.

#define UTF16
#include "UdpClient.h"
#include "Networking.h"
#include "Engine.h"
#include "MyBlueprintFunctionLibrary.h"
#include "Misc/DateTime.h"
FOnRawsend UdpClient::onrawsendevent;
UdpClient::UdpClient()
{
	SenderSocket = FUdpSocketBuilder(FString("normal"))
		.AsReusable()
		//.WithBroadcast()
		;
}

UdpClient::~UdpClient()
{
	ikcp_release(kcp1);
}
void UdpClient::setserveraddress(FString ipaddress, int32 port)
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
	kcp1 = ikcp_create(0x11223344, (void*)0);
	ikcp_wndsize(kcp1, 128, 128);
	int mode = 0;
	// 判断测试用例的模式
	if (mode == 0) {
		// 默认模式
		ikcp_nodelay(kcp1, 0, 10, 0, 0);
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
		onrawsendevent.ExecuteIfBound(buf,len,kcp,user);
		return 0;
	};
	onrawsendevent.BindLambda([=](const char* buf, int len, struct IKCPCB* kcp, void* user){
		int32 BytesSent = 0;
		bool bs = SenderSocket->SendTo((const uint8*)buf, len, BytesSent, *RemoteAddr);
		});
}
void UdpClient::kcpsend(ikcpcb* kcp, const uint8* buffer, int len)
{
	ikcp_send(kcp1, (const char*)buffer, len);
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
void UdpClient::ReceiveWork()
{
	//UMyBlueprintFunctionLibrary::CLogtofile(FString("ReceiveWork() ok"));

	while (true)
	{
		ikcp_update(kcp1, FDateTime::UtcNow().ToUnixTimestamp());
		int hr = ikcp_recv(kcp1, (char*)kcpreceive, 10);
		if (hr > 0)
		{
			OnkcpReceiveddata.ExecuteIfBound(kcpreceive,hr);
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

			SenderSocket->Recv(&datareceive[0], datasize, bytes, ESocketReceiveFlags::None);
			OnUdpClientReceiveddata.ExecuteIfBound(datareceive);
			ikcp_input(kcp1,(const char*)&datareceive[0], datasize);

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
