#include "UdpServer.h"
#include "RunnableThreadx.h"
#include "MyBlueprintFunctionLibrary.h"

#pragma comment(lib, "Ws2_32.lib")

UdpServer::UdpServer(UINT16 port, int& iResultp)
{
    //-----------------------------------------------
// Initialize Winsock
    iResultp = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResultp != NO_ERROR) {
        wprintf(L"WSAStartup failed with error %d\n", iResultp);
        iResultp = -1;
        return;
    }

    //-----------------------------------------------
    // Create a receiver socket to receive datagrams
    RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (RecvSocket == INVALID_SOCKET) {
        wprintf(L"socket failed with error %d\n", WSAGetLastError());
        iResultp = -2;
        return;
    }
    //-----------------------------------------------
// Bind the socket to any address and the specified port.
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(port);
    RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    iResultp = bind(RecvSocket, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
    if (iResultp != 0) {
        wprintf(L"bind failed with error %d\n", WSAGetLastError());
        iResultp = -3;
        return;
    }
    receivethread = new RunnableThreadx([=]() {
        iResult = recvfrom(RecvSocket,
            RecvBuf, BufLen, 0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);
        if (iResult == SOCKET_ERROR) {
           
            wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
        }
        uint32 conn = *(uint32*)RecvBuf;
        switch (conn)
        { 
        case KcpProtocalType::SYN:
            if (iResult != 8)
            {
                break;
            }
            HandleAccept((const SOCKADDR*)&SenderAddr, (const char*)RecvBuf, iResult);
            break;
        case KcpProtocalType::ACK:
            if (iResult != 12)
            {
                break;
            }
            HandleConnect((const SOCKADDR*)&SenderAddr, (const char*)RecvBuf, iResult);
            break;
        case KcpProtocalType::PING:
            if (iResult != 8)
            {
                break;
            }
            HandlePing((const SOCKADDR*)&SenderAddr, (const char*)RecvBuf, iResult);
            break;
        default:
            HandleRecv(conn,(const SOCKADDR*)&SenderAddr, (const char*)RecvBuf, iResult);
            break;
        }
        FPlatformProcess::Sleep(0.001);
     });
    iResultp = 0;
    updatethread = new RunnableThreadx([=]() {      
        FPlatformProcess::Sleep(0.010);
        {
            FScopeLock Lock(&Mutex);
            for (auto It = idChannels.CreateConstIterator(); It; ++It)
            {
                It.Value()->Update(FDateTime::UtcNow().ToUnixTimestamp() * 1000);
            }
        }
     });
    pingcheckthread = new RunnableThreadx([=]() {
        FPlatformProcess::Sleep(60);
        {
            FScopeLock Lock(&Mutex);
            for (auto It = idChannels.CreateConstIterator(); It; ++It)
            {
                It.Value()->CheckPing(FDateTime::UtcNow().ToUnixTimestamp() * 1000);
            }
        }
     });
}
UdpServer::~UdpServer()
{
    if (receivethread)
    {
        receivethread->StopThread();
        delete receivethread;
    }
    if (updatethread)
    {
        updatethread->StopThread();
        delete updatethread;
    }
    if (pingcheckthread)
    {
        pingcheckthread->StopThread();
        delete pingcheckthread;
    }
    close();
}

bool UdpServer::send(SOCKADDR* remoteaddr,char*data, const UINT16& size)
{
    iResult = sendto(RecvSocket,
        data, size, 0, remoteaddr, SenderAddrSize);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return false;
    }
    return true;
}
bool UdpServer::close()
{
    iResult = closesocket(RecvSocket);
    WSACleanup();
    if (iResult == SOCKET_ERROR) {
        wprintf(L"closesocket failed with error %d\n", WSAGetLastError());
        return false;
    }
    if (receivethread)
    {
        receivethread->StopThread();
        delete receivethread;
        receivethread = nullptr;
    }
    return true;
}
void UdpServer::HandleAccept(const SOCKADDR* remoteaddr, const char* data, UINT16& size)
{
    FString remoteepstr = FMD5::HashBytes((const uint8*)remoteaddr->sa_data, 14);
    if (EPChannels.Contains(remoteepstr))
    {
    }
    else
    {
        uint32 requestConn = *(uint32*)(data+4);
        uint32 newid;
        do {
            newid = IdGenerater++;
        }
        while (idChannels.Contains(newid));
        KChannel* channel = new KChannel(newid, requestConn, *remoteaddr, this);
        idChannels.Add(channel->Id, channel);
        EPChannels.Add(remoteepstr, channel);
        if (onacceptchannel)
        {
            onacceptchannel(channel);
        }
    }
    EPChannels[remoteepstr]->HandleAccept();
}
void UdpServer::HandleConnect(const SOCKADDR* remoteaddr, const char* data, UINT16& size)
{

}
void UdpServer::HandlePing(const SOCKADDR* remoteaddr, const char* data, UINT16& size)
{
    KChannel** kc = idChannels.Find(*(uint32*)(data + 4));
    if (kc)
    {
        (*kc)->HandlePing();
    }
}
void UdpServer::HandleRecv(const uint32& conn, const SOCKADDR* remoteaddr, const char* data, UINT16& size)
{
    KChannel** kc = idChannels.Find(conn);
    if (kc)
    {
        (*kc)->HandleRecv(remoteaddr,data, size);
    }
}
///////////////////////////////////////////////////////////////////////////////////
KChannel::KChannel(uint32 newid, uint32 requestConn, SOCKADDR remotesocket, UdpServer* server)
{
    this->Id = newid;
    this->requestConn = requestConn;
    this->remotesocket = remotesocket;
    this->server = server;

    kcp1 = ikcp_create(newid, (void*)this);
    ikcp_nodelay(kcp1, 1, 10, 2, 1);
    ikcp_wndsize(kcp1, 512, 512);
    ikcp_setmtu(kcp1, 1400);
    kcp1->output = [](const char* buf, int len, struct IKCPCB* kcp, void* user)->int {
        ((KChannel*)user)->server->send(&(((KChannel*)user)->remotesocket), (char*)buf, len);
        return 0;
    };
    lastpingtime = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
    isConnected = true;
}
KChannel::~KChannel()
{}
void KChannel::HandleAccept()
{
    uint32 ack = KcpProtocalType::ACK;
    FMemory::Memcpy(cacheBytes, (uint8*)&ack, 4);
    FMemory::Memcpy(cacheBytes+4, (uint8*)&Id, 4);
    FMemory::Memcpy(cacheBytes+8, (uint8*)&requestConn, 4);
    server->send(&remotesocket, (char*)cacheBytes, 12);
}
void KChannel::HandleRecv(const SOCKADDR* premotesocket, const char* data, const UINT16& size)
{
    FString remoteepstr = FMD5::HashBytes((const uint8*)premotesocket->sa_data, 14);
    FString remoteepstr1 = FMD5::HashBytes((const uint8*)remotesocket.sa_data, 14);
    if (remoteepstr != remoteepstr1)
    {
        server->EPChannels.Remove(remoteepstr1);
        remotesocket = *premotesocket;
        UMyBlueprintFunctionLibrary::CLogtofile(remoteepstr + " : remoteepstr");
        UMyBlueprintFunctionLibrary::CLogtofile(remoteepstr1 + " : remoteepstr1");
    }
    ikcp_input(kcp1, (const char*)data, size);
}
void KChannel::HandlePing()
{
    lastpingtime = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
}
#define TIMEOUTPERIOD 1000*60//1000*60*60
void KChannel::CheckPing(const IUINT32& currenttime)
{
    if (currenttime - lastpingtime> TIMEOUTPERIOD)
    {
        disconnect();
    }
}
void KChannel::Update(const IUINT32& currenttime)
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
void KChannel::Send(const char* data, const UINT16& size)
{
    ikcp_send(kcp1, data, size);
}
void KChannel::disconnect()
{
    isConnected = false;
    UMyBlueprintFunctionLibrary::CLogtofile(FString::FromInt(server->EPChannels.Num()) + " : server->EPChannels.Num()");
    UMyBlueprintFunctionLibrary::CLogtofile(FString::FromInt(server->idChannels.Num()) + " : server->idChannels.Num()");
    FString remoteepstr1 = FMD5::HashBytes((const uint8*)remotesocket.sa_data, 14);
    {
        FScopeLock Lock(&server->Mutex);
        KChannel** kc = server->idChannels.Find(Id);
        if (kc)
        {
            if (*kc)
            {
                delete* kc;
                UMyBlueprintFunctionLibrary::CLogtofile("delete ");
            }
        }
        server->EPChannels.Remove(remoteepstr1);
        server->idChannels.Remove(Id);
    }
    if (ondisconnect)
    {
        ondisconnect();
    }
    UMyBlueprintFunctionLibrary::CLogtofile(FString::FromInt(server->EPChannels.Num()) + " : server->EPChannels.Num()");
    UMyBlueprintFunctionLibrary::CLogtofile(FString::FromInt(server->idChannels.Num()) + " : server->idChannels.Num()");
}
