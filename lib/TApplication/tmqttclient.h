#pragma once
#include "Objects.h"
#include "stack.h"

#if defined(ESP8266) || defined(ESP32)
#include <MQTT.h>
#include <WiFiClient.h>

inline WiFiClient &LegacyMQTTNet()
{
    static WiFiClient Net;
    return Net;
}

typedef void (*TTopicEventHandler)(String payload);

struct TTopicEventHandlerStruct
{
    TTopicEventHandler func;
    String Topic;
};

class TMQTTClient : public TControl, public MQTTClient
{

private:
    String Topic;
    TStack<TTopicEventHandlerStruct *> TopicEventsHandlers;
    TTimeStamp ConnectTime{0};

    // Обработчик сырых MQTT-сообщений
    static void StaticMessageHandler(MQTTClient *client, char topic[], char payload[], int length)
    {
        TMQTTClient *instance = static_cast<TMQTTClient *>(client);
        instance->ProcessMessage(topic, payload, length);
    }

    void ProcessMessage(char *topic, char *payload, int length)
    {
        String sTopic(topic);
        String sPayload(payload, length);
        MessageReceived(sTopic, sPayload);
    }

public:
    void RegisterTopicHandler(String _Topic, TTopicEventHandler func)
    {

        TTopicEventHandlerStruct *s = new TTopicEventHandlerStruct;
        s->Topic = Topic + "/" + _Topic;
        s->func = func;

        s->Topic.toUpperCase();

        TopicEventsHandlers.Add(s);
    }

    void UnregisterTopicHandler(String _Topic)
    {

        _Topic = Topic + "/" + _Topic;
        _Topic.toUpperCase();

        TTopicEventHandlerStruct *Current{NULL};
        Current = TopicEventsHandlers.GetNext(Current);

        while (Current != NULL)
        {
            if(Current->Topic == _Topic){
                TopicEventsHandlers.Delete(Current);
                delete Current;
                return;
            }
            Current = TopicEventsHandlers.GetNext(Current);
        }
    }

    void MessageReceived(String topic, String payload)
    {
        TTopicEventHandlerStruct *Current{NULL};
        Current = TopicEventsHandlers.GetNext(Current);

        topic.toUpperCase();

        while (Current != NULL)
        {
            if(Current->Topic == topic){
                Current->func(payload);
                return;
            }
            Current = TopicEventsHandlers.GetNext(Current);
        }
    }

    TMQTTClient(String Server, int Port, String _Topic) : TControl(NULL)
    {
        Topic = _Topic;
        begin(Server.c_str(), Port, LegacyMQTTNet());
        
        onMessageAdvanced(StaticMessageHandler);
    };

    virtual ~TMQTTClient()
    {
        TTopicEventHandlerStruct *Current{NULL};
        Current = TopicEventsHandlers.GetNext(Current);
        while (Current != NULL)
        {
            TTopicEventHandlerStruct *Next = TopicEventsHandlers.GetNext(Current);
            delete Current;
            Current = Next;
        }
    }

    void Send(String SubTopic, String Payload)
    {
        if (connected())
            publish((Topic + "/" + SubTopic).c_str(), Payload.c_str());
    }

    void UpdateSetting(String Server, int Port, String _Topic){

        disconnect();
        begin(Server.c_str(), Port, LegacyMQTTNet());
        Topic = _Topic;

    }

    void Connect()
    {
        if (WiFi.status() == WL_CONNECTED)
            if (connect("client", "", ""))
                subscribe((Topic + "/#").c_str());
    }

    virtual void Idle()
    {
        TTimeStamp CurTime = millis();

        if (!connected() && (CurTime - ConnectTime) > 5000)
        {
            ConnectTime = CurTime;
            Connect();
        }

        else
        {
            ConnectTime = 0;
            loop();
        }
    }
};

#endif
