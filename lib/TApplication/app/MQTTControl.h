#pragma once

#if defined(ESP8266) || defined(ESP32)

#include "../Objects.h"
#include <WiFiClient.h>
#include <PubSubClient.h>

void AppMQTTProcessMessage(String topic, String payload);
void MqttPublishFullState();

class TMQTTControl : public TControl
{
private:
	WiFiClient MQTTClient;
	PubSubClient mqtt;
	String MQTT_Server;
	int MQTT_Port{1883};
	String MQTT_Topic{"teplica"};
	bool MQTT_Intialized{false};
	TTimeStamp MQTT_Last_Tick{0};

	static TMQTTControl *&Instance()
	{
		static TMQTTControl *instance = nullptr;
		return instance;
	}

	static void MQTT_callback(char *topic, byte *payload, size_t length)
	{
		TMQTTControl *instance = Instance();
		if (instance == NULL)
			return;

		String Topic = String(topic);
		String Payload;
		Payload.reserve(length);

		for (size_t i = 0; i < length; i++)
			Payload += (char)payload[i];

		instance->ProcessMessage(Topic, Payload);
	}

	void ProcessMessage(String topic, String payload)
	{
		AppMQTTProcessMessage(topic, payload);
	}

	void reconnect()
	{
		if (!MQTT_Intialized || WiFi.status() != WL_CONNECTED || mqtt.connected())
			return;

		String clientId = "ESPClient-";
#if defined(ESP8266)
		clientId += String(ESP.getChipId(), HEX);
#elif defined(ESP32)
		clientId += String((uint32_t)ESP.getEfuseMac(), HEX);
#else
		clientId += String(random(0xffff), HEX);
#endif

		if (mqtt.connect(clientId.c_str()))
		{
			// Serial.println("[MQTT] connected");
			String subscription = MQTT_Topic + "/#";
			if (!mqtt.subscribe(subscription.c_str()))
				; // Serial.println("[MQTT] subscribe failed");
			MqttPublishFullState();
		}
		else
		{
			// Serial.print("[MQTT] connect failed, state=");
			// Serial.println(mqtt.state());
		}
	}

public:
	int MQTT_Timeout{5000};

	TMQTTControl() : TControl(NULL) {}

	virtual ~TMQTTControl()
	{
		MqttShutdown();
	}

	void InitMQTT(String Server, int Port, String Topic)
	{
		mqtt.disconnect();
		MQTT_Server = Server;
		MQTT_Port = Port;
		if (Topic.length() == 0)
			Topic = "teplica";
		MQTT_Topic = Topic;

		mqtt.setClient(MQTTClient);
		if (!mqtt.setBufferSize(512, 512))
			; // Serial.println("[MQTT] buffer allocation failed");
		mqtt.setServer(MQTT_Server.c_str(), MQTT_Port);
		mqtt.setCallback(MQTT_callback);
		MQTT_Intialized = true;
		Instance() = this;
	}

	void MqttShutdown()
	{
		mqtt.disconnect();
		MQTT_Intialized = false;
		if (Instance() == this)
			Instance() = NULL;
	}

	bool IsMQTTConnected()
	{
		return MQTT_Intialized && mqtt.connected();
	}

	void PublishUnderTopic(const String &relativePath, const String &payload, bool retained = false)
	{
		if (!MQTT_Intialized || !mqtt.connected())
			return;

		String t = MQTT_Topic + "/" + relativePath;
		if (!mqtt.publish(t.c_str(), payload.c_str(), retained))
			; // Serial.println("[MQTT] publish failed");
	}

	virtual void Idle()
	{
		if (!MQTT_Intialized)
			return;

		TTimeStamp CurrentTime = millis();
		if ((CurrentTime - MQTT_Last_Tick) > MQTT_Timeout)
		{
			MQTT_Last_Tick = CurrentTime;
			reconnect();
		}

		if (mqtt.connected())
			mqtt.loop();
	}
};

#endif
