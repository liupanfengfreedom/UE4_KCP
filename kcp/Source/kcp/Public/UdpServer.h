#pragma once
#if PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN


#include <winsock2.h>
#include <Ws2tcpip.h>
#include "ikcp.h"

class UdpServer
{
    uint16 iResult = 0;

    WSADATA wsaData;

    SOCKET RecvSocket;
    struct sockaddr_in RecvAddr;

    unsigned short Port = 27015;

    char RecvBuf[0xfe5];
    int BufLen = 0xfe5;

    struct sockaddr_in SenderAddr;
    int SenderAddrSize = sizeof(SenderAddr);
    class RunnableThreadx* receivethread = nullptr;
    class RunnableThreadx* updatethread = nullptr;
    class RunnableThreadx* pingcheckthread = nullptr;

public:
    enum KcpProtocalType {
        SYN = 1,
        ACK,
        FIN,
        PING
    };
	UdpServer(uint16 port, int& iResultp);
	~UdpServer();
    bool send(SOCKADDR* remoteaddr, char*data,const uint16& size);
    bool close();
//////////////////////////////////////////////////
    uint32 IdGenerater=1000;
    TMap<uint32, class KChannel*>idChannels;
    TMap<uint32, class KChannel*>requestChannels;
    TFunction<void(KChannel* channel)> onacceptchannel;
    FCriticalSection Mutex;
private:
    void HandleAccept(const SOCKADDR* remoteaddr, const char*data, uint16& size);
    void HandleConnect(const SOCKADDR* remoteaddr, const char* data, uint16& size);
    void HandlePing(const SOCKADDR* remoteaddr, const char* data, uint16& size);
    void HandleRecv(const uint32& conn, const SOCKADDR* remoteaddr, const char* data, uint16& size);
};
///////////////////////////////////////////////////////////////////////////
class KChannel
{
    ikcpcb* kcp1;
    IUINT32 nexttimecallupdate = 0;
    uint8 cacheBytes[12] = {0};
    uint8 kcpreceive[1024 * 4] = { 0 };

    IUINT32 lastpingtime = 0;
    FCriticalSection Mutex;
public:
    bool isConnected = false;
    uint32 Id;
    uint32 requestConn;
    UdpServer* server;
    SOCKADDR remotesocket;
public:
    KChannel(uint32 newid,uint32 requestConn, SOCKADDR remotesocket, UdpServer*server);
    ~KChannel();
    void HandleAccept();
    void HandleRecv(const SOCKADDR* remotesocket, const char* data, const uint16& size);
    void HandlePing();
    void CheckPing(const IUINT32& currenttime);
    void Update(const IUINT32& currenttime);
    void Send(const char* data, const uint16& size);
    TFunction<void(const uint8* data, const uint16& size)> onUserLevelReceivedCompleted;
    TFunction<void()> ondisconnect;
    void disconnect();
};
#endif


