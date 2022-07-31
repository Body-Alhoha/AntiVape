#include <Windows.h>
#include "JNI/jni.h"
#include "hook/MinHook.h"
#include <thread>

typedef jint (JNICALL * getCreatedJavaVMS)(JavaVM **, jsize, jsize *);
getCreatedJavaVMS real = NULL;
jint WINAPI GetCreatedJavaVMs(JavaVM** vm, jsize s, jsize* ss)
{
    return 0;
}

void init() {
    HMODULE jvm = GetModuleHandleA("jvm.dll");
    if (jvm == nullptr || MH_Initialize() != MH_OK)
        return;
    FARPROC get_created_java_vms = GetProcAddress(jvm, "JNI_GetCreatedJavaVMs");
    getCreatedJavaVMS temp = (getCreatedJavaVMS)reinterpret_cast<LPVOID>((get_created_java_vms));
    MH_CreateHook(temp, &GetCreatedJavaVMs, (void**)&real);
    MH_EnableHook(temp);
}


BOOL APIENTRY DllMain(HMODULE dll_module, DWORD reason_for_call,LPVOID lpReserved)
{
    if(reason_for_call == DLL_PROCESS_ATTACH)
        std::thread(init).detach();
    return TRUE;
}

