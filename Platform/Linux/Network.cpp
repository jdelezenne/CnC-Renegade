#include "Platform/Network/Transport.h"
#include <algorithm>
#include <cerrno>
#include <charconv>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <ifaddrs.h>
#include <net/if.h>
#include <sstream>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
thread_local int SocketError = 0;
template<class T> T SocketResult(T result)
{
    if (result < 0) SocketError = errno;
    return result;
}
int InvalidArgument() { errno = EINVAL; return SocketResult(SOCKET_ERROR); }
void NameLookupError(int result)
{
    SocketError = result == EAI_SYSTEM ? errno : result == EAI_AGAIN ? EAGAIN :
        result == EAI_MEMORY ? ENOMEM : result == EAI_FAMILY ? EAFNOSUPPORT : EHOSTUNREACH;
}
bool TcpAddress(const std::string& text, std::uint32_t& address, std::uint16_t& port)
{
    const auto colon = text.find(':');
    if (colon == std::string::npos) return false;
    std::uint32_t value = 0;
    const auto ip = std::from_chars(text.data(), text.data() + colon, address, 16);
    const auto number = std::from_chars(text.data() + colon + 1, text.data() + text.size(), value, 16);
    if (ip.ec != std::errc{} || ip.ptr != text.data() + colon ||
        number.ec != std::errc{} || number.ptr != text.data() + text.size() || value > 65535) return false;
    port = static_cast<std::uint16_t>(value);
    return true;
}
}

int Platform::SocketStartup() { return 0; }
SOCKET Platform::SocketCreate(int family, int type, int protocol) { return SocketResult(::socket(family, type | SOCK_CLOEXEC, protocol)); }
int Platform::SocketAddressFamilyError() { return EAFNOSUPPORT; }
int Platform::SocketNoBufferError() { return ENOBUFS; }
std::string Platform::SocketErrorText(int error) { return std::strerror(error); }
int Platform::SocketCleanup() { return 0; }
int Platform::SocketLastError() { return SocketError; }
void Platform::SetSocketLastError(int error) { SocketError = error; }
int Platform::SocketWouldBlockError() { return EWOULDBLOCK; }
int Platform::SocketConnectionResetError() { return ECONNRESET; }
int Platform::SocketAddressInUseError() { return EADDRINUSE; }
int Platform::SocketSetNonblocking(SOCKET socket)
{
    const int flags = fcntl(socket, F_GETFL);
    return SocketResult(flags < 0 ? SOCKET_ERROR : fcntl(socket, F_SETFL, flags | O_NONBLOCK));
}
int Platform::SocketPendingBytes(SOCKET socket, int& bytes)
{
    bytes = 0;
    return SocketResult(ioctl(socket, FIONREAD, &bytes));
}
int Platform::SocketClose(SOCKET socket) { return SocketResult(::close(socket)); }
int Platform::SocketShutdown(SOCKET socket, int how) { return SocketResult(::shutdown(socket, how)); }
int Platform::SocketBind(SOCKET socket, const SOCKADDR* address, int size)
{
    return size < 0 ? InvalidArgument() : SocketResult(::bind(socket, address, static_cast<socklen_t>(size)));
}
int Platform::SocketGetOption(SOCKET socket, int level, int option, char* value, int* size)
{
    if (!size || *size < 0) return InvalidArgument();
    socklen_t length = static_cast<socklen_t>(*size);
    const int result = getsockopt(socket, level, option, value, &length);
    if (result == 0) *size = static_cast<int>(length);
    return SocketResult(result);
}
int Platform::SocketSetOption(SOCKET socket, int level, int option, const char* value, int size)
{
    return size < 0 ? InvalidArgument() : SocketResult(setsockopt(socket, level, option, value, static_cast<socklen_t>(size)));
}
int Platform::SocketSendTo(SOCKET socket, const char* data, int size, int flags, const SOCKADDR* address, int addressSize)
{
    if (size < 0 || addressSize < 0) return InvalidArgument();
    ssize_t result;
    do {
        result = sendto(socket, data, static_cast<std::size_t>(size), flags | MSG_NOSIGNAL, address, static_cast<socklen_t>(addressSize));
    } while (result < 0 && errno == EINTR);
    return SocketResult(static_cast<int>(result));
}
int Platform::SocketReceiveFrom(SOCKET socket, char* data, int size, int flags, SOCKADDR* address, int* addressSize)
{
    if (size < 0 || (address && !addressSize) || (addressSize && *addressSize < 0)) return InvalidArgument();
    iovec buffer{data, static_cast<std::size_t>(size)};
    msghdr message{};
    message.msg_name = address;
    message.msg_namelen = addressSize ? static_cast<socklen_t>(*addressSize) : 0;
    message.msg_iov = &buffer;
    message.msg_iovlen = 1;
    ssize_t result;
    do {
        result = recvmsg(socket, &message, flags);
    } while (result < 0 && errno == EINTR);
    if (result >= 0 && addressSize) *addressSize = static_cast<int>(message.msg_namelen);
    if (result >= 0 && (message.msg_flags & MSG_TRUNC)) {
        errno = EMSGSIZE;
        return SocketResult(SOCKET_ERROR);
    }
    return SocketResult(static_cast<int>(result));
}

std::string Platform::IPv4HostName(std::uint32_t address)
{
    sockaddr_in socket{};
    socket.sin_family = AF_INET;
    socket.sin_addr.s_addr = address;
    char name[NI_MAXHOST];
    const int result = getnameinfo(reinterpret_cast<const sockaddr*>(&socket), sizeof(socket), name, sizeof(name), nullptr, 0, NI_NAMEREQD);
    if (result) { NameLookupError(result); return {}; }
    return name;
}
bool Platform::LocalIPv4Addresses(std::vector<std::uint32_t>& addresses)
{
    addresses.clear();
    ifaddrs* list = nullptr;
    if (getifaddrs(&list)) { SocketResult(SOCKET_ERROR); return false; }
    struct Interfaces {
        ifaddrs* List;
        ~Interfaces() { freeifaddrs(List); }
    } interfaces{list};
    std::vector<std::uint32_t> loopback;
    for (auto* item = list; item; item = item->ifa_next) {
        if (!item->ifa_addr || item->ifa_addr->sa_family != AF_INET || !(item->ifa_flags & IFF_UP)) continue;
        const auto address = reinterpret_cast<const sockaddr_in*>(item->ifa_addr)->sin_addr.s_addr;
        if (address == INADDR_ANY) continue;
        auto& output = (item->ifa_flags & IFF_LOOPBACK) ? loopback : addresses;
        if (std::find(output.begin(), output.end(), address) == output.end()) output.push_back(address);
    }
    if (addresses.empty()) addresses = std::move(loopback);
    return true;
}
bool Platform::LocalTcpEndpoint(const char* host, std::uint16_t remotePort, std::uint32_t& address, std::uint16_t& port)
{
    address = 0;
    port = 0;
    if (!host || !*host) return false;
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* list = nullptr;
    const int result = getaddrinfo(host, nullptr, &hints, &list);
    if (result) { NameLookupError(result); return false; }
    struct Addresses {
        addrinfo* List;
        ~Addresses() { freeaddrinfo(List); }
    } resolved{list};
    std::ifstream table("/proc/net/tcp");
    std::string line;
    std::getline(table, line);
    while (std::getline(table, line)) {
        std::istringstream row(line);
        std::string index, local, remote, state;
        if (!(row >> index >> local >> remote >> state) || state != "01") continue;
        std::uint32_t remoteAddress = 0, localAddress = 0;
        std::uint16_t peerPort = 0, localPort = 0;
        if (!TcpAddress(remote, remoteAddress, peerPort) || (remotePort && peerPort != remotePort) ||
            !TcpAddress(local, localAddress, localPort)) continue;
        for (auto* item = list; item; item = item->ai_next) {
            if (reinterpret_cast<const sockaddr_in*>(item->ai_addr)->sin_addr.s_addr == remoteAddress) {
                address = localAddress;
                port = localPort;
                return true;
            }
        }
    }
    return false;
}
