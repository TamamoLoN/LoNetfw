#pragma once

namespace lon
{
namespace util
{
class Nonecopyable
{
  public:
    Nonecopyable()                     = default;
    ~Nonecopyable()                    = default;
    Nonecopyable(const Nonecopyable &) = delete;
    Nonecopyable &operator=(const Nonecopyable &) = delete;
};
} // namespace util
} // namespace lon
