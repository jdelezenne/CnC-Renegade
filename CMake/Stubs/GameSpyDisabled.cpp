// Disabled GameSpy facade: retain the game interface without the SDK.
#include "GameSpy_QnR.h"

const char *CGameSpyQnR::gamename = "ccrenegade";
const char *CGameSpyQnR::bname = "SDK disabled";
const int CGameSpyQnR::prodid = 0;
const int CGameSpyQnR::cdkey_id = 0;
const char *CGameSpyQnR::default_heartbeat_list = "";
CGameSpyQnR GameSpyQnR;

CGameSpyQnR::CGameSpyQnR() : m_GSInit(false), m_GSEnabled(false),
    query_reporting_rec(0), StartTime(0) { secret_key[0] = 0; }
CGameSpyQnR::~CGameSpyQnR() {}
void CGameSpyQnR::Init() {}
void CGameSpyQnR::Shutdown() { m_GSInit = false; m_GSEnabled = false; }
void CGameSpyQnR::Think() {}
void CGameSpyQnR::LaunchArcade() {}
void CGameSpyQnR::TrackUsage() {}
bool CGameSpyQnR::Parse_HeartBeat_List(const char *) { return false; }
void CGameSpyQnR::DoGameStuff() {}
bool CGameSpyQnR::Append_InfoKey_Pair(char *, int, const char *, const char *) { return false; }
bool CGameSpyQnR::Append_InfoKey_Pair(char *, int, const char *, const StringClass &) { return false; }
bool CGameSpyQnR::Append_InfoKey_Pair(char *, int, const char *, const WideStringClass &) { return false; }
void CGameSpyQnR::basic_callback(char *out, int size) { if(out && size > 0) out[0] = 0; }
void CGameSpyQnR::info_callback(char *out, int size) { if(out && size > 0) out[0] = 0; }
void CGameSpyQnR::rules_callback(char *out, int size) { if(out && size > 0) out[0] = 0; }
void CGameSpyQnR::players_callback(char *out, int size) { if(out && size > 0) out[0] = 0; }
