#pragma once

namespace lon
{
namespace util
{
class Nonecopyable
{
  public:
    Nonecopyable()  = default;
    ~Nonecopyable() = default;

  private:
    Nonecopyable(const Nonecopyable &) = delete;
    Nonecopyable(Nonecopyable &&)      = delete;
    Nonecopyable &operator=(const Nonecopyable &) = delete;
    Nonecopyable &operator=(Nonecopyable &&) = delete;
};
} // namespace util
} // namespace lon
