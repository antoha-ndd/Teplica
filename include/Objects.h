#pragma once
#include "stack.h"

#if defined(ESP8266)
#include "espwifi.h"
#include <WiFiClient.h>
#endif

class TApplication;

typedef unsigned int ObjectId;
typedef void (*TApplicationPrintCallback)(TApplication *App, String Buffer);

void DefaultApplicationPrintCallBack(TApplication *App, String Buffer)
{
	Serial.print(Buffer);
}

void DefaultApplicationPrintLnCallBack(TApplication *App, String Buffer)
{
	Serial.println(Buffer);
}

#if defined(ESP8266)
void Pairing()
{
	WiFiManager wifiManager;
	wifiManager.resetSettings();
	wifiManager.autoConnect("AutoConnectAP");
}
#endif

void EmptryEvent() {};

unsigned long int EmptyTimer() { return millis(); };
unsigned long int (*GetTimerValue)(){EmptyTimer};

ObjectId LastObjectId{0};

ObjectId GetGlobalIDValue()
{

	LastObjectId++;
	return LastObjectId;
}

class TObject;
class TApplication;
class TControl;
class TActiveControl;

class TObject
{
private:
	TObject *Parent;
	ObjectId ID;

public:
	TStack<TObject *> *Children;

	TObject(TObject *_Parent);
	~TObject();
	void AddChild(TObject *Child);
	void RemoveChild(TObject *Child);

	ObjectId GetID()
	{
		return ID;
	};

	TObject GetParent()
	{
		return Parent;
	}
};

TObject::TObject(TObject *_Parent)
{
	this->ID = GetGlobalIDValue();
	this->Parent = _Parent;

	Children = new TStack<TObject *>();

	if (Parent != NULL)
	{
		Parent->AddChild(this);
	}
}

TObject::~TObject()
{
	if (Parent != NULL)
	{
		Parent->RemoveChild(this);
	}
}

void TObject::AddChild(TObject *Child)
{

	Children->Add(Child);
}

void TObject::RemoveChild(TObject *Child)
{
	Children->Delete(Child);
}

class TApplication : public TObject
{
private:
	TActiveControl *CurrentActiveControl{NULL};
	bool isRun{false};
	TApplicationPrintCallback PrintCb{DefaultApplicationPrintCallBack};
	TApplicationPrintCallback PrintLnCb{DefaultApplicationPrintLnCallBack};

public:
	uint64_t Tick{0};
	TStack<TControl *> *Controls;
	TStack<TActiveControl *> *ActiveControls;
	bool DoStop{false};

	void Print(String Buffer)
	{

		PrintCb(this, Buffer);
	}

	void PrintLn(String Buffer)
	{

		PrintLnCb(this, Buffer);
	}

	void SetPrintLnProcedure(TApplicationPrintCallback Cb)
	{
		PrintLnCb = Cb;
	}

	void SetPrintProcedure(TApplicationPrintCallback Cb)
	{
		PrintCb = Cb;
	}

	bool AppIsRun()
	{
		return isRun;
	}
	TApplication() : TObject(NULL)
	{
		Controls = new TStack<TControl *>();
		ActiveControls = new TStack<TActiveControl *>();

#if defined(ESP8266)

		WiFi.begin();
		WiFiManager wifiManager;
		wifiManager.setDebugOutput(false);
		wifiManager.autoConnect("AutoConnectAP");

		ArduinoOTA.setPort(8266);
		ArduinoOTA.setHostname("ESP_Board");
		ArduinoOTA.setPassword("8764956");
		ArduinoOTA.begin(true);

#endif

	}

	~TApplication() {}

	void Stop();

	void NeedAction();

	void AddControl(TControl *Control);

	long int ControlsCount()
	{

		return Controls->GetCount();
	}

	void DeleteControl(TControl *Control);
	void AddActiveControl(TActiveControl *Control)
	{
		if (CurrentActiveControl == NULL)
			CurrentActiveControl = Control;
		ActiveControls->Add(Control);
	}
	void DeleteActiveControl(TActiveControl *Control)
	{
		ActiveControls->Delete(Control);
		if (ActiveControls->GetCount() == 0)
			CurrentActiveControl = NULL;
	}

	void Idle();
	uint64_t GetTick()
	{
		return Tick;
	}

	void Run()
	{
		Tick = 0;
		DoStop = false;
		isRun = true;

	}

	void (*OnIdle)(){EmptryEvent};
};

class TControl : public TObject
{
protected:
	TApplication *Application{NULL};

public:
	TControl(TObject *_Parent) : TObject(_Parent) {};
	TControl() : TObject(NULL) {};

	TApplication *GetApplication()
	{
		return Application;
	}

	void Register(TApplication *App)
	{
		Application = App;
		Application->AddControl(this);
	}

	~TControl();

	virtual void Idle()
	{
		OnIdle();
	}
	void (*OnIdle)(){EmptryEvent};
};

TControl::~TControl()
{
	Application->DeleteControl(this);
}

class TActiveControl : public TControl
{
private:
	/* data */

public:
	TActiveControl(TObject *_Parent) : TControl(_Parent)
	{
		Application->AddActiveControl(this);
	};
	~TActiveControl();

	void Action()
	{
		OnAction();
	}

	void Focus()
	{
		OnFocus();
	}

	void FocusLeave()
	{
		OnFocusLeave();
	}

	void (*OnAction)(){EmptryEvent};
	void (*OnFocus)(){EmptryEvent};
	void (*OnFocusLeave)(){EmptryEvent};
};

TActiveControl::~TActiveControl()
{
	Application->DeleteActiveControl(this);
}

void TApplication::NeedAction()
{
	if (CurrentActiveControl != NULL)
		CurrentActiveControl->Action();
}

void TApplication::DeleteControl(TControl *Control)
{
	Controls->Delete(Control);
}

void TApplication::Stop()
{
	isRun = false;
	DoStop = true;

	TControl *CurrentControl{NULL};
	CurrentControl = Controls->GetNext(CurrentControl);
}

void TApplication::Idle()
{
#if defined(ESP8266)
	static unsigned long WiFiConnectionTimeout = 0;

	ArduinoOTA.handle();

	if (WiFi.status() != WL_CONNECTED)
	{
		unsigned long CurrentTime = millis();
		if ((CurrentTime - WiFiConnectionTimeout) >= 10000)
		{
			WiFiConnectionTimeout = CurrentTime;
			Serial.println("Try reconnect WiFi");
			WiFi.reconnect();
		}
	}

#endif

	if (DoStop)
		return;

	TControl *CurrentControl{NULL};
	CurrentControl = Controls->GetNext(CurrentControl);

	while (CurrentControl != NULL)
	{
		if (DoStop)
			return;
		CurrentControl->Idle();
		CurrentControl = Controls->GetNext(CurrentControl);
	}

	OnIdle();
	Tick++;
}

void TApplication::AddControl(TControl *Control)
{
	Controls->Add(Control);
}
