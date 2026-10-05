#include <cstddef>
#include "Code/BandTest/BandTest.h"
unsigned long Detect_Bandwidth(unsigned long, unsigned long, int, int& failure, unsigned long& downstream, unsigned long, BandtestSettingsStruct*, char*) { failure = BANDTEST_NO_WINSOCK2; downstream = 0; return 0; }
