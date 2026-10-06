#include "Platform/Network/Transport.h"
#include <cstring>
#include <cstdio>
#include <iphlpapi.h>

std::string Platform::IPv4HostName(std::uint32_t address)
{
    const auto* host = gethostbyaddr(reinterpret_cast<const char*>(&address), sizeof(address), AF_INET);
    return host && host->h_name ? std::string(host->h_name) : std::string();
}

bool Platform::LocalTcpEndpoint(const char* hostname, std::uint16_t remotePort, std::uint32_t& address, std::uint16_t& port)
{
    address = 0;
    port = 0;
    if (!hostname || !*hostname) return false;
    WSADATA data;
    if (WSAStartup(MAKEWORD(1, 1), &data) != 0) return false;
    struct SocketSession {
        ~SocketSession() { WSACleanup(); }
    } session;
    const auto* host = gethostbyname(hostname);
    if (!host || host->h_addrtype != AF_INET || host->h_length != sizeof(std::uint32_t) || !host->h_addr_list[0]) return false;
    std::uint32_t remoteAddress;
    std::memcpy(&remoteAddress, host->h_addr_list[0], sizeof(remoteAddress));
    DWORD size = 0;
    if (GetTcpTable(nullptr, &size, FALSE) != ERROR_INSUFFICIENT_BUFFER) return false;
    std::vector<DWORD> storage;
    DWORD result;
    do {
        storage.resize((static_cast<std::size_t>(size) + sizeof(DWORD) - 1) / sizeof(DWORD));
        result = GetTcpTable(reinterpret_cast<MIB_TCPTABLE*>(storage.data()), &size, FALSE);
    } while (result == ERROR_INSUFFICIENT_BUFFER);
    if (result != NO_ERROR) return false;
    const auto* table = reinterpret_cast<const MIB_TCPTABLE*>(storage.data());
    for (DWORD i = 0; i < table->dwNumEntries; ++i) {
        const auto& row = table->table[i];
        if (row.dwState == MIB_TCP_STATE_ESTAB && row.dwRemoteAddr == remoteAddress &&
            (!remotePort || ntohs(static_cast<u_short>(row.dwRemotePort)) == remotePort)) {
            address = row.dwLocalAddr;
            port = ntohs(static_cast<u_short>(row.dwLocalPort));
            return true;
        }
    }
    return false;
}
bool Platform::LocalIPv4Addresses(std::vector<std::uint32_t>& addresses)
{
    addresses.clear();
    WSADATA data;
    const int startup = WSAStartup(MAKEWORD(1, 1), &data);
    if (startup != 0) {
        WSASetLastError(startup);
        return false;
    }
    struct SocketSession {
        ~SocketSession() { WSACleanup(); }
    } session;
    char hostname[300];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) return false;
    const auto* host = gethostbyname(hostname);
    if (!host || host->h_addrtype != AF_INET || host->h_length != sizeof(std::uint32_t)) return false;
    for (auto entry = host->h_addr_list; *entry; ++entry) {
        std::uint32_t address;
        std::memcpy(&address, *entry, sizeof(address));
        addresses.push_back(address);
    }
    return true;
}
int Platform::SocketCleanup() { return WSACleanup(); }
int Platform::SocketLastError() { return WSAGetLastError(); }
void Platform::SetSocketLastError(int error) { WSASetLastError(error); }
int Platform::SocketWouldBlockError() { return WSAEWOULDBLOCK; }
int Platform::SocketConnectionResetError() { return WSAECONNRESET; }
int Platform::SocketAddressInUseError() { return WSAEADDRINUSE; }
int Platform::SocketSetNonblocking(SOCKET socket) { unsigned long value = 1; return ioctlsocket(socket, FIONBIO, &value); }
int Platform::SocketPendingBytes(SOCKET socket, int& bytes) {
    unsigned long value = 0;
    const int result = ioctlsocket(socket, FIONREAD, &value);
    bytes = static_cast<int>(value);
    return result;
}
int Platform::SocketClose(SOCKET socket) { return closesocket(socket); }
int Platform::SocketShutdown(SOCKET socket, int how) { return shutdown(socket, how); }
int Platform::SocketBind(SOCKET socket, const SOCKADDR* address, int size) { return bind(socket, address, size); }
int Platform::SocketGetOption(SOCKET socket, int level, int option, char* value, int* size) { return getsockopt(socket, level, option, value, size); }
int Platform::SocketSetOption(SOCKET socket, int level, int option, const char* value, int size) { return setsockopt(socket, level, option, value, size); }
int Platform::SocketSendTo(SOCKET socket, const char* data, int size, int flags, const SOCKADDR* address, int addressSize) { return sendto(socket, data, size, flags, address, addressSize); }
int Platform::SocketReceiveFrom(SOCKET socket, char* data, int size, int flags, SOCKADDR* address, int* addressSize) { return recvfrom(socket, data, size, flags, address, addressSize); }

int Platform::SocketStartup() { WSADATA data; return WSAStartup(MAKEWORD(1, 1), &data); }
SOCKET Platform::SocketCreate(int family, int type, int protocol) { return ::socket(family, type, protocol); }
int Platform::SocketAddressFamilyError() { return WSAEAFNOSUPPORT; }
int Platform::SocketNoBufferError() { return WSAENOBUFS; }
#define ADD_CASE(exp) case exp: std::sprintf(error_msg, #exp); break;
std::string Platform::SocketErrorText(int error_code)
{
   char error_msg[500];

   switch (error_code) {

      ADD_CASE(WSAEINTR)
      ADD_CASE(WSAEBADF)
      ADD_CASE(WSAEACCES)
      ADD_CASE(WSAEFAULT)
      ADD_CASE(WSAEINVAL)
      ADD_CASE(WSAEMFILE)

      ADD_CASE(WSAEWOULDBLOCK)
      ADD_CASE(WSAEINPROGRESS)
      ADD_CASE(WSAEALREADY)
      ADD_CASE(WSAENOTSOCK)
      ADD_CASE(WSAEDESTADDRREQ)
      ADD_CASE(WSAEMSGSIZE)
      ADD_CASE(WSAEPROTOTYPE)
      ADD_CASE(WSAENOPROTOOPT)
      ADD_CASE(WSAEPROTONOSUPPORT)
      ADD_CASE(WSAESOCKTNOSUPPORT)
      ADD_CASE(WSAEOPNOTSUPP)
      ADD_CASE(WSAEPFNOSUPPORT)
      ADD_CASE(WSAEAFNOSUPPORT)
      ADD_CASE(WSAEADDRINUSE)
      ADD_CASE(WSAEADDRNOTAVAIL)
      ADD_CASE(WSAENETDOWN)
      ADD_CASE(WSAENETUNREACH)
      ADD_CASE(WSAENETRESET)
      ADD_CASE(WSAECONNABORTED)
      ADD_CASE(WSAECONNRESET)
      ADD_CASE(WSAENOBUFS)
      ADD_CASE(WSAEISCONN)
      ADD_CASE(WSAENOTCONN)
      ADD_CASE(WSAESHUTDOWN)
      ADD_CASE(WSAETOOMANYREFS)
      ADD_CASE(WSAETIMEDOUT)
      ADD_CASE(WSAECONNREFUSED)
      ADD_CASE(WSAELOOP)
      ADD_CASE(WSAENAMETOOLONG)
      ADD_CASE(WSAEHOSTDOWN)
      ADD_CASE(WSAEHOSTUNREACH)
      ADD_CASE(WSAENOTEMPTY)
      ADD_CASE(WSAEPROCLIM)
      ADD_CASE(WSAEUSERS)
      ADD_CASE(WSAEDQUOT)
      ADD_CASE(WSAESTALE)
      ADD_CASE(WSAEREMOTE)

      ADD_CASE(WSASYSNOTREADY)
      ADD_CASE(WSAVERNOTSUPPORTED)
      ADD_CASE(WSANOTINITIALISED)
		ADD_CASE(WSAEDISCON)

		default:
         ::sprintf(error_msg, "Unknown Winsock Error (%d)", error_code);
         break;
   }

	return(error_msg);
}
#undef ADD_CASE
