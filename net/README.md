# LoNetfw 网络基础模块架构设计

## 架构概览

```
Net 网络基础模块
    │
    ├─ Address（地址抽象）
    │    ├─ IPAddress（IP 地址）
    │    │    ├─ IPv4Address
    │    │    └─ IPv6Address
    │    ├─ UnixAddress（Unix 套接字地址）
    │    └─ UnknownAddress（未知地址）
    │
    ├─ Socket（套接字封装）
    │    ├─ TCP Socket
    │    ├─ UDP Socket
    │    └─ SSL Socket
    │
    ├─ SocketStream（套接字流）
    │    └─ Socket 到 Stream 的适配
    │
    └─ Uri（URI 解析）
         └─ URL 解析和构造
```

```mermaid
graph TD
    A[Address 地址] --> B[IPv4Address]
    A --> C[IPv6Address]
    A --> D[UnixAddress]
    
    E[Socket 契接字] --> F[TCP]
    E --> G[UDP]
    E --> H[SSL]
    
    I[SocketStream 流] --> J[Stream 接口]
    I --> E
    
    K[Uri 解析] --> L[URL]
```

---

## 1. 什么是网络基础模块？（通俗理解）

### 1.1 用生活例子理解网络基础

想象你要寄快递：

| 角色 | 对应概念 | 作用 |
|------|---------|------|
| 收件地址 | Address | 定位目标 |
| 电话号码 | IPAddress | IP 地址 |
| 门牌号 | Port | 端口号 |
| 寄送方式 | Socket | TCP/UDP |
| 包裹 | Data | 发送的数据 |
| 快递单 | Uri | URL 地址 |

**网络基础模块就是网络通信的"基础设施"。**

### 1.2 模块的作用

| 模块 | 作用 | 类比 |
|------|------|------|
| **Address** | 地址抽象 | 地址管理系统 |
| **Socket** | 套接字封装 | 快递寄送服务 |
| **SocketStream** | 流式读写 | 数据传输管道 |
| **Uri** | URL 解析 | 地址格式化 |

---

## 2. Address 地址模块详解

### 2.1 Address 类层次

```
Address（基类）
    │
    ├─ IPAddress（IP 地址）
    │    │
    │    ├─ IPv4Address（IPv4 地址）
    │    │    └─ sockaddr_in
    │    │
    │    └─ IPv6Address（IPv6 地址）
    │    │    └─ sockaddr_in6
    │    │
    │    └─ 方法
    │         ├─ getPort() / setPort()
    │         ├─ broadcastAddress()（广播地址）
    │         ├─ networkAddress()（网络地址）
    │         └─ subnetMask()（子网掩码）
    │
    ├─ UnixAddress（Unix 套接字地址）
    │    └─ sockaddr_un（仅 Linux）
    │
    └─ UnknownAddress（未知地址）
         └─ sockaddr
```

### 2.2 Address 核心功能

**地址解析**：

```cpp
// 解析地址（多种方式）
lon::net::Address::Ptr addr;

// 1. 从字符串解析（域名/IP + 端口）
lon::net::Address::parse(addr, "www.example.com:80", AF_INET);
lon::net::Address::parse(addr, "192.168.1.1:8080", AF_INET);
lon::net::Address::parse(addr, "[::1]:8080", AF_INET6);

// 2. 批量解析（解析所有地址）
std::vector<Address::Ptr> addrs;
lon::net::Address::parse(addrs, "www.baidu.com:http", AF_INET);

// 3. 创建 IPv4 地址
IPv4Address::Ptr ipv4 = IPv4Address::create("127.0.0.1", 8080);

// 4. 创建 IPv6 地址
IPv6Address::Ptr ipv6 = IPv6Address::create("::1", 8080);

// 5. 获取本机网卡地址
std::multimap<std::string, std::pair<Address::Ptr, uint32_t>> if_addrs;
lon::net::Address::getInterfaceAddresses(if_addrs, AF_INET);
```

**地址信息**：

```cpp
Address::Ptr addr;
// ...

// 获取信息
int family = addr->getFamily();           // 地址族（AF_INET/AF_INET6）
sockaddr* sa = addr->getAddr();           // sockaddr 结构
socklen_t len = addr->getAddrLen();       // 地址长度
std::string str = addr->toString();       // 字符串表示（如 "192.168.1.1:8080"）

// 对于 IPAddress
IPAddress::Ptr ip_addr;
uint16_t port = ip_addr->getPort();       // 端口号
ip_addr->setPort(9090);                   // 设置端口

// 子网计算
IPAddress::Ptr broadcast = ip_addr->broadcastAddress(24);  // 广播地址
IPAddress::Ptr network = ip_addr->networkAddress(24);      // 网络地址
IPAddress::Ptr subnet = ip_addr->subnetMask(24);           // 子网掩码
```

### 2.3 Address 使用示例

```cpp
#include "net/address.h"

void address_example() {
    // 解析域名
    lon::net::Address::Ptr addr;
    if (lon::net::Address::parse(addr, "www.example.com:80", AF_INET)) {
        std::cout << "解析成功: " << addr->toString() << std::endl;
    }
    
    // 解析 IP 地址
    lon::net::IPv4Address::Ptr ipv4;
    ipv4 = lon::net::IPv4Address::create("192.168.1.100", 8080);
    std::cout << "IPv4: " << ipv4->toString() << std::endl;
    std::cout << "端口: " << ipv4->getPort() << std::endl;
    
    // 计算子网
    auto broadcast = ipv4->broadcastAddress(24);
    auto network = ipv4->networkAddress(24);
    auto subnet = ipv4->subnetMask(24);
    
    std::cout << "广播地址: " << broadcast->toString() << std::endl;
    std::cout << "网络地址: " << network->toString() << std::endl;
    std::cout << "子网掩码: " << subnet->toString() << std::endl;
    
    // 获取本机网卡地址
    std::multimap<std::string, std::pair<lon::net::Address::Ptr, uint32_t>> interfaces;
    lon::net::Address::getInterfaceAddresses(interfaces, AF_INET);
    
    for (auto& pair : interfaces) {
        std::cout << "网卡: " << pair.first 
                  << ", 地址: " << pair.second.first->toString()
                  << ", 子网掩码长度: " << pair.second.second << std::endl;
    }
}
```

---

## 3. Socket 套接字模块详解

### 3.1 Socket 类设计

```
Socket（套接字封装）
    │
    ├─ 属性
    │    ├─ m_sockfd（文件描述符）
    │    ├─ m_family（地址族：IPv4/IPv6）
    │    ├─ m_type（类型：TCP/UDP）
    │    ├─ m_protocol（协议）
    │    ├─ m_is_connected（是否已连接）
    │    └─ m_local_addr / m_peer_addr（本地/对端地址）
    │
    ├─ 类型
    │    ├─ TCP（SOCK_STREAM）
    │    └─ UDP（SOCK_DGRAM）
    │
    ├─ 地址族
    │    ├─ IPV4（AF_INET）
    │    ├─ IPV6（AF_INET6）
    │    └─ UNIX（AF_UNIX，仅 Linux）
    │
    └─ 方法
         ├─ 创建/连接/监听/关闭
         ├─ 发送/接收数据
         ├─ 选项设置
         └─ 获取地址信息
```

### 3.2 Socket 核心功能

**创建和连接**：

```cpp
// 创建 Socket
lon::net::Socket::Ptr sock;

// 1. 指定类型创建
sock = lon::net::Socket::create(lon::net::Socket::IPV4, lon::net::Socket::TCP);

// 2. 根据地址创建
lon::net::Address::Ptr addr;
lon::net::Address::parse(addr, "192.168.1.1:8080", AF_INET);
sock = lon::net::Socket::create(addr, lon::net::Socket::TCP);

// 连接服务器
bool success = sock->connect(addr);            // 阻塞连接
bool success = sock->connect(addr, 3000);      // 超时连接（3秒）

// 绑定地址（服务器端）
sock->bind(addr);

// 监听（服务器端）
sock->listen(128);  // backlog = 128

// 接受连接（服务器端）
lon::net::Socket::Ptr client = sock->accept();
```

**数据传输**：

```cpp
// 发送数据
char buf[] = "Hello World";
ssize_t n = sock->send(buf, sizeof(buf), 0);

// 发送多个缓冲区
iovec iov[2];
iov[0].iov_base = buf1;
iov[0].iov_len = len1;
iov[1].iov_base = buf2;
iov[1].iov_len = len2;
ssize_t n = sock->send(iov, 2, 0);

// 发送 UDP 数据（指定目标地址）
lon::net::Address::Ptr dest;
sock->sendto(buf, sizeof(buf), dest, 0);

// 接收数据
char recv_buf[1024];
ssize_t n = sock->recv(recv_buf, sizeof(recv_buf), 0);

// 接收多个缓冲区
iovec iov[2];
iov[0].iov_base = buf1;
iov[0].iov_len = len1;
iov[1].iov_base = buf2;
iov[1].iov_len = len2;
ssize_t n = sock->recv(iov, 2, 0);

// 接收 UDP 数据（获取源地址）
lon::net::Address::Ptr src;
sock->recvfrom(recv_buf, sizeof(recv_buf), src, 0);
```

**选项设置**：

```cpp
// 获取超时时间
int64_t send_timeout = sock->getSendTimeout();
int64_t recv_timeout = sock->getRecvTimeout();

// 设置超时时间
sock->setSendTimeout(3000);  // 发送超时 3 秒
sock->setRecvTimeout(3000);  // 接收超时 3 秒

// 获取/设置 socket 选项
int keepalive = 0;
sock->getOption(SOL_SOCKET, SO_KEEPALIVE, keepalive);
sock->setOption(SOL_SOCKET, SO_KEEPALIVE, 1);

// 获取地址信息
lon::net::Address::Ptr local = sock->getLocalAddress();
lon::net::Address::Ptr peer = sock->getPeerAddress();

// 获取 socket 信息
int family = sock->getFamily();
int type = sock->getType();
int protocol = sock->getProtocol();
int sockfd = sock->getSocket();
int error = sock->getError();

// 状态检查
bool connected = sock->isConnected();
bool valid = sock->isValid();

// 取消 IO 事件（用于 IOScheduler）
sock->cancelRead();
sock->cancelWrite();
sock->cancelAll();
```

### 3.3 Socket 使用示例

**TCP 客户端**：

```cpp
#include "net/socket.h"
#include "net/address.h"

void tcp_client() {
    // 解析服务器地址
    lon::net::Address::Ptr addr;
    lon::net::Address::parse(addr, "www.example.com:80", AF_INET);
    
    // 创建 TCP Socket
    lon::net::Socket::Ptr sock = lon::net::Socket::create(addr);
    
    // 连接服务器
    if (!sock->connect(addr)) {
        std::cerr << "连接失败" << std::endl;
        return;
    }
    
    std::cout << "已连接: " << sock->toString() << std::endl;
    
    // 发送数据
    std::string request = "GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
    sock->send(request.c_str(), request.size(), 0);
    
    // 接收响应
    char buf[4096];
    ssize_t n = sock->recv(buf, sizeof(buf), 0);
    
    std::cout << "收到 " << n << " 字节" << std::endl;
    std::cout << std::string(buf, n) << std::endl;
    
    // 关闭
    sock->close();
}
```

**TCP 服务器**：

```cpp
#include "net/socket.h"
#include "net/address.h"
#include "scheduler/ioscheduler.h"

void handle_client(lon::net::Socket::Ptr client) {
    std::cout << "新客户端: " << client->toString() << std::endl;
    
    // 接收数据
    char buf[1024];
    while (true) {
        ssize_t n = client->recv(buf, sizeof(buf), 0);
        if (n <= 0) {
            std::cout << "客户端断开" << std::endl;
            break;
        }
        
        // 发送响应
        client->send(buf, n, 0);
    }
    
    client->close();
}

void tcp_server() {
    // 创建监听地址
    lon::net::IPv4Address::Ptr addr = lon::net::IPv4Address::create("0.0.0.0", 8080);
    
    // 创建 TCP Socket
    lon::net::Socket::Ptr sock = lon::net::Socket::create(addr);
    
    // 绑定
    if (!sock->bind(addr)) {
        std::cerr << "绑定失败" << std::endl;
        return;
    }
    
    // 监听
    if (!sock->listen(128)) {
        std::cerr << "监听失败" << std::endl;
        return;
    }
    
    std::cout << "服务器启动: " << addr->toString() << std::endl;
    
    // 接受连接循环
    while (true) {
        lon::net::Socket::Ptr client = sock->accept();
        if (client) {
            // 创建协程处理客户端
            lon::scheduler::IOScheduler::getThis()->schedule([client]() {
                handle_client(client);
            });
        }
    }
}
```

---

## 4. SocketStream 流模块详解

### 4.1 SocketStream 的作用

**SocketStream = 将 Socket 封装成 Stream（流）接口**

```
┌─────────────────────────────────────────────────┐
│              SocketStream                        │
│                                                 │
│  Socket（原始套接字）                            │
│       ↓                                         │
│  SocketStream（封装）                            │
│       ↓                                         │
│  Stream 接口（统一读写接口）                     │
│       ↓                                         │
│  HTTP/WebSocket 等上层模块使用                   │
└─────────────────────────────────────────────────┘
```

**为什么需要 SocketStream？**

| 对比项 | 直接使用 Socket | 使用 SocketStream |
|--------|----------------|------------------|
| 接口 | send/recv（socket 专用） | read/write（通用流接口） |
| 继承 | 不能继承 | 继承 Stream 基类 |
| 统一性 | 不统一 | 统一接口 |

### 4.2 SocketStream 核心功能

```cpp
class SocketStream : public util::Stream {
public:
    // 构造函数
    SocketStream(const Socket::Ptr& socket, bool proxy = true);
    
    // Stream 接口实现
    ssize_t read(void* buf, size_t len) override;
    ssize_t read(const ByteArray::Ptr& buf, size_t len) override;
    
    ssize_t write(const void* buf, size_t len) override;
    ssize_t write(const ByteArray::Ptr& buf, size_t len) override;
    
    void close() override;
    
    // Socket 相关
    Socket::Ptr getSocket() const;
    bool isConnected() const;
    bool isEof() const;  // 是否到达末尾（连接断开）
    
protected:
    Socket::Ptr m_socket;
    bool m_proxy;        // 是否代理（析构时关闭 socket）
    bool m_eof;          // 是否 EOF
};
```

### 4.3 SocketStream 使用示例

```cpp
#include "net/socketstream.h"
#include "net/socket.h"
#include "net/address.h"

void socketstream_example() {
    // 创建 Socket
    lon::net::Address::Ptr addr;
    lon::net::Address::parse(addr, "192.168.1.1:8080", AF_INET);
    
    lon::net::Socket::Ptr sock = lon::net::Socket::create(addr);
    sock->connect(addr);
    
    // 创建 SocketStream
    lon::net::SocketStream::Ptr stream = 
        std::make_shared<lon::net::SocketStream>(sock, true);
    
    // 使用 Stream 接口读写
    char buf[1024];
    ssize_t n = stream->read(buf, sizeof(buf));  // 使用 read 而不是 recv
    
    stream->write(buf, n);  // 使用 write 而不是 send
    
    // 检查状态
    if (stream->isEof()) {
        std::cout << "连接已断开" << std::endl;
    }
    
    // 关闭
    stream->close();  // 如果 proxy=true，会自动关闭 socket
}
```

---

## 5. Uri URL 模块详解

### 5.1 Uri 的作用

**Uri = URL 解析和构造**

```
URL 示例：http://www.example.com:8080/path?query=value#fragment

解析结果：
    ├─ scheme（协议）：http
    ├─ host（主机）：www.example.com
    ├─ port（端口）：8080
    ├─ path（路径）：/path
    ├─ query（查询）：query=value
    └─ fragment（片段）：fragment
```

### 5.2 Uri 核心功能

```cpp
class Uri {
public:
    // 创建
    static Uri::Ptr create(const std::string& url);
    
    // 属性获取
    const std::string& getScheme() const;    // 协议
    const std::string& getUserinfo() const;  // 用户信息
    const std::string& getHost() const;      // 主机
    int getPort() const;                     // 端口
    const std::string& getPath() const;      // 路径
    const std::string& getQuery() const;     // 查询
    const std::string& getFragment() const;  // 片段
    
    // 属性设置
    void setScheme(const std::string& scheme);
    void setUserinfo(const std::string& userinfo);
    void setHost(const std::string& host);
    void setPort(int port);
    void setPath(const std::string& path);
    void setQuery(const std::string& query);
    void setFragment(const std::string& fragment);
    
    // 辅助方法
    Address::Ptr createAddress() const;      // 创建地址
    std::string toString() const;            // 转字符串
    
    // 查询参数
    std::string getQueryParam(const std::string& key);
};
```

### 5.3 Uri 使用示例

```cpp
#include "net/uri.h"

void uri_example() {
    // 解析 URL
    lon::net::Uri::Ptr uri = lon::net::Uri::create(
        "http://user:pass@example.com:8080/path/to/resource?key1=value1&key2=value2#section"
    );
    
    if (!uri) {
        std::cerr << "URL 解析失败" << std::endl;
        return;
    }
    
    // 获取各部分
    std::cout << "协议: " << uri->getScheme() << std::endl;        // http
    std::cout << "用户: " << uri->getUserinfo() << std::endl;      // user:pass
    std::cout << "主机: " << uri->getHost() << std::endl;          // example.com
    std::cout << "端口: " << uri->getPort() << std::endl;          // 8080
    std::cout << "路径: " << uri->getPath() << std::endl;          // /path/to/resource
    std::cout << "查询: " << uri->getQuery() << std::endl;         // key1=value1&key2=value2
    std::cout << "片段: " << uri->getFragment() << std::endl;      // section
    
    // 获取查询参数
    std::cout << "key1: " << uri->getQueryParam("key1") << std::endl;  // value1
    std::cout << "key2: " << uri->getQueryParam("key2") << std::endl;  // value2
    
    // 创建地址
    lon::net::Address::Ptr addr = uri->createAddress();
    std::cout << "地址: " << addr->toString() << std::endl;
    
    // 构造 URL
    lon::net::Uri::Ptr new_uri = std::make_shared<lon::net::Uri>();
    new_uri->setScheme("https");
    new_uri->setHost("www.example.com");
    new_uri->setPort(443);
    new_uri->setPath("/api/data");
    new_uri->setQuery("id=123&name=test");
    
    std::cout << "构造 URL: " << new_uri->toString() << std::endl;
    // https://www.example.com:443/api/data?id=123&name=test
}
```

---

## 6. SSL Socket 安全套接字

### 6.1 SSLSocket 的作用

**SSLSocket = 支持 SSL/TLS 加密通信的 Socket**

```
┌─────────────────────────────────────────────────┐
│              SSLSocket                          │
│                                                 │
│  Socket（基础套接字）                            │
│       ↓                                         │
│  SSL/TLS 加密层                                 │
│       ↓                                         │
│  安全通信（HTTPS、WSS）                          │
└─────────────────────────────────────────────────┘
```

### 6.2 SSLSocket 使用示例

```cpp
#include "net/sslsocket.h"
#include "net/address.h"

void ssl_client() {
    // 解析地址
    lon::net::Address::Ptr addr;
    lon::net::Address::parse(addr, "www.example.com:443", AF_INET);
    
    // 创建 SSL Socket
    lon::net::SSLSocket::Ptr sock = lon::net::SSLSocket::create(addr);
    
    // 加载证书（可选，用于双向认证）
    sock->loadCertificates("client.crt", "client.key");
    
    // 连接
    if (!sock->connect(addr)) {
        std::cerr << "SSL 连接失败" << std::endl;
        return;
    }
    
    // 发送 HTTPS 请求
    std::string request = "GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
    sock->send(request.c_str(), request.size(), 0);
    
    // 接收响应
    char buf[4096];
    ssize_t n = sock->recv(buf, sizeof(buf), 0);
    
    std::cout << std::string(buf, n) << std::endl;
    
    sock->close();
}
```

---

## 7. 结合 IOScheduler 使用（实际应用）

### 7.1 HTTP 客户端示例

```cpp
#include "scheduler/ioscheduler.h"
#include "net/socket.h"
#include "net/address.h"
#include "hook/hook.h"

static auto g_logger = LON_LOG_ROOT;

void test_http_client()
{
    // 解析地址
    lon::net::Address::Ptr addr;
    lon::net::Address::parse(addr, "httpbin.org:80", AF_INET);
    
    LON_INFO(g_logger) << "addr=" << addr->toString();
    
    // 创建 Socket
    auto socket = lon::net::Socket::create(addr);
    
    // 连接（Hook 后自动协程化）
    auto ret = socket->connect(addr);
    if (!ret)
    {
        LON_ERROR(g_logger) << "connect failed: " << socket->toString();
        return;
    }
    
    // 发送 HTTP 请求
    auto request = std::make_shared<lon::http::HttpRequest>();
    request->setPath("/stream/3");
    request->setHeader("Host", "httpbin.org");
    
    LON_INFO(g_logger) << "request=" << request->toString();
    
    // 使用 HttpConnection 发送（见 httpservice 模块）
    auto connection = std::make_shared<lon::httpservice::HttpConnection>(socket, true);
    connection->sendRequest(request);
    
    // 接收响应
    auto response = connection->recvResponse();
    if (!response)
    {
        LON_ERROR(g_logger) << "recvResponse failed";
        return;
    }
    
    LON_INFO(g_logger) << "response=" << response->toString();
}

int main(int argc, char const *argv[])
{
    // 创建 IO 调度器
    auto ios = std::make_shared<lon::scheduler::IOScheduler>(2, true, "io_scheduler");
    
    // 添加任务
    ios->schedule(test_http_client);
    
    return 0;
}
```

---

## 8. 常见问题解答

### Q1: Address 和 Socket 有什么区别？

| 对比项 | Address | Socket |
|--------|---------|--------|
| 作用 | 地址管理 | 通信操作 |
| 使用 | 定位目标 | 发送接收数据 |
| 类比 | 电话号码 | 电话机 |

### Q2: TCP 和 UDP Socket 有什么区别？

| 对比项 | TCP | UDP |
|--------|-----|-----|
| 连接 | 需要 connect | 不需要 |
| 可靠性 | 可靠传输 | 不保证 |
| 顺序 | 保证顺序 | 可能乱序 |
| 适用场景 | HTTP、文件传输 | 视频、游戏 |

### Q3: SocketStream 和 Socket 有什么区别？

| 对比项 | Socket | SocketStream |
|--------|--------|--------------|
| 接口 | send/recv | read/write |
| 继承 | 不能继承 | 继承 Stream |
| 适用场景 | 直接网络操作 | 上层模块使用 |

### Q4: IPv4 和 IPv6 有什么区别？

| 对比项 | IPv4 | IPv6 |
|--------|------|------|
| 地址长度 | 32位 | 128位 |
| 地址数量 | 约 43 亿 | 无穷多 |
| 示例 | 192.168.1.1 | ::1 或 2001:db8::1 |

---

## 9. 总结

### 网络基础模块的本质

**网络基础模块 = 网络通信的"基础设施"**

- Address：地址管理
- Socket：通信操作
- SocketStream：流式接口
- Uri：URL 解析
- SSLSocket：安全通信

### 各模块的关系

```
Uri（URL 解析）
    ↓
Address（地址）
    ↓
Socket（套接字）
    ↓
SocketStream（流）
    ↓
HttpConnection / WebSocket（上层）
```

### 模块的优势

1. **统一接口**：Address 统一地址管理
2. **封装简单**：Socket 封装底层操作
3. **流式统一**：SocketStream 提供统一流接口
4. **URL 解析**：Uri 自动解析 URL
5. **安全通信**：SSLSocket 支持加密

### 配合其他模块

```
Address + Socket + Hook + IOScheduler
    ↓
实现高性能异步网络通信

Address + SocketStream + HttpConnection
    ↓
实现 HTTP 服务

Address + SSLSocket + HttpConnection
    ↓
实现 HTTPS 安全服务
```