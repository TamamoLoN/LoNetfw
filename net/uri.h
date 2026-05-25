#pragma once

#include "net/address.h"

namespace lon
{
namespace net
{
#ifdef _WIN32
#define in_port_t uint16_t
#endif
/**
 * Uri
 * @brief Uri类用于封装uri
 * @note
 *   foo://user@sylar.com:8042/over/there?name=ferret#nose
       \_/   \______________/\_________/ \_________/ \__/
        |           |            |            |        |
     scheme     authority       path        query   fragment
 */
class LON_API Uri
{
  public:
    using Ptr = std::shared_ptr<Uri>;
    explicit Uri();
    virtual ~Uri();
    static Uri::Ptr create(const std::string &uristr);
    Address::Ptr create() const;
    LON_API friend std::ostream &operator<<(std::ostream &os, const Uri &uri);
    std::ostream &toString(std::ostream &os) const;
    std::string toString() const;

    const std::string getScheme() const;
    void setScheme(const std::string &scheme);

    const std::string getUserinfo() const;
    void setUserinfo(const std::string &userinfo);

    const std::string getHost() const;
    void setHost(const std::string &host);

    const in_port_t getPort() const;
    void setPort(in_port_t port);

    const std::string getPath() const;
    void setPath(const std::string &path);

    const std::string getQuery() const;
    void setQuery(const std::string &query);

    const std::string getFragment() const;
    void setFragment(const std::string &fragment);

  private:
    bool isDefaultPort() const;

  private:
    // scheme
    std::string m_scheme;
    // authority
    std::string m_userinfo;
    std::string m_host;
    in_port_t m_port;
    // path
    std::string m_path;
    // query
    std::string m_query;
    // fragment
    std::string m_fragment;
};
} // namespace net
} // namespace lon