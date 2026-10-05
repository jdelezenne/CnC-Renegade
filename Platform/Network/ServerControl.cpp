#include "Code/SControl/servercontrolsocket.h"
#include <cstring>
ServerControlSocketClass::ServerControlSocketClass() : Socket(INVALID_SOCKET), Port(0), StaticInBuffers{}, StaticOutBuffers{}, InBufferArrayPos(0), OutBufferArrayPos(0), InBuffersUsed(0), OutBuffersUsed(0), ReceiveBuffer{}, Key{} {}
ServerControlSocketClass::~ServerControlSocketClass() = default;
bool ServerControlSocketClass::Open(int, bool, unsigned long) { return false; }
void ServerControlSocketClass::Close() { Socket = INVALID_SOCKET; }
void ServerControlSocketClass::Discard_In_Buffers() {}
void ServerControlSocketClass::Discard_Out_Buffers() {}
void ServerControlSocketClass::Set_Encryption_Key(char* key) { std::memset(Key, 0, sizeof(Key)); if (key) std::strncpy(Key, key, 8); }
int ServerControlSocketClass::Peek(void*, int, void*, unsigned short* port, int) { if (port) *port = 0; return 0; }
int ServerControlSocketClass::Read(void*, int, void*, unsigned short* port, int) { if (port) *port = 0; return 0; }
void ServerControlSocketClass::Write(void*, int, void*, unsigned short) {}
void ServerControlSocketClass::Service() {}
void ServerControlSocketClass::Clear_Socket_Error() {}
void* ServerControlSocketClass::Get_New_Out_Buffer() { return nullptr; }
void* ServerControlSocketClass::Get_New_In_Buffer() { return nullptr; }
void ServerControlSocketClass::Add_CRC(unsigned long*, unsigned long) {}
void ServerControlSocketClass::Build_Packet_CRC(WinsockBufferType*) {}
bool ServerControlSocketClass::Passes_CRC_Check(WinsockBufferType*) { return false; }
void ServerControlSocketClass::Encrypt(unsigned char*, int) {}
void ServerControlSocketClass::Decrypt(unsigned char*, int) {}
