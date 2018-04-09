// fsmiddle_DTU.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <conio.h>
#include <ctype.h>

extern void test_log(TCHAR *path);
extern bool init_dtu_enviroment(TCHAR *path);
extern void FSMID_Task(void*);
char exeFullPath[256];

extern DWORD WINAPI threadConsole(LPVOID lpParameter);

int _tmain(int argc, _TCHAR* argv[])
{
	char *p;
	GetModuleFileName(NULL,exeFullPath,MAX_PATH);
	p = exeFullPath + strlen(exeFullPath) - 1;
	while(*p != '\\' && p != exeFullPath)
		p--;
	*p = '\0';
#if 0
	test_log(exeFullPath);
#else
	HANDLE t1 = CreateThread(NULL, 0, threadConsole, exeFullPath, 0, NULL); 
	Sleep(100);
	FSMID_Task(NULL);
	WaitForSingleObject(t1,INFINITE);
#endif
	_getch();
	return 0;
}

