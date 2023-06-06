#define _CRT_SECURE_NO_WARNINGS
# include "Windows.h"
# include "stdio.h"
# include "resource.h"
# include "PeOperator.h"


extern TCHAR szExeName[MAX_PATH];	//需要加壳的EXE 完整文件名
extern TCHAR szShellName[MAX_PATH];	//壳子程序的完整文件名
extern TCHAR szNewExeName[MAX_PATH];	//需要加壳后的EXE 完整文件名


//壳子的消息处理函数
BOOL CALLBACK ShellProc(
  HWND hwndShell,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

//加壳函数
DWORD CreateShell();