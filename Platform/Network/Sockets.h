#pragma once
#ifdef _WIN32
#include "Platform/Windows/TextTypes.h"
#include <winsock.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
using SOCKET = int;
using SOCKADDR_IN = sockaddr_in;
using SOCKADDR = sockaddr;
using LPSOCKADDR_IN = sockaddr_in*;
using LPSOCKADDR = sockaddr*;
inline constexpr SOCKET INVALID_SOCKET = -1;
inline constexpr int SOCKET_ERROR = -1;
#endif
