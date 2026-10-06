#include "singletoninstancekeeper.h"
#include "Platform/Application.h"
#include "autostart.h"

const char* SingletonInstanceKeeperClass::APP_GUID = "C6D925A3-7A9B-4ca3-866D-8B4D506C3665";
bool SingletonInstanceKeeperClass::AllowMultipleInstances = false;

SingletonInstanceKeeperClass::SingletonInstanceKeeperClass() : AppMutex(nullptr)
{
    if (AutoRestart.Get_Restart_Flag()) AllowMultipleInstances = true;
}

SingletonInstanceKeeperClass::~SingletonInstanceKeeperClass()
{
    Platform::ReleaseApplicationInstance(AppMutex);
}

bool SingletonInstanceKeeperClass::Verify_Safe_To_Execute()
{
    if (!AppMutex) AppMutex = Platform::AcquireApplicationInstance(APP_GUID, AllowMultipleInstances);
    return AppMutex != nullptr;
}

void SingletonInstanceKeeperClass::Allow_Multiple_Instances(bool flag)
{
    AllowMultipleInstances = flag;
}
