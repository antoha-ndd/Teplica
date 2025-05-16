#pragma once
#include "Objects.h"

#if defined(ESP8266)
#include "tudp.h"

#endif

class TSerialCommander;

typedef void (*TSerialCommanderCallback)(TSerialCommander *Commander, String Param);
typedef void (*TSerialCommanderCallbackUnknown)(TSerialCommander *Commander, String Command, String Param);
typedef void (*TSerialCommanderCallbackError)(TSerialCommander *Commander, String Buffer);

typedef struct
{
    String Command;
    TSerialCommanderCallback Callback{};
} TSerialCommanderStruct;

typedef struct
{
    String Command;
    String Param;
} TSerialCommand;

TSerialCommand ParseCommandString(String Buffer)
{
    TSerialCommand Result;

    int DP_Pos = Buffer.indexOf(":");
    int EndPos = Buffer.length();

    if (DP_Pos > 0)
    {
        Result.Command = Buffer.substring(0, DP_Pos);
        Result.Param = Buffer.substring(DP_Pos + 1, EndPos);
    }
    else
        Result.Command = Buffer;

    Result.Command.toUpperCase();

    return Result;
}

TSerialCommand CreateCommandStruct(String Command, String Param = "")
{
    TSerialCommand Result;

    Result.Command = Command;
    Result.Param = Param;

    Result.Command.toUpperCase();

    return Result;
}

String CmdStr(String Command, String Param)
{
    Command.toUpperCase();
    return Command + ":" + Param;
}

typedef TStack<TSerialCommanderStruct *> TSerialCommands;

class TSerialCommander : public TControl
{
private:
    TSerialCommands *Commands;

#if defined(ESP8266)
    TUDP *UDP = NULL;
    unsigned int UDP_Port = 0;
#endif

    void
    ProcessBuffer(String Buffer)
    {
        Buffer.trim();
        TSerialCommand CommandStruct = ParseCommandString(Buffer);

#if defined(ESP8266)
        if (CommandStruct.Command != "ECHO")
        {
            if (CommandStruct.Command == "" || CommandStruct.Param == "")
                return;

            if (CommandStruct.Command != "ALL" && !IsMainID(CommandStruct.Command))
                return;

            CommandStruct = ParseCommandString(CommandStruct.Param);
        }

#else
        if (CommandStruct.Command == "")
        {
            if (ErrorCommand == NULL)
            {
                if (!NoSendErrorAnswer)
                    Println("There is no command");
            }
            else
                ErrorCommand(this, Buffer);
            return;
        }

#endif
        ProcessCommand(CommandStruct.Command, CommandStruct.Param);
    }

    TSerialCommanderCallback FindCallback(String Command)
    {
        TSerialCommanderStruct *Struct{NULL};
        Command.toUpperCase();

        Struct = Commands->GetNext(NULL);
        while (Struct != NULL)
        {
            String CmdName = Struct->Command;
            CmdName.toUpperCase();
            if (CmdName == Command)
                return Struct->Callback;

            Struct = Commands->GetNext(Struct);
        }

        return NULL;
    }

    void ProcessCommand(String Command, String Param)
    {

        TSerialCommanderCallback Cb;
        Cb = FindCallback(Command);

        if (Cb != NULL)
            Cb(this, Param);
        else
        {
            if (UnknownCommand == NULL)
            {
                if (!NoSendErrorAnswer)
                    Println("Unknown command");
            }
            else
                UnknownCommand(this, Command, Param);
        }
    }

public:
#if defined(ESP8266)
    uint16 DeviceType;
    char DeviceName[20];
#endif
    HardwareSerial *SerialPort;
    boolean NoSendErrorAnswer{true};

    void SendCommand(String Command, String Param = "")
    {
        String JSON = "" + Command + ":" + Param;
        Println(JSON);
    }

    TSerialCommander(HardwareSerial *_SerialPort, unsigned long baud, int TelnetPort);
    ~TSerialCommander();

    virtual void Idle();
    void Print(String Buffer)
    {
#if defined(ESP8266)
        Buffer = String(ESP.getChipId()) + ":" + Buffer;
#endif
        SerialPort->print(Buffer);
#if defined(ESP8266)
        if (UDP_Port > 0)
            UDP->Send(WiFi.broadcastIP().toString(), UDP_Port, Buffer);
#endif
    }

#if defined(ESP8266)
    bool IsMainID(String ID)
    {
        return String(ESP.getChipId()) == ID;
    }

    void SetDeviceName(String _DeviceName)
    {
        strncpy(DeviceName, _DeviceName.c_str(), 20);
    }

#endif

    void Println(String Buffer)
    {
        Print(Buffer + "\n");
    }

    TSerialCommanderCallbackUnknown UnknownCommand{NULL};
    TSerialCommanderCallbackError ErrorCommand{NULL};

    void AddCommand(String Command, TSerialCommanderCallback Callback)
    {

        TSerialCommanderStruct *Cmd = new TSerialCommanderStruct();
        Cmd->Command = Command;
        Cmd->Callback = Callback;
        Commands->Add(Cmd);
    };
};

#if defined(ESP8266)

void SC_ECHO(TSerialCommander *Commander, String Param)
{
    Commander->Println(CmdStr(WiFi.localIP().toString(), String(Commander->DeviceType)));
}

void SC_WHOIS(TSerialCommander *Commander, String Param)
{
    Commander->Println(CmdStr(String(Commander->DeviceName), String(Commander->DeviceType)));
}

void SC_IP(TSerialCommander *Commander, String Param)
{
    Commander->Println(CmdStr("IP", WiFi.localIP().toString()));
}

void SC_IPMASK(TSerialCommander *Commander, String Param)
{
    Commander->Println(CmdStr("IPMASK", WiFi.subnetMask().toString()));
}

void SC_IPDNS(TSerialCommander *Commander, String Param)
{
    Commander->Println(CmdStr("IPDNS", WiFi.dnsIP().toString()));
}

void SC_IPGATEWAY(TSerialCommander *Commander, String Param)
{
    Commander->Println(CmdStr("IPGATEWAY", WiFi.gatewayIP().toString()));
}

void SC_SSID(TSerialCommander *Commander, String Param)
{
    char ssid[32];
    struct station_config stationConf;

    wifi_station_get_config(&stationConf);

    os_memcpy(&ssid, stationConf.ssid, 32);

    Commander->Println(CmdStr("SSID", String(ssid)));
}

void SC_KEY(TSerialCommander *Commander, String Param)
{
    char password[64];
    struct station_config stationConf;

    wifi_station_get_config(&stationConf);

    os_memcpy(&password, stationConf.password, 64);

    Commander->Println(CmdStr("KEY", String(password)));
}

void SC_SETSSID(TSerialCommander *Commander, String Param)
{
    char ssid[32];
    // char password[64];

    strcpy(ssid, Param.c_str());
    struct station_config stationConf;
    wifi_station_get_config(&stationConf);

    stationConf.bssid_set = 0; // need	not	check	MAC	address
    os_memcpy(&stationConf.ssid, ssid, 32);
    // os_memcpy(&stationConf.password, password, 64);

    wifi_station_set_config(&stationConf);
    Commander->Println(CmdStr("SETSSID", "OK"));
}
void SC_SETKEY(TSerialCommander *Commander, String Param)
{
    // char ssid[32];
    char password[64];

    strcpy(password, Param.c_str());
    struct station_config stationConf;
    wifi_station_get_config(&stationConf);

    stationConf.bssid_set = 0; // need	not	check	MAC	address
    // os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);

    wifi_station_set_config(&stationConf);
    Commander->Println(CmdStr("SETKEY", "OK"));
}

void SC_PAIRING(TSerialCommander *Commander, String Param)
{
    Pairing();
}

void SC_REBOOT(TSerialCommander *Commander, String Param)
{
    Commander->Println("REBOOT:OK");
    delay(500);
    ESP.restart();
}

#endif

TSerialCommander::TSerialCommander(HardwareSerial *_SerialPort, unsigned long baud, int _UDP_Port = 0)
{
    SerialPort = _SerialPort;
    SerialPort->begin(baud);
    Commands = new TSerialCommands();

#if defined(ESP8266)

    SetDeviceName(String(ESP.getChipId()));
    AddCommand("echo", SC_ECHO);
    AddCommand("whois", SC_WHOIS);
    AddCommand("getip", SC_IP);
    AddCommand("getipmask", SC_IPMASK);
    AddCommand("getipdns", SC_IPDNS);
    AddCommand("getipgateway", SC_IPGATEWAY);
    AddCommand("getssid", SC_SSID);
    AddCommand("getkey", SC_KEY);
    AddCommand("setssid", SC_SETSSID);
    AddCommand("setkey", SC_SETKEY);
    AddCommand("pairing", SC_PAIRING);
    AddCommand("reboot", SC_REBOOT);

    UDP_Port = _UDP_Port;
    if (UDP_Port > 0)
    {
        UDP = new TUDP();
        UDP->Listen(UDP_Port);
    }

#endif
}

TSerialCommander::~TSerialCommander()
{
#if defined(ESP8266)
    if (UDP != NULL)
        delete UDP;
#endif
}

void TSerialCommander::Idle()
{
    static String LocalBuffer = "";
    if (SerialPort->available() > 0)
    {
        char Ch = SerialPort->read();
        if (Ch == '\n')
        {
            ProcessBuffer(LocalBuffer);
            LocalBuffer = "";
        }
        else
        {
            LocalBuffer += Ch;
        }
    }

#if defined(ESP8266)

    if (UDP_Port > 0)
    {
        if (UDP->PacketAvailable())
        {
            TUDPPacket Packet;
            Packet = UDP->GetPacket();
            Packet.Body.trim();
            ProcessBuffer(Packet.Body);
        }
    }
#endif
}