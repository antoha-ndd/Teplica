#pragma once

#include <stdint.h>
#include <stddef.h>
#include "stack.h"

typedef unsigned long TTimeStamp;
typedef unsigned int ObjectId;
typedef void (*TEmptyCallback)();
typedef unsigned long int (*TTimerValueCallback)();

inline void EmptryEvent() {}
extern TTimerValueCallback GetTimerValue;

inline ObjectId GetGlobalIDValue()
{
	static ObjectId LastObjectId{0};
	LastObjectId++;
	return LastObjectId;
}

class TObject;
class TApplication;
class TControl;

class TObject
{
private:
	TObject *Parent{NULL};
	ObjectId ID{0};

public:
	TStack<TObject *> *Children{NULL};

	TObject(TObject *_Parent);
	virtual ~TObject();

	void AddChild(TObject *Child);
	void RemoveChild(TObject *Child);

	ObjectId GetID()
	{
		return ID;
	}

	TObject *GetParent()
	{
		return Parent;
	}
};

class TApplication : public TObject
{
private:
	bool isRun{false};

public:
	uint64_t Tick{0};
	TStack<TControl *> *Controls{NULL};
	TEmptyCallback OnIdle{EmptryEvent};

	TApplication();
	virtual ~TApplication();

	void Run();
	void Idle();
	void AddControl(TControl *Control);
	void DeleteControl(TControl *Control);
};

class TControl : public TObject
{
protected:
	TApplication *Application{NULL};

public:
	TEmptyCallback OnIdle{EmptryEvent};

	TControl(TObject *_Parent) : TObject(_Parent) {}
	TControl() : TObject(NULL) {}
	virtual ~TControl();

	TApplication *GetApplication()
	{
		return Application;
	}

	void Register(TApplication *App);

	virtual void Idle()
	{
		OnIdle();
	}
};

inline TObject::TObject(TObject *_Parent)
{
	ID = GetGlobalIDValue();
	Parent = _Parent;
	Children = new TStack<TObject *>();

	if (Parent != NULL)
		Parent->AddChild(this);
}

inline TObject::~TObject()
{
	if (Parent != NULL)
		Parent->RemoveChild(this);

	delete Children;
}

inline void TObject::AddChild(TObject *Child)
{
	if (Child == NULL)
		return;

	Children->Add(Child);
}

inline void TObject::RemoveChild(TObject *Child)
{
	if (Child == NULL)
		return;

	Children->Delete(Child);
}

inline TApplication::TApplication() : TObject(NULL)
{
	Controls = new TStack<TControl *>();
}

inline TApplication::~TApplication()
{
	delete Controls;
}

inline void TApplication::Run()
{
	Tick = 0;
	isRun = true;
}

inline void TApplication::Idle()
{
	if (!isRun)
		return;

	TControl *CurrentControl{NULL};
	CurrentControl = Controls->GetNext(CurrentControl);

	while (CurrentControl != NULL)
	{
		CurrentControl->Idle();
		CurrentControl = Controls->GetNext(CurrentControl);
	}

	OnIdle();
	Tick++;
}

inline void TApplication::AddControl(TControl *Control)
{
	if (Control == NULL)
		return;

	Controls->AddUniqe(Control);
}

inline void TApplication::DeleteControl(TControl *Control)
{
	if (Control == NULL)
		return;

	Controls->Delete(Control);
}

inline TControl::~TControl()
{
	if (Application != NULL)
	{
		Application->DeleteControl(this);
		Application = NULL;
	}
}

inline void TControl::Register(TApplication *App)
{
	if (App == NULL || Application == App)
		return;

	if (Application != NULL)
		Application->DeleteControl(this);

	Application = App;
	Application->AddControl(this);
}
