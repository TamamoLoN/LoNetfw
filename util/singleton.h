#pragma once
#include "util/noncopyable.h"
#include <memory>
namespace lon
{
namespace util
{
template <typename T, typename V = void, int N = 0> class Singleton
{
  public:
    static T &Instance()
    {
        static T instance;
        return instance;
    }
};

template <typename T, typename V = void, int N = 0> class SingletonPtr
{
  public:
    static std::shared_ptr<T> Instance()
    {
        static std::shared_ptr<T> instance = std::make_shared<T>();
        return instance;
    }
};
} // namespace util
} // namespace lon