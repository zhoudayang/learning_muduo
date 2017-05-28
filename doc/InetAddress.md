# InetAddress

#### 实现分析

这个类可以视为```sockaddr_in```以及```sockaddr_in6```结构的简单wrapper。虽然代码比较简单明了，但是其内部机理可不太简单。

我们知道，对于双栈ipv6服务器，它也支持ipv4连接，内核会对ipv4连接的地址结构进行处理，转换为ipv6兼容的```sockaddr_in6```结构，这样工作在ipv6模式上的服务器也能够接受ipv4连接。作为一个网络库，我们需要保证兼容性，如何处理好ipv4和ipv6的兼容问题，值得深思。

这里使用了union来存放地址信息，定义如下：
```
union
  {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
```
使用这一方式，需要保证```addr_```与```addr6_```重合的部分内存布局是一样的。在这里，使用了```static_assert```进行断言。
```
static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
              "InetAddress is same size as sockaddr_in6");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");
```

因为在union内部的协议版本字段标志了地址对应的版本，将其转换为```sockaddr*``` 作为connect的输入时能够保证正常运行。并且通过getLocalAddr以及getSockAddr返回的地址信息使用InetAddress能够保证有足够的空间进行存储。总而言之，这一设定堪称完美。

#### 线程安全的dns解析
为了实现线程安全的dns解析，InetAddress中做了以下处理，使用了线程局部变量。
```
static __thread char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(StringArg hostname, InetAddress* out)
{
  assert(out != NULL);
  struct hostent hent;
  struct hostent* he = NULL;
  int herrno = 0;
  bzero(&hent, sizeof(hent));

  int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
  if (ret == 0 && he != NULL)
  {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
    out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
    return true;
  }
  else
  {
    if (ret)
    {
      LOG_SYSERR << "InetAddress::resolve";
    }
    return false;
  }
}
```
