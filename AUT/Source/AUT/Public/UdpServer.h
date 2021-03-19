#pragma once

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>

class UdpServer
{
    UINT16 iResult = 0;

    WSADATA wsaData;

    SOCKET RecvSocket;
    struct sockaddr_in RecvAddr;

    unsigned short Port = 27015;

    char RecvBuf[0xffff];
    int BufLen = 0xffff;

    struct sockaddr_in SenderAddr;
    int SenderAddrSize = sizeof(SenderAddr);
    class RunnableThreadx* receivethread = nullptr;

public:
	UdpServer(UINT16 port, int& iResultp, TFunction<void(char* data, UINT16& size)>receivework);
	~UdpServer();
    bool send(char*data,const UINT16& size);
    bool close();

};

