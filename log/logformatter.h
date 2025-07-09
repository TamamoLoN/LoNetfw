#pragma once

#include <memory>

namespace lon
{
namespace log
{
class LogFormatter
{
  public:
    using Ptr               = std::shared_ptr<LogFormatter>;
    explicit LogFormatter() = default;
    virtual ~LogFormatter() = default;
    std::string format();
};
} // namespace log
} // namespace lon