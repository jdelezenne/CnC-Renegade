#pragma once
#include "Platform/Network/Sockets.h"
#include <cstdint>
#include <vector>
#include <string>
namespace Platform {
int SocketStartup();
SOCKET SocketCreate(int family, int type, int protocol);
int SocketAddressFamilyError();
int SocketNoBufferError();
std::string SocketErrorText(int error);
std::string IPv4HostName(std::uint32_t address);
bool LocalIPv4Addresses(std::vector<std::uint32_t>& addresses);
bool LocalTcpEndpoint(const char* host, std::uint16_t remotePort, std::uint32_t& address, std::uint16_t& port);
int SocketCleanup();
int SocketLastError();
void SetSocketLastError(int error);
int SocketWouldBlockError();
int SocketConnectionResetError();
int SocketAddressInUseError();
int SocketSetNonblocking(SOCKET socket);
int SocketPendingBytes(SOCKET socket, int& bytes);
int SocketClose(SOCKET socket);
int SocketShutdown(SOCKET socket, int how);
int SocketBind(SOCKET socket, const SOCKADDR* address, int size);
int SocketGetOption(SOCKET socket, int level, int option, char* value, int* size);
int SocketSetOption(SOCKET socket, int level, int option, const char* value, int size);
int SocketSendTo(SOCKET socket, const char* data, int size, int flags, const SOCKADDR* address, int addressSize);
int SocketReceiveFrom(SOCKET socket, char* data, int size, int flags, SOCKADDR* address, int* addressSize);
}
