#pragma once
#include <WiFiUdp.h>
#include "Objects.h"

class TUDP;

typedef struct
{
    String remoteIP;
    uint16 remotePort;
    String destinationIP;
    uint16 localPort;
    String Body;
} TUDPPacket;

void EmptyOnReceive(TUDP *Socket, TUDPPacket Packet) {};

class TUDP : public TControl, private WiFiUDP
{
private:
    uint16 Port{0};
    char packetBuffer[1];

public:
    TUDP(TObject *_Parent) : TControl(_Parent)
    {
    }

    TUDP() : TControl()
    {
    }

    void Listen(uint16 _Port)
    {
        begin(_Port);
    }

    void Stop()
    {
        stop();
    }

    bool PacketAvailable()
    {
        return parsePacket() > 0;
    }

    TUDPPacket GetPacket()
    {
        TUDPPacket Packet;
        // if (PacketAvailable())
        //{

        Packet.remoteIP = remoteIP().toString();
        Packet.remotePort = remotePort();
        Packet.destinationIP = destinationIP().toString();
        Packet.localPort = localPort();

        int ReadyBites = 1;
        while (ReadyBites > 0)
        {
            ReadyBites = read(packetBuffer, 1);
            if (ReadyBites > 0)
                Packet.Body += packetBuffer[0];
        }
        //}

        return Packet;
    }

    void Idle()
    {
        if (PacketAvailable())
            OnReceive(this, GetPacket());
    }

    void Send(String Host, uint16_t Port, String Body)
    {
        beginPacket(Host.c_str(), Port);
        write(Body.c_str());
        endPacket();
    }
    void (*OnReceive)(TUDP *Socket, TUDPPacket Packet){EmptyOnReceive};
};
