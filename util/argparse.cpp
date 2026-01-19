#include "util/argparse.h"

namespace lon
{
namespace util
{
Argument::Argument()
    : m_val({}), m_help(""), m_action(""), m_choices({}), m_default(nullptr), m_nargs("1")
{
}

Argument *Argument::help(const std::string &h)
{
    m_help = h;
    return this;
}

Argument *Argument::action(const std::string &a)
{
    m_action = a;
    if (m_action == "store_true")
    {
        m_default.reset(new std::string("false"));
    }
    return this;
}

Argument *Argument::defaultValue(const std::string &d)
{
    m_default.reset(new std::string(d));
    return this;
}

Argument *Argument::choices(const std::vector<std::string> &c)
{
    m_choices = c;
    return this;
}

Argument *Argument::nargs(const std::string &n)
{
    m_nargs = n;
    return this;
}

Argument *Argument::name(const std::string &name)
{
    m_name = name;
    return this;
}

bool Argument::isMulti() const
{
    return m_nargs == "+" || m_nargs == "*" ||
           (m_nargs != "+" && m_nargs != "*" && util::lexical_cast<int>(m_nargs) > 1);
}

std::string Argument::toUpper(const std::string &s)
{
    auto res = s;
    res.erase(0, s.find_first_not_of('-'));
    for (char &c : res)
    {
        if (c >= 'a' && c <= 'z')
        {
            c -= ('a' - 'A');
        }
    }
    return res;
}

bool Argument::isArgHasValue()
{
    if (m_val.empty() && !m_default && m_nargs != "*")
    {
        return false;
    }
    else if (m_val.empty() && m_default)
    {
        m_val.push_back(*m_default);
    }
    return true;
}

bool Argument::isArgCountValid() const
{
    if (m_nargs != "+" && m_nargs != "*")
    {
        int nargs = util::lexical_cast<int>(m_nargs);
        if (nargs != m_val.size())
        {
            return false;
        }
    }
    return true;
}

bool Argument::isArgChoicesValid() const
{
    if (!m_choices.empty())
    {
        for (const auto &val : m_val)
        {
            if (std::find(m_choices.begin(), m_choices.end(), val) == m_choices.end())
            {
                return false;
            }
        }
    }
    return true;
}

std::string Argument::getHelp(const std::string &name) const
{
    bool is_optional  = name.at(0) == '-';
    std::string uname = is_optional ? toUpper(name) : name;
    if (m_action == "store_true" || m_action == "help")
    {
        return name;
    }
    else
    {
        if (!m_choices.empty())
        {
            uname = "{";
            for (const auto &choice : m_choices)
            {
                uname += choice + ",";
            }
            uname.erase(uname.size() - 1);
            uname += "}";
        }
        std::string res = "";
        if (is_optional)
        {
            res += name + " ";
        }
        if (m_nargs == "+")
        {
            res += uname + " [" + uname + " ...]";
        }
        else if (m_nargs == "*")
        {
            res += "[" + uname + " [" + uname + " ...]]";
        }
        else
        {
            int count = util::lexical_cast<int>(m_nargs);
            // int j     = is_optional ? count - 1 : count;
            int j = count;
            for (int i = 0; i < j; ++i)
            {
                res += uname + " ";
            }
            res.erase(res.size() - 1);
        }
        return res;
    }
}

ArgumentParser::ArgumentParser()
    : m_name(""), m_description(""), m_positional_args({}), m_optional_args({})
{
    addArgument(std::vector<std::string>{"-h", "--help"})
        ->help("show this help message and exit")
        ->action("help")
        ->m_val.push_back("help");
}

ArgumentParser::~ArgumentParser() {}

void ArgumentParser::addDescription(const std::string &description) { m_description = description; }

Argument::Ptr ArgumentParser::addArgument(const std::string &name)
{
    return addArgument(std::vector<std::string>{name});
}

Argument::Ptr ArgumentParser::addArgument(const std::vector<std::string> &names)
{
    if (names.empty())
    {
        throw std::invalid_argument("names must not be empty");
    }
    auto arg = Argument::Ptr(new Argument);
    for (const auto &name : names)
    {
        if (name.empty())
        {
            handleError("names must not be empty");
        }
        if (name.at(0) == '-')
        {
            m_optional_args[name] = arg;
        }
        else
        {
            m_positional_args.push_back(std::make_pair(name, arg));
        }
    }
    return arg;
}

void ArgumentParser::parse(int argc, char *argv[])
{
    std::unordered_map<Argument::Ptr, bool> handled = {};
    m_name                                          = argv[0];
    // 仅存储参数
    for (int cnt = 1; cnt < argc; ++cnt)
    {
        std::string name = argv[cnt];
        if (name.at(0) != '-')
        {
            if (m_positional_args.empty())
            {
                handleError("no positional arguments");
            }
            // TODO - 位置参数暂时只处理一个
            m_positional_args.at(0).second->m_val.push_back(name);
            continue;
        }
        // getArgName(name);
        auto it = m_optional_args.find(name);
        if (it == m_optional_args.end())
        {
            handleError("unknown argument: " + name);
        }
        if (handled.find(it->second) != handled.end())
        {
            // 已经处理过该参数，跳过当前循环
            while (cnt + 1 < argc && argv[cnt + 1][0] != '-')
            {
                ++cnt;
            }
            continue;
        }
        // 如果为bool类型参数，则无需后续参数
        if (it->second->m_action == "store_true")
        {
            it->second->m_val.push_back("true");
            // 这里不存入map，防止重复处理
            continue;
        }
        // 如果为help参数，则打印帮助信息并退出程序
        else if (it->second->m_action == "help")
        {
            std::cout << help() << std::endl;
            throw std::runtime_error("help argument, exit program");
        }
        while (cnt + 1 < argc && argv[cnt + 1][0] != '-')
        {
            if (it->second->m_nargs != "+" && it->second->m_nargs != "*")
            {
                if (util::lexical_cast<int>(it->second->m_nargs) <= it->second->m_val.size())
                {
                    break;
                }
            }
            it->second->m_val.push_back(argv[++cnt]);
        }
        handled[it->second] = true;
    }
    handled.clear();
    // 判断参数合法性
    for (const auto &arg : m_positional_args)
    { // 判断参数是否有值
        if (!arg.second->isArgHasValue())
        {
            handleError("missing argument for '" + arg.first + "'");
        }

        // 判断参数个数是否合法
        if (!arg.second->isArgCountValid())
        {
            handleError(
                "error: fatal argument: " + arg.first + ", nargs=" + arg.second->m_nargs +
                ", exactly args=" + util::lexical_cast<std::string>(arg.second->m_val.size()));
        }
    }
    for (const auto &arg : m_optional_args)
    {
        if (handled.find(arg.second) != handled.end())
        {
            // 已经处理过该参数，跳过当前循环
            continue;
        }
        handled[arg.second] = true;
        // 判断参数是否有值
        if (!arg.second->isArgHasValue())
        {
            handleError("missing argument for '" + arg.first + "'");
        }

        // 判断参数个数是否合法
        if (!arg.second->isArgCountValid())
        {
            handleError(
                "error: fatal argument: " + arg.first + ", nargs=" + arg.second->m_nargs +
                ", exactly args=" + util::lexical_cast<std::string>(arg.second->m_val.size()));
        }

        // 判断选择参数是否合法
        if (!arg.second->isArgChoicesValid())
        {
            std::string choices_str;
            for (const auto &choice : arg.second->m_choices)
            {
                choices_str += choice + ",";
            }
            choices_str.erase(choices_str.size() - 1);
            handleError("invalid choice for '" + arg.first + "' (choose from " + choices_str + ")");
        }
    }
}

const std::string ArgumentParser::usage() const
{
    std::unordered_map<Argument::Ptr, bool> showed = {};
    std::string res                                = "usage: " + m_name;
    for (const auto &arg : m_optional_args)
    {
        if (showed.find(arg.second) != showed.end())
        {
            continue;
        }
        showed[arg.second] = true;
        res += " [" + arg.second->getHelp(arg.first) + "]";
    }
    for (const auto &arg : m_positional_args)
    {
        res += " " + arg.second->getHelp(arg.first);
    }

    return res;
}

const std::string ArgumentParser::help() const
{
    std::unordered_map<Argument::Ptr, std::vector<std::string>> showed = {};
    std::string res = usage() + "\n\n" + m_description + "\n";
    if (!m_positional_args.empty())
    {
        res += "\npositional arguments:\n";
    }
    for (const auto &arg : m_positional_args)
    {
        res += formatHelper(arg.first, arg.second->m_help);
    }
    if (!m_optional_args.empty())
    {
        res += "\noptional arguments:\n";
    }
    for (const auto &arg : m_optional_args)
    {
        showed[arg.second].push_back(arg.first);
    }
    for (const auto &arg : showed)
    {
        std::string left = "";
        for (const auto &it : arg.second)
        {
            left += arg.first->getHelp(it) + ", ";
        }
        left.erase(left.size() - 2);
        res += formatHelper(left, arg.first->m_help);
    }
    return res;
}

void ArgumentParser::getArgName(std::string &name)
{
    auto pos = name.find_first_not_of('-');
    if (pos != std::string::npos)
    {
        name.erase(0, pos);
    }
    else
    {
        name.clear();
    }
}

void ArgumentParser::handleError(const std::string &msg) const
{
    std::cout << usage() << std::endl;
    std::cout << "error: " << msg << std::endl;
    // exit(0);
    throw std::runtime_error(msg);
}

std::string ArgumentParser::formatHelper(const std::string &left, const std::string &right,
                                         size_t indent, size_t help_col, size_t width) const
{
    std::ostringstream oss;
    std::string indent_str(indent, ' ');

    oss << indent_str << left;

    if (indent + left.size() >= help_col)
    {
        oss << "\n" << std::string(help_col, ' ');
    }
    else
    {
        oss << std::string(help_col - indent - left.size(), ' ');
    }

    size_t pos         = 0;
    size_t right_width = width - help_col;

    while (pos < right.size())
    {
        size_t len = std::min(right_width, right.size() - pos);
        oss << right.substr(pos, len) << "\n";
        pos += len;
        if (pos < right.size())
        {
            oss << std::string(help_col, ' ');
        }
    }

    return oss.str();
}
} // namespace util
} // namespace lon
