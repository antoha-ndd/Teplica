#pragma once

template <typename T>
class TVariable;

template <typename T>
class TVariable
{
private:
    T Value{NULL};

public:
    void (*OnChange)(TVariable *Variable){NULL};
    T GetValue()
    {
        return Value;
    };

    operator T() const
    {
        return this->Value;
    }
    void SetValue(T _Value)
    {
        Value = _Value;
        if (OnChange != NULL)
            OnChange(this);
    };
    TVariable &operator=(const T &_Value)
    {
        SetValue(_Value);
        return *this;
    }

    TVariable(){};
    ~TVariable(){};
};
