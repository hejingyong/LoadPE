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


//Զ���߳�ע��
DWORD RemoteThreadInjectDll(HWND hwndInject);
DWORD RemoteThreadUninstallDll(HWND hwndInject);


//
extern LPVOID NewBase;		//�µĻ�ַ
extern DWORD dwImageSize;	    //����ImageBase
//����ģ��ע��	ͨ��Զ���߳�  ���ڴ�д��
DWORD HiddenModuleInjectDll_RemoteThread(HWND hwndInject);
DWORD HiddenModuleUninstallDll(HWND hwndInject);
VOID WINAPI EntryProc();//ע���ִ�еĴ���


//����ģ��ע��	ͨ�������߳�  ���ڴ�д��
//ͨ������ID��ȡ�߳�ID
DWORD GetThreadByPorcessID(DWORD dwObjPID, PDWORD pObjThreadID);
DWORD HiddenModuleInjectDll_SuspendThread(HWND hwndInject);
//VOID WINAPI EntryProc();//ע���ִ�еĴ���



//���ؽ���		����ִ���ļ����ص��ڴ沢����
DWORD LoadProcessInject(HWND hwndInject);
