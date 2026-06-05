#pragma once

#if defined(ESP8266) || defined(ESP32)

#include "../Objects.h"
#include "../espwifi.h"
#include "Pairing.h"
#include <Preferences.h>

const char WIFI_FALLBACK_AP_SSID[] = "AutoConnectAP";
const char WIFI_AP_PASSWORD_KEY[] = "ap_pass";
const char WIFI_OTA_PASSWORD_KEY[] = "ota_pass";
const TTimeStamp WIFI_RECONNECT_INTERVAL = 30000;
const TTimeStamp APP_HEARTBEAT_INTERVAL = 10000;

#ifndef OTA_DEFAULT_PASSWORD
#define OTA_DEFAULT_PASSWORD ""
#endif

static String GetWiFiChipId()
{
#if defined(ESP8266)
	return String(ESP.getChipId(), HEX);
#elif defined(ESP32)
	char buf[17];
	snprintf(buf, sizeof(buf), "%llX", static_cast<unsigned long long>(ESP.getEfuseMac()));
	return String(buf);
#else
	return String("device");
#endif
}

static String GetPreferenceString(const char *key, const char *fallback = "")
{
	Preferences pref;
	pref.begin("config", true);
	String value = pref.getString(key, fallback);
	pref.end();
	return value;
}

static String GetFallbackAPPassword()
{
	String password = GetPreferenceString(WIFI_AP_PASSWORD_KEY);
	if (password.length() >= 8)
		return password;

	String chipId = GetWiFiChipId();
	chipId.toUpperCase();
	return String("teplica-") + chipId;
}

static String GetOTAPassword()
{
	String password = GetPreferenceString(WIFI_OTA_PASSWORD_KEY);
	if (password.length() > 0)
		return password;

	return String(OTA_DEFAULT_PASSWORD);
}

class TWiFiControl : public TControl
{
private:
	bool WiFiApStarted{false};
	bool WiFiStaConnected{false};
	bool OTAStarted{false};
	bool OTADisabledLogged{false};
	bool WifiUseSta{false};
	String WifiStaSsid;
	String WifiStaPass;
	String WiFiTargetSSID;
	TTimeStamp WiFiLastReconnectAttempt{0};
	TTimeStamp LastHeartbeatTime{0};

	void UpdateWiFiTargetSSID()
	{
		String CurrentSSID = WiFi.SSID();
		if (CurrentSSID.length() > 0)
			WiFiTargetSSID = CurrentSSID;
	}

	String GetWiFiTargetSSID()
	{
		if (WiFiTargetSSID.length() > 0)
			return WiFiTargetSSID;

		return String("(saved network)");
	}

	void LogWiFiConnectAttempt()
	{
		// Serial.print("[WiFi] Connecting to ");
		// Serial.println(GetWiFiTargetSSID());
	}

	void LogWiFiConnected()
	{
		// Serial.print("[WiFi] Connected to ");
		// Serial.print(WiFi.SSID());
		// Serial.print(", IP: ");
		// Serial.print(WiFi.localIP());
		// Serial.print(", RSSI: ");
		// Serial.print(WiFi.RSSI());
		// Serial.println(" dBm");
	}

	void LogHeartbeat()
	{
		// Serial.print("[Heartbeat] uptime=");
		// Serial.print(millis() / 1000);
		// Serial.print("s, wifi=");

		if (WiFi.status() == WL_CONNECTED)
		{
			// Serial.print("connected, ip=");
			// Serial.print(WiFi.localIP());
			// Serial.print(", rssi=");
			// Serial.print(WiFi.RSSI());
			// Serial.print(" dBm");
		}
		else
		{
			// Serial.print("disconnected");
		}

		// Serial.print(", ap=");
		// Serial.println(WiFiApStarted ? "on" : "off");
	}

	void HandleHeartbeat(TTimeStamp CurrentTime)
	{
		if ((CurrentTime - LastHeartbeatTime) < APP_HEARTBEAT_INTERVAL)
			return;

		LastHeartbeatTime = CurrentTime;
		LogHeartbeat();
	}

	void EnsureSoftAP()
	{
		if (WiFiApStarted)
			return;

		WiFi.setSleep(false);
		WiFi.setTxPower(WIFI_POWER_8_5dBm);

		IPAddress ap_ip(192, 168, 4, 1);
		IPAddress ap_gw(192, 168, 4, 1);
		IPAddress ap_mask(255, 255, 255, 0);

		String apPassword = GetFallbackAPPassword();
		WiFiApStarted = WiFi.softAP(WIFI_FALLBACK_AP_SSID, apPassword.c_str());
		if (WiFiApStarted)
			WiFi.softAPConfig(ap_ip, ap_gw, ap_mask);

		delay(100);

		if (WiFiApStarted)
		{
			// Serial.print("[WiFi] AP ready IP ");
			// Serial.print(WiFi.softAPIP());
			// Serial.print(", ssid=");
			// Serial.print(WIFI_FALLBACK_AP_SSID);
			// Serial.print(", password=");
			// Serial.println(apPassword);
		}
		else
		{
			// Serial.println("[WiFi] Failed to start fallback AP");
		}
	}

	void LoadWiFiSettings()
	{
		Preferences pref;
		pref.begin("config", true);
		String ssid = pref.getString("wifi_ssid", "");
		String pass = pref.getString("wifi_pass", "");
		pref.end();

		WifiUseSta = (ssid.length() > 0);
		WifiStaSsid = ssid;
		WifiStaPass = pass;
	}

	void BeginWiFi()
	{
		LoadWiFiSettings();
		WiFiLastReconnectAttempt = millis();

		if (!WifiUseSta)
		{
			WiFi.mode(WIFI_AP);
			EnsureSoftAP();
			return;
		}

		WiFi.mode(WIFI_AP_STA);
		WiFi.setSleep(false);
		WiFi.setTxPower(WIFI_POWER_8_5dBm);
		WiFiTargetSSID = WifiStaSsid;
		LogWiFiConnectAttempt();
		WiFi.begin(WifiStaSsid.c_str(), WifiStaPass.c_str());
		EnsureSoftAP();
	}

	void HandleWiFi()
	{
		if (!WifiUseSta)
		{
			EnsureSoftAP();
			return;
		}

		EnsureSoftAP();

		bool connected = (WiFi.status() == WL_CONNECTED);
		if (connected)
		{
			if (!WiFiStaConnected)
			{
				UpdateWiFiTargetSSID();
				LogWiFiConnected();
			}
			WiFiStaConnected = true;
			return;
		}

		if (WiFiStaConnected)
			; // Serial.println("[WiFi] STA connection lost");

		WiFiStaConnected = false;

		TTimeStamp now = millis();
		if ((now - WiFiLastReconnectAttempt) >= WIFI_RECONNECT_INTERVAL)
		{
			WiFiLastReconnectAttempt = now;
			// Serial.println("[WiFi] STA reconnect");
			WiFi.disconnect(false);
			WiFiTargetSSID = WifiStaSsid;
			LogWiFiConnectAttempt();
			WiFi.begin(WifiStaSsid.c_str(), WifiStaPass.c_str());
		}
	}

	bool IsOTANetworkReady()
	{
		return WiFi.status() == WL_CONNECTED || WiFiApStarted;
	}

	void LogOTAReady()
	{
		// Serial.print("[OTA] Ready, port=8266");
		if (WiFi.status() == WL_CONNECTED)
		{
			// Serial.print(", sta=");
			// Serial.print(WiFi.localIP());
		}
		if (WiFiApStarted)
		{
			// Serial.print(", ap=");
			// Serial.print(WiFi.softAPIP());
		}
		// Serial.println();
	}

	void HandleOTA()
	{
		if (!IsOTANetworkReady())
			return;

		if (!OTAStarted)
		{
			String otaPassword = GetOTAPassword();
			if (otaPassword.length() == 0)
			{
				if (!OTADisabledLogged)
				{
					// Serial.println("[OTA] Disabled: set ota_pass in Preferences or OTA_DEFAULT_PASSWORD to enable OTA");
					OTADisabledLogged = true;
				}
				return;
			}

			ArduinoOTA.setPort(8266);
			ArduinoOTA.setHostname("ESP_Board");
			ArduinoOTA.setPassword(otaPassword.c_str());
			ArduinoOTA.begin();
			OTAStarted = true;
			OTADisabledLogged = false;
			LogOTAReady();
		}

		ArduinoOTA.handle();
	}

public:
	TWiFiControl() : TControl(NULL)
	{
		LastHeartbeatTime = millis() - APP_HEARTBEAT_INTERVAL;
		BeginWiFi();
	}

	IPAddress SoftAPIP()
	{
		return WiFi.softAPIP();
	}

	void ApplySettingsFromNvs()
	{
		OTAStarted = false;
		OTADisabledLogged = false;
		WiFiStaConnected = false;
		WiFiLastReconnectAttempt = millis();
		LoadWiFiSettings();
		WiFi.softAPdisconnect(true);
		WiFiApStarted = false;

		if (!WifiUseSta)
		{
			WiFi.disconnect(false);
			WiFi.mode(WIFI_AP);
			EnsureSoftAP();
			return;
		}

		WiFi.mode(WIFI_AP_STA);
		WiFi.setSleep(false);
		WiFi.setTxPower(WIFI_POWER_8_5dBm);
		WiFiTargetSSID = WifiStaSsid;
		LogWiFiConnectAttempt();
		WiFi.disconnect(false);
		WiFi.begin(WifiStaSsid.c_str(), WifiStaPass.c_str());
		EnsureSoftAP();
	}

	virtual void Idle()
	{
		TTimeStamp CurrentTime = millis();
		HandleHeartbeat(CurrentTime);
		HandleWiFi();
		HandleOTA();
	}
};

inline void Pairing()
{
	Preferences pref;
	pref.begin("config", false);
	pref.remove("wifi_ssid");
	pref.remove("wifi_pass");
	pref.end();

	// Serial.println("[WiFi] Pairing mode requested: saved STA credentials cleared, restarting into AP setup");
	delay(100);
	ESP.restart();
}

#endif
