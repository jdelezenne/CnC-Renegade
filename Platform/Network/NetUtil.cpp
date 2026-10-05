#include "Code/wwnet/netutil.h"
#include "Platform/Network/Transport.h"
#include <cstdio>
#include <cstring>
UINT cNetUtil::DefaultResendTimeoutMs = 200;
bool cNetUtil::IsInternet = false;
char cNetUtil::WorkingAddressBuffer[300]{};
int g_c_wouldblock = 0;
int g_c_nobufs = 0;
const USHORT cNetUtil::NETSTATS_SAMPLE_TIME_MS = 2000;
const USHORT cNetUtil::KEEPALIVE_TIMEOUT_MS = 2000;
const USHORT cNetUtil::MAX_RESENDS = 50;
const USHORT cNetUtil::MULTI_SENDS = 10;
const USHORT cNetUtil::RESEND_TIMEOUT_LAN_MS = 300;
const USHORT cNetUtil::RESEND_TIMEOUT_INTERNET_MS = 500;
const ULONG cNetUtil::CLIENT_CONNECTION_LOSS_TIMEOUT = 15000;
const ULONG cNetUtil::SERVER_CONNECTION_LOSS_TIMEOUT = 15000;
const ULONG cNetUtil::SERVER_CONNECTION_LOSS_TIMEOUT_LOADING_ALLOWANCE = 45000;
void cNetUtil::Wsa_Init() {}
bool cNetUtil::Protocol_Init(bool) { IsInternet = false; return false; }
bool cNetUtil::Is_Tcpip_Present() { return false; }
void cNetUtil::Wsa_Error(LPCSTR, unsigned) {}
const char* cNetUtil::Winsock_Error_Text(int) { return "Networking unavailable"; }
bool cNetUtil::Would_Block(LPCSTR, unsigned, int result) { return result == SOCKET_ERROR && Platform::SocketLastError() == Platform::SocketWouldBlockError(); }
bool cNetUtil::Send_Resource_Failure(LPCSTR, unsigned, int) { return false; }
bool cNetUtil::Get_Local_Address(LPSOCKADDR_IN address) { if (address) *address = {}; return false; }
int cNetUtil::Get_Local_Tcpip_Addresses(SOCKADDR_IN*, USHORT) { return 0; }
bool cNetUtil::Is_Same_Address(LPSOCKADDR_IN first, const SOCKADDR_IN* second) { return first && second && first->sin_addr.s_addr == second->sin_addr.s_addr && first->sin_port == second->sin_port; }
void cNetUtil::Address_To_String(LPSOCKADDR_IN address, char* buffer, UINT size, USHORT& port) {
    port = address ? ntohs(address->sin_port) : 0;
    if (!buffer || !size) return;
    if (!address || !inet_ntop(AF_INET, &address->sin_addr, buffer, size)) buffer[0] = 0;
}
LPCSTR cNetUtil::Address_To_String(ULONG address) {
    const in_addr value{address};
    if (!inet_ntop(AF_INET, &value, WorkingAddressBuffer, sizeof(WorkingAddressBuffer))) WorkingAddressBuffer[0] = 0;
    return WorkingAddressBuffer;
}
void cNetUtil::String_To_Address(LPSOCKADDR_IN address, LPCSTR text, USHORT port) {
    if (!address) return;
    *address = {}; address->sin_family = AF_INET; address->sin_port = htons(port);
    if (text) inet_pton(AF_INET, text, &address->sin_addr);
}
void cNetUtil::Create_Unbound_Socket(SOCKET& socket) { socket = INVALID_SOCKET; }
bool cNetUtil::Create_Bound_Socket(SOCKET& socket, USHORT, SOCKADDR_IN& address) { socket = INVALID_SOCKET; address = {}; return false; }
void cNetUtil::Close_Socket(SOCKET& socket) { socket = INVALID_SOCKET; }
void cNetUtil::Create_Broadcast_Address(LPSOCKADDR_IN address, USHORT port) { if (address) { *address = {}; address->sin_family = AF_INET; address->sin_port = htons(port); address->sin_addr.s_addr = INADDR_BROADCAST; } }
void cNetUtil::Create_Local_Address(LPSOCKADDR_IN address, USHORT port) { if (address) { *address = {}; address->sin_family = AF_INET; address->sin_port = htons(port); } }
void cNetUtil::Set_Socket_Buffer_Sizes(SOCKET, int) {}
void cNetUtil::Broadcast(SOCKET&, USHORT, cPacket&) {}
void cNetUtil::Lan_Servicing(SOCKET&, LanPacketHandlerCallback) {}
