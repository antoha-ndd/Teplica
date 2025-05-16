#pragma once
#include "Objects.h"

#if defined(ESP8266)
#include <MQTT.h>
#include <WiFiClient.h>
#include <ElegantOTA.h>

WiFiClient Net;
class TMQTTClient;

void OnReceiveCb(MQTTClient *client, char topic[], char bytes[], int length);
void OnReceiveEmpty(TMQTTClient *Client, String Topic, String Payload);

class TMQTTClient : public TControl, public MQTTClient
{
private:
    void (*OnReceiveEvent)(MQTTClient *client, char topic[], char bytes[], int length);

public:
    bool Active{false};
    String Server;
    int Port{1883};
    String Topic;
    TMQTTClient(String _Server, int _Port, String _Topic) : TControl(NULL)
    {
        Topic = _Topic;
        Server = _Server;
        Port = _Port;
        begin(Server.c_str(), Port, Net);
        ref = this;
        OnReceiveEvent = OnReceiveCb;
        OnReceive = OnReceiveEmpty;
        onMessageAdvanced(OnReceiveEvent);
    };
    ~TMQTTClient();

    void (*OnReceive)(TMQTTClient *Client, String Topic, String Payload){nullptr};

    void Send(String SubTopic, String Payload)
    {
        if (connected())
            publish((Topic + "/" + SubTopic).c_str(), Payload.c_str());
    }

    void Connect()
    {
        if (WiFi.status() == WL_CONNECTED)
            if (connect("client", "", ""))
                subscribe((Topic + "/#").c_str());
    }

    virtual void Idle()
    {
        /*if (Active && !connected())
            Connect();
        else if (!Active && connected())
            disconnect();
        else*/
        loop();
    }
};

TMQTTClient::~TMQTTClient()
{
}

void OnReceiveCb(MQTTClient *client, char topic[], char bytes[], int length)
{
    TMQTTClient *Cl = (TMQTTClient *)client->ref;
    Cl->OnReceive(Cl, String(topic), String(bytes));
}

void OnReceiveEmpty(TMQTTClient *Client, String Topic, String Payload) {

};

#endif
