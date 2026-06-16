#pragma once
#include <memory>

// NOTE - windows上使用此模板在.h做单例会出现两个实例(插件中使用框架中单例), 建议使用.h声明 .cpp实现
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