#include "Code/Commando/natsock.h"
DynamicVectorClass<SocketHandlerClass*> SocketHandlerClass::AllSocketHandlers;
SocketHandlerClass::SocketHandlerClass() : Socket(INVALID_SOCKET), IncomingPort(0), OutgoingPort(0), StaticInBuffers{}, StaticOutBuffers{}, InBufferArrayPos(0), OutBufferArrayPos(0), InBuffersUsed(0), OutBuffersUsed(0), ReceiveBuffer{}, CanService(false) {}
SocketHandlerClass::~SocketHandlerClass() = default;
bool SocketHandlerClass::Open(int, int) { return false; }
void SocketHandlerClass::Close() { Socket = INVALID_SOCKET; }
void SocketHandlerClass::Discard_In_Buffers() {}
void SocketHandlerClass::Discard_Out_Buffers() {}
int SocketHandlerClass::Peek(void*, int, void*, unsigned short* port, int) { if (port) *port = 0; return 0; }
int SocketHandlerClass::Read(void*, int, void*, unsigned short* port, int) { if (port) *port = 0; return 0; }
void SocketHandlerClass::Write(void*, int, void*, unsigned short) {}
void SocketHandlerClass::Service() {}
void SocketHandlerClass::Service_All() {}
void SocketHandlerClass::Clear_Socket_Error() {}
void* SocketHandlerClass::Get_New_Out_Buffer() { return nullptr; }
void* SocketHandlerClass::Get_New_In_Buffer() { return nullptr; }
void SocketHandlerClass::Add_CRC(unsigned long*, unsigned long) {}
void SocketHandlerClass::Build_Packet_CRC(WinsockBufferType*) {}
bool SocketHandlerClass::Passes_CRC_Check(WinsockBufferType*) { return false; }
