#include <stdio.h>
#include <vector>
#include <windows.h>
#include <string>

#include <debugapi.h>
#include <tchar.h>
#include <strsafe.h>

#include "DebugFile.h"
#include "DebuggerCmd.h"


void onDebugStringEvent(
	const DEBUG_EVENT event)
{
	// open process
	HANDLE process = OpenProcess(PROCESS_VM_READ, FALSE, event.dwProcessId);

	TCHAR buff[1024];

	size_t countReaded = 0;

	BOOL readProc = ReadProcessMemory(
		process,
		event.u.DebugString.lpDebugStringData,
		buff,
		event.u.DebugString.nDebugStringLength,
		&countReaded);

	int buff2[1];

	//for (int i = 1; i < 10; i++)
	//{
	//	//printf("onDebugStringEvent: read %p\n", event.u.DebugString.lpDebugStringData + sizeof(int) * i);
	//	ReadProcessMemory(process, (void*)0x000000B2D56FFCE0, buff2, sizeof(int), 0);
	//	//0000004D99B3FB10
	//}

	if (!readProc)
	{
		printf("onDebugStringEvent: cant ReadProcessMemory -> false");
		return;
	}

	_tprintf(TEXT("debug data: %p, %s\n"), event.u.DebugString.lpDebugStringData, buff);
}

void onProcessDebugEvent(DEBUG_EVENT event)
{
	TCHAR fileName[128];
	BOOL fileNameGet = DebugFile::GetFileNameFromHandle(event.u.CreateProcessInfo.hFile, fileName);

	_tprintf(TEXT("debug caller: %s\n"), fileName);

	int t = 3;
}


void onExceptionDebugEvent(DEBUG_EVENT event)
{
	EXCEPTION_RECORD exp = event.u.Exception.ExceptionRecord;
	
	switch (exp.ExceptionCode)
	{
	case EXCEPTION_BREAKPOINT:
		printf("exp: EXCEPTION_BREAKPOINT\n");
		break;

	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		printf("exp: EXCEPTION_ARRAY_BOUNDS_EXCEEDED\n");
		break;
	}

}

void main() {

	DebuggerCmd cmd;


	STARTUPINFO si;
	PROCESS_INFORMATION pi;


	LPCWSTR processName = L"C:\\ghrepos\\low\\debugger\\src\\x64\\Release\\debugApp.exe";

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(
		processName,
		NULL,
		NULL,
		NULL,
		FALSE,
		DEBUG_PROCESS + \
		DEBUG_ONLY_THIS_PROCESS,
		NULL,
		NULL,
		&si,
		&pi))
	{
		printf("create proccess failed\n");
		return;
	}

	//WaitForSingleObject(pi.hProcess, INFINITE);

	DEBUG_EVENT event{ 0 };

	while (true)
	{
		if (WaitForDebugEventEx(&event, INFINITE))
		{
			switch (event.dwDebugEventCode)
			{
			case CREATE_PROCESS_DEBUG_EVENT:
				printf("CREATE_PROCESS_DEBUG_EVENT\n");
				onProcessDebugEvent(event);
				break;

			case EXCEPTION_DEBUG_EVENT:
				onExceptionDebugEvent(event);
				break;

			case OUTPUT_DEBUG_STRING_EVENT:
				printf("OUTPUT_DEBUG_STRING_EVENT\n");
				onDebugStringEvent(event);
				break;

			case LOAD_DLL_DEBUG_EVENT:
				printf("LOAD_DLL_DEBUG_EVENT\n");
				break;

				//LOAD_DLL_DEBUG_EVENT

			default:
				printf("eventCode=%i\n", event.dwDebugEventCode);
				break;
			}

			DWORD command = cmd.ReadNextCommand();

			printf("cmd=%i\n", command);

			ContinueDebugEvent(
				event.dwProcessId,
				event.dwThreadId,
				DBG_CONTINUE);
		}
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);





	//WaitForDebugEvent(,)
	return;
}