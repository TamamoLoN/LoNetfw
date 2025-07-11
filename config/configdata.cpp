#include "config/configdata.h"

namespace lon
{
namespace config
{
ConfigDataBase::ConfigDataBase(const std::string name, const std::string description)
    : m_name(util::toLower(name)), m_description(description)
{
}

std::string ConfigDataBase::getName() const { return m_name; }
std::string ConfigDataBase::getDescription() const { return m_description; }

} // namespace config
} // namespace lon