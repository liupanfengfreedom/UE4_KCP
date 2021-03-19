#include "UdpServer.h"
#include "RunnableThreadx.h"

#pragma comment(lib, "Ws2_32.lib")

UdpServer::UdpServer(UINT16 port, int& iResultp ,TFunction<void(char* data, UINT16& size)>receivework)
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
        if (receivework == nullptr)
        {
        }
        else
        {
            receivework(RecvBuf, iResult);
        }
        FPlatformProcess::Sleep(0.005);
        //receive();
     });
    iResultp = 0;
}
UdpServer::~UdpServer()
{
    if (receivethread)
    {
        receivethread->StopThread();
        delete receivethread;
    }
    close();
}

bool UdpServer::send(char*data, const UINT16& size)
{
    iResult = sendto(RecvSocket,
        data, size, 0, (SOCKADDR*)&SenderAddr, SenderAddrSize);
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