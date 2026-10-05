#include "Platform/Network/Transport.h"
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
