// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ikcp.h"

/**
 * 
 */

class KCP_API Kcpclient
{
    class FSocket* Socket;
    class RunnableThreadx* receivethread;
    #define receivebuffersize  1024 * 4
    uint8 datareceive[receivebuffersize] = {0};
    TMap<uint32, class CChannel*>idChannels;
    TMap<uint32, class CChannel*>requestChannels;
public:
    enum KcpProtocalType {
        SYN = 1,
        ACK,
        FIN,
        PING
    };
	Kcpclient();
	~Kcpclient();
    class CChannel* CreateChannel(const FString& ipaddress, int32 port);
private:
    void HandleConnect(const char* data, uint16 size);
    void HandleRecv(const uint32& id, const char* data,uint16 size);
};
class KCP_API CChannel
{
    ikcpcb* kcp1;
    uint32 Id;
    class FSocket* Socket;
    TSharedPtr<class FInternetAddr>	RemoteEp;
    uint8 cacheBytes[12] = { 0 };
    class RunnableThreadx* pingthread;
    IUINT32 nexttimecallupdate = 0;
    uint8 kcpreceive[1024 * 4] = { 0 };

public:
    bool isConnected = false;
    uint32 requestConn;
    CChannel(uint32 requestConn,class FSocket* Socket,const TSharedPtr<class FInternetAddr>& RemoteEp);
    ~CChannel();
    void HandleConnect(uint32& id);
    void HandleRecv(const char* data, uint16 size);
    void Update(const IUINT32& currenttime);
    TFunction<void(const uint8* data, const uint16& size)> onUserLevelReceivedCompleted;
    void Send(const char* data, const uint16& size);
};