// Fill out your copyright notice in the Description page of Project Settings.


#include "Kcpclient.h"
#include "Networking/Public/Networking.h"
#include "Engine.h"
#include "RunnableThreadx.h"
#include <chrono>
//#define UTF16
Kcpclient::Kcpclient()
{
	TSharedPtr<class FInternetAddr>	RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	Socket = ISocketSubsystem::Get()->CreateSocket(NAME_DGram, TEXT("default send"), RemoteAddr->GetProtocolType() /*FName("IPv4")*/);
	Socket->SetReuseAddr();
	Socket->SetNonBlocking();
	receivethread = new RunnableThreadx([=]() {
		FPlatformProcess::Sleep(0.010);

		uint32 datasize;
		bool b = false;
		b = Socket->HasPendingData(datasize);


		int32 bytes;
		if (b)
		{
			Socket->Recv(&datareceive[0],receivebuffersize, bytes, ESocketReceiveFlags::None);
            uint32 conn = *(uint32*)datareceive;
            switch (conn)
            {
            case KcpProtocalType::SYN:
                if (bytes != 8)
                {
                    break;
                }
              //  HandleAccept((const SOCKADDR*)&SenderAddr, (const char*)RecvBuf, iResult);
                break;
            case KcpProtocalType::ACK:
                if (bytes != 12)
                {
                    break;
                }
                HandleConnect((const char*)datareceive, bytes);
                break;
            case KcpProtocalType::PING:
                if (bytes != 8)
                {
                    break;
                }
               // HandlePing((const SOCKADDR*)&SenderAddr, (const char*)RecvBuf, iResult);
                break;
            default:
                HandleRecv(conn,(const char*)datareceive, bytes);
                break;
            }

		}
        //////////////////////////////////////////////////////////////////
        for (auto It = idChannels.CreateConstIterator(); It; ++It)
        {
            It.Value()->Update(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
        }
	});
}
void Kcpclient::HandleConnect(const char* data, uint16 size)
{
    uint32 id = *(uint32*)(data + 4);
    uint32 requestConn = *(uint32*)(data + 8);
    if (requestChannels.Contains(requestConn))
    {
        requestChannels[requestConn]->HandleConnect(id);
        idChannels.Add(id, requestChannels[requestConn]);
        requestChannels.Remove(requestConn);
    }
}
void Kcpclient::HandleRecv(const uint32& id, const char* data, uint16 size)
{   
    CChannel** kc = idChannels.Find(id);
    if (kc)
    {
        (*kc)->HandleRecv(data, size);
    }
}
Kcpclient::~Kcpclient()
{
    if (receivethread)
    {
        receivethread->StopThread();
    }
    for (auto It = idChannels.CreateConstIterator(); It; ++It)
    {
        It.Value()->isConnected = false;
        delete It.Value();
    }
    idChannels.Empty();
}
CChannel* Kcpclient::CreateChannel(const FString& ipaddress, int32 port)
{
    bool bIsValid;
    TSharedPtr<class FInternetAddr>	RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    RemoteAddr->SetIp(*ipaddress, bIsValid);
    RemoteAddr->SetPort(port);
    if (!bIsValid)
    {
        //GEngine->AddOnScreenDebugMessage(-1, 51.0f, FColor::Yellow, TEXT("UDP Sender>> IP address was not valid!"));
        return nullptr;
    }
    uint32 id = 0;
    bool bc = false;
    do {
        id = FMath::RandRange(500, 0xffffffff);
        bc = requestChannels.Contains(id);
    } while (bc);  
    CChannel* channel = new CChannel(id, Socket,RemoteAddr);
    requestChannels.Add(channel->requestConn,channel);
    return channel;
}
//////////////////////////////////////////////////////////////////////////
CChannel::CChannel(uint32 requestConn,FSocket* Socket, const TSharedPtr<class FInternetAddr>& RemoteEp) {
    this->requestConn = requestConn;
    this->Socket = Socket;
    this->RemoteEp = RemoteEp;
    Async(EAsyncExecution::ThreadPool, [=]() {
        while (true)
        {
            FPlatformProcess::Sleep(0.2f);
            if (isConnected)
            {
                break;
            }
            uint32 syn = Kcpclient::KcpProtocalType::SYN;
            FMemory::Memcpy(cacheBytes, (uint8*)&syn, 4);
            FMemory::Memcpy(cacheBytes + 4, (uint8*)&requestConn, 4);
            int32 BytesSent = 0;
            Socket->SendTo(cacheBytes, 8, BytesSent, *RemoteEp);
        }
        }, nullptr);
}
CChannel::~CChannel() {
    if (pingthread)
    {
        pingthread->StopThread();
    }
}
void CChannel::HandleConnect(uint32& id)
{
    if (isConnected)
    {
        return;
    }
    this->Id = id;
    kcp1 = ikcp_create(Id, (void*)this);
    ikcp_nodelay(kcp1, 1, 10, 2, 1);
    ikcp_wndsize(kcp1, 512, 512);
    ikcp_setmtu(kcp1, 1400);
    kcp1->output = [](const char* buf, int len, struct IKCPCB* kcp, void* user)->int {
        int32 BytesSent = 0;
        ((CChannel*)user)->Socket->SendTo((const uint8*)buf, len, BytesSent, *((CChannel*)user)->RemoteEp);
        return 0;
    };
    isConnected = true;
    pingthread = new RunnableThreadx([=]() {
        FPlatformProcess::Sleep(2);
        uint32 ping = Kcpclient::KcpProtocalType::PING;
        FMemory::Memcpy(cacheBytes, (uint8*)&ping, 4);
        FMemory::Memcpy(cacheBytes + 4, (uint8*)&Id, 4);
        int32 BytesSent = 0;
        Socket->SendTo(cacheBytes, 8, BytesSent, *RemoteEp);
    });
}
void CChannel::HandleRecv(const char* data, uint16 size)
{
    ikcp_input(kcp1, (const char*)data, size);
}
void CChannel::Update(const IUINT32& currenttime)
{
    if (currenttime >= nexttimecallupdate)
    {
        ikcp_update(kcp1, currenttime);
        nexttimecallupdate = ikcp_check(kcp1, currenttime);
    }
    int hr = ikcp_recv(kcp1, (char*)kcpreceive, sizeof(kcpreceive));
    if (hr > 0)
    {
        if (onUserLevelReceivedCompleted)
        {
            onUserLevelReceivedCompleted((const uint8*)kcpreceive, hr);
        }
    }
}
void CChannel::Send(const char* data, const UINT16& size)
{
    if (isConnected)
    {
        ikcp_send(kcp1, data, size);
    }
}