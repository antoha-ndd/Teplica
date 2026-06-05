#pragma once
#include <stddef.h>

template <typename T>

struct TStackList
{
    T Value;
    TStackList *Next;
    TStackList *Prev;
};

template <typename T>
class TStack
{
private:
    TStackList<T> *First{NULL};
    TStackList<T> *Last{NULL};
    int Count{0};

public:
    int GetCount();
    int Find(T Value);
    void Delete(T Value);
    void DeleteByIndex(int Index);
    T GetValue(int Index);
    void Add(T Value);
    void AddUniqe(T Value);
    T GetNext(T Value);
    T GetPrev(T Value);
    T GetNextCircle(T Value);
    bool IsEmpty();
    bool IsLast(T Value);
    bool IsFirst(T Value);
    T Extract(TStackList<T> *Stack);
    T GetFirst();
    T GetLast();

    TStack(){};
    ~TStack();
};

template <typename T>
int TStack<T>::GetCount()
{
    return Count;
}


template <typename T>
int TStack<T>::Find(T Value)
{

    if (First == NULL)
        return -1;

    TStackList<T> *Array = First;

    for (long int i = 0; Array != NULL; i++)
    {
        if (Array->Value == Value)
            return i;

        Array = Array->Next;
    }

    return -1;
}

template <typename T>
void TStack<T>::Delete(T Value)
{
    TStackList<T> *Array = First;

    while (Array != NULL)
    {
        if (Array->Value == Value)
        {
            TStackList<T> *Prev = Array->Prev;
            TStackList<T> *Next = Array->Next;

            if (Prev != NULL)
                Prev->Next = Next;
            else
                First = Next;

            if (Next != NULL)
                Next->Prev = Prev;
            else
                Last = Prev;

            delete Array;
            Count--;
            return;
        }

        Array = Array->Next;
    }
}

template <typename T>
void TStack<T>::DeleteByIndex(int Index)
{
    if (Index < 0 || (Index + 1) > Count)
        return;

    TStackList<T> *Array = First;

    for (int Iter = 0; Iter < Index; Iter++)
    {
        Array = Array->Next;
    }

    TStackList<T> *Prev = Array->Prev;
    TStackList<T> *Next = Array->Next;

    if (Prev != NULL)
        Prev->Next = Next;
    else
        First = Next;

    if (Next != NULL)
        Next->Prev = Prev;
    else
        Last = Prev;

    delete Array;
    Count--;
}

template <typename T>
T TStack<T>::GetValue(int Index)
{
    if (First == NULL)
        return NULL;

    if ((Index < 0) || ((Index + 1) > Count))
        return NULL;

    TStackList<T> *Array = First;

    for (int i = 0; i != Index; i++)
    {
        Array = Array->Next;
    }

    return Array->Value;
}

template <typename T>
void TStack<T>::Add(T Value)
{

    if (First == NULL)
    {

        First = new TStackList<T>;
        First->Next = NULL;
        First->Prev = NULL;
        First->Value = NULL;
        Last = First;
        Count = 0;
    }
    else
    {

        Last->Next = new TStackList<T>;
        Last->Next->Prev = Last;
        Last = Last->Next;
        Last->Next = NULL;
    }

    Count++;
    Last->Value = Value;
}

template <typename T>
void TStack<T>::AddUniqe(T Value)
{
    if (Find(Value) == -1)
        Add(Value);
}

template <typename T>
T TStack<T>::GetNext(T Value)
{
    if (Value == NULL)
    {
        return Extract(First);
    }

    int Index = Find(Value);
    if (Index != -1)
        return GetValue(Index + 1);

    return NULL;
}

template <typename T>
T TStack<T>::GetPrev(T Value)
{
    int Index = Find(Value);
    if (Index != -1)
        return GetValue(Index - 1);

    return NULL;
}

template <typename T>
bool TStack<T>::IsEmpty()
{
    return First == NULL;
}

template <typename T>
bool TStack<T>::IsLast(T Value)
{
    if (Last == NULL)
        return false;

    return Last->Value == Value;
}

template <typename T>
bool TStack<T>::IsFirst(T Value)
{
    if (First == NULL)
        return false;

    return First->Value == Value;
}

template <typename T>
T TStack<T>::GetFirst()
{
    if (First != NULL)
        return First->Value;

    return NULL;
}

template <typename T>
T TStack<T>::GetLast()
{
    if (Last != NULL)
        return Last->Value;

    return NULL;
}

template <typename T>
T TStack<T>::Extract(TStackList<T> *Stack)
{

    if (Stack == NULL)
        return NULL;
    else
        return Stack->Value;
}

template <typename T>
T TStack<T>::GetNextCircle(T Value)
{
    if ((Value == NULL) || IsLast(Value))
        return Extract(First);

    int Index = Find(Value);
    if (Index != -1)
        return GetValue(Index + 1);

    return Extract(First);
}

template <typename T>
TStack<T>::~TStack()
{
    TStackList<T> *Array = First;

    while (Array != NULL)
    {
        TStackList<T> *Next = Array->Next;
        delete Array;
        Array = Next;
    }

    First = NULL;
    Last = NULL;
    Count = 0;
}