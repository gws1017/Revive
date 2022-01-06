#pragma once
#include "iocp_server.h"
class PacketManager;
class InGameServer :
    public IOCPServer
{
public:
    InGameServer();
    virtual ~InGameServer();

    virtual void Disconnect(int c_id) override;
    virtual bool OnAccept( EXP_OVER* exp_over) override;
    virtual bool OnRecv(int c_id, EXP_OVER* exp_over,DWORD num_bytes) override;
    void ProcessPacket(int c_id, unsigned char* p);
    void Run();

private:
    std::unique_ptr< PacketManager>m_PacketManager;
};

