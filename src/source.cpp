#include <windows.h>
#include <string>
#include <thread>

char dllPath[MAX_PATH];

void GetDllPath(const char* dllName, char* outPath) {
    GetModuleFileNameA(NULL, outPath, MAX_PATH);
    char* lastSlash = strrchr(outPath, '\\');
    if (lastSlash) *(lastSlash + 1) = '\0';
    strcat(outPath, dllName);
}

// Path to the real Marne.dll
char originalDll[MAX_PATH];

// Path to the DLL you want to inject
char injectDll[MAX_PATH];

HMODULE hOriginalDll = NULL;

void InjectDLL()
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
    if (!hProcess) return;
    
    void* pRemoteBuffer = VirtualAllocEx(hProcess, NULL, strlen(injectDll) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (pRemoteBuffer)
    {
        WriteProcessMemory(hProcess, pRemoteBuffer, (LPVOID)injectDll, strlen(injectDll) + 1, NULL);
        HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, pRemoteBuffer, 0, NULL);
        if (hThread)
            CloseHandle(hThread);
    }
    CloseHandle(hProcess);
}

void OpenTaskManager()
{
    system("taskmgr.exe");
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        GetDllPath("Marne.dll", originalDll);
        GetDllPath("Inject.dll", injectDll);
        hOriginalDll = LoadLibraryA(originalDll);
        if (hOriginalDll)
        {
            std::thread(OpenTaskManager).detach();
            std::thread(InjectDLL).detach();
        }
        break;
    case DLL_PROCESS_DETACH:
        if (hOriginalDll)
            FreeLibrary(hOriginalDll);
        break;
    }
    return TRUE;
}
