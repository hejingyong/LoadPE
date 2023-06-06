#define _CRT_SECURE_NO_WARNINGS
#include "windows.h"
#include "resource.h"
#include "stdio.h"
#include <Tlhelp32.h>
#include "PeOperator.h"

extern TCHAR szProcessID[MAX_PATH];
extern TCHAR szPePath[MAX_PATH];


BOOL CALLBACK InjectProc(
  HWND hwndInject,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);


//远程线程注入
DWORD RemoteThreadInjectDll(HWND hwndInject);
DWORD RemoteThreadUninstallDll(HWND hwndInject);


//
extern LPVOID NewBase;		//新的基址
extern DWORD dwImageSize;	    //保存ImageBase
//隐藏模块注入	通过远程线程  在内存写入
DWORD HiddenModuleInjectDll_RemoteThread(HWND hwndInject);
DWORD HiddenModuleUninstallDll(HWND hwndInject);
VOID WINAPI EntryProc();//注入后执行的代码


//隐藏模块注入	通过挂起线程  在内存写入
//通过进程ID获取线程ID
DWORD GetThreadByPorcessID(DWORD dwObjPID, PDWORD pObjThreadID);
DWORD HiddenModuleInjectDll_SuspendThread(HWND hwndInject);
//VOID WINAPI EntryProc();//注入后执行的代码



//加载进程		将可执行文件加载到内存并运行
DWORD LoadProcessInject(HWND hwndInject);
