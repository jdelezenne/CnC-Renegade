#include "Platform/Network/Transport.h"
#include <cerrno>
#include <cstring>
namespace { thread_local int Error = ENETDOWN; int Fail() { Error = ENETDOWN; return SOCKET_ERROR; } }
int Platform::SocketLastError() { return Error; }
void Platform::SetSocketLastError(int error) { Error = error; }
int Platform::SocketWouldBlockError() { return EWOULDBLOCK; }
int Platform::SocketConnectionResetError() { return ECONNRESET; }
int Platform::SocketAddressInUseError() { return EADDRINUSE; }
int Platform::SocketSetNonblocking(SOCKET) { return Fail(); }
int Platform::SocketPendingBytes(SOCKET, int& bytes) { bytes = 0; return 0; }
int Platform::SocketClose(SOCKET) { return 0; }
int Platform::SocketShutdown(SOCKET, int) { return 0; }
int Platform::SocketBind(SOCKET, const SOCKADDR*, int) { return Fail(); }
int Platform::SocketGetOption(SOCKET, int level, int option, char* value, int* size) {
    if (value && size && *size > 0) std::memset(value, 0, static_cast<std::size_t>(*size));
    if (level == SOL_SOCKET && option == SO_ERROR) return 0;
    return Fail();
}
int Platform::SocketSetOption(SOCKET, int, int, const char*, int) { return Fail(); }
int Platform::SocketSendTo(SOCKET, const char*, int, int, const SOCKADDR*, int) { return Fail(); }
int Platform::SocketReceiveFrom(SOCKET, char*, int, int, SOCKADDR*, int*) { Error = EWOULDBLOCK; return SOCKET_ERROR; }
