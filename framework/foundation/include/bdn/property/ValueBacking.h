#pragma once

#include <bdn/property/Backing.h>
#include <optional>

namespace bdn
{

    template <class ValType> class ValueBacking : public Backing<ValType>
    {
        using value_accessor_t = typename Backing<ValType>::value_accessor_t;

        template <class T> struct Compare
        {
            static bool not_equal(const T &left, const T &right) { return left != right; }
        };

        template <class _fp> struct Compare<std::function<_fp>>
        {
            static bool not_equal(const std::function<_fp> &left, const std::function<_fp> &right) { return true; }
        };

      public:
        ValueBacking() : _value() {}
        ValueBacking(ValType value) : _value(value) {}
        ValueBacking(const ValueBacking &other) : _value(other.get()) {}

        ValType get() const override { return _value; }

        void set(const ValType &value, bool notify = true) override
        {
            bool changed = false;

            {
                if (Compare<ValType>::not_equal(_value, value)) {
                    _value = value;
                    changed = true;
                }
            }

            if (changed && notify) {
                this->_pOnChange->notify(
                    std::dynamic_pointer_cast<value_accessor_t>(Backing<ValType>::shared_from_this()));
            }
        }

      protected:
        ValType _value;
    };
}
