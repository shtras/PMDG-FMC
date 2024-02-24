#pragma once

#include <utility>

template<typename T>
class finally
{
public:
    explicit finally(T&& f)
        : f_(std::move(f))
    {
    }
    ~finally()
    {
        f_();
    }

private:
    T f_;
};
