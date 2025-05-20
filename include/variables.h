template <typename T>
class TVariable {
protected:
    T Value{}; // Инициализация значением по умолчанию для типа T

public:
    void (*OnChange)(TVariable* Variable){nullptr};

    T GetValue() {
        return Value;
    };

    operator T() const {
        return this->Value;
    }

    virtual void SetValue(T _Value) {
        Value = _Value;
        if (OnChange != nullptr)
            OnChange(this);
    }

    TVariable& operator=(const T& _Value) {
        SetValue(_Value);
        return *this;
    }

    TVariable() = default;
    virtual ~TVariable() = default;
};
