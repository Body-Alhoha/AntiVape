// dllmain.cpp : Définit le point d'entrée de l'application DLL.
#include <Windows.h>
#include "JNI/jni.h"
#include "JNI/jvmti.h"
#include <iostream>
#include "md5.h"
#include "hook/MinHook.h"
JavaVM* jvm;
JNIEnv* jenv;
jvmtiEnv* jvmti;
jvmtiError error;

unsigned char ripvape[] = {
    6,8,9,5,8,9,8,8,8,9,8,9,8,5,6,5,6,5,7
};

static void JNICALL ClassFileLoadHook(jvmtiEnv* jvmti, JNIEnv* jni, jclass class_being_redefined, jobject loader, const char* name, jobject protection_domain, jint class_data_len, const unsigned char* class_data, jint* new_class_data_len, unsigned char** new_class_data);

FILE* pFile = nullptr;

typedef jint (JNICALL * getCreatedJavaVMS)(JavaVM **, jsize, jsize *);
getCreatedJavaVMS real = NULL;
jint WINAPI GetCreatedJavaVMs(JavaVM** vm, jsize s, jsize* ss)
{
    std::cout << "Vape injection prevented" << std::endl;
    return 0;
    //return real(vm, s, ss);
}

void HookCreatedJavaVMS() {
    HMODULE jvmDll = GetModuleHandleA("jvm.dll");
    FARPROC javavms = GetProcAddress(jvmDll, "JNI_GetCreatedJavaVMs");
    printf("Address of JNI_GetCreatedJavaVMs :%p\n", javavms);
    if (MH_Initialize() != MH_OK)
    {
        std::cout << "failed to initialize MinHok" << std::endl;
        return;
    }
    std::cout << "MinHook initalized" << std::endl;
    getCreatedJavaVMS temp = (getCreatedJavaVMS)reinterpret_cast<LPVOID>((javavms));
    MH_CreateHook(temp, &GetCreatedJavaVMs, (void**)&real);
    MH_EnableHook(temp);
    std::cout << "Hook done" << std::endl;

   
    
}



void init() {
    AllocConsole();
    freopen_s(&pFile, "CONOUT$", "w", stdout);
    freopen_s(&pFile, "CONIN$", "r", stdin);
   
    jsize vmCount;
    if (JNI_GetCreatedJavaVMs(&jvm, 1, &vmCount) != JNI_OK || vmCount == 0) {
        std::cout << "JVM not found" << std::endl;
        return;
    }
    if (jvm->GetEnv((void**)&jenv, JNI_VERSION_1_8) == JNI_EDETACHED) {
        jvm->AttachCurrentThread((void**)&jenv, NULL);
    }
    else {
        std::cout << "Jenv not found" << std::endl;
        return;
    }
    if (jenv == nullptr) {
        std::cout << "Jenv = nullptr" << std::endl;
        return;
    }
    jint res = jvm->GetEnv((void**)&jvmti, JVMTI_VERSION_1);
    if (res != JNI_OK || jvmti == NULL) {
        std::cout << "Failed to get jvmti ver 1" << std::endl;
        return;
    }

    std::cout << "Loaded successfuly, Hooking..." << std::endl;

    HookCreatedJavaVMS();

    //incase it bypasses attach current thread

    static jvmtiCapabilities capa;
    (void)memset(&capa, 0, sizeof(jvmtiCapabilities));
    capa.can_retransform_classes = 1;
    jvmti->AddCapabilities(&capa);
    jvmtiEventCallbacks callbacks = { 0 };
    callbacks.ClassFileLoadHook = ClassFileLoadHook;

    jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));
    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE,
        JVMTI_EVENT_CLASS_FILE_LOAD_HOOK,
        (jthread)NULL);
    
   

}
unsigned char* antivape = NULL;
static void JNICALL
ClassFileLoadHook(jvmtiEnv* jvmti, JNIEnv* jni, jclass class_being_redefined, jobject loader, const char* name, jobject protection_domain, jint class_data_len, const unsigned char* class_data, jint* new_class_data_len, unsigned char** new_class_data) {
    if (name != NULL) {
        std::string sName(name);
        if (sName.starts_with("a/")) {
            std::cout << "vape v4 injection prevented" << std::endl;
            ZeroMemory(antivape, sizeof(antivape));
            jvmti->Allocate(((jlong)(sizeof(ripvape))), &antivape);
            memcpy(antivape, ripvape, (int)sizeof(ripvape));
            *new_class_data_len = ((jint)(sizeof(ripvape)));
            *new_class_data = ripvape;
                     
        }
    }
    
   
   

}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)init, 0, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

