#define _CRT_SECURE_NO_WARNINGS
# include "Windows.h"
# include "stdio.h"
# include "resource.h"
# include "PeOperator.h"


extern TCHAR szExeName[MAX_PATH];	//��Ҫ�ӿǵ�EXE �����ļ���
extern TCHAR szShellName[MAX_PATH];	//���ӳ���������ļ���
extern TCHAR szNewExeName[MAX_PATH];	//��Ҫ�ӿǺ��EXE �����ļ���


//���ӵ���Ϣ������
BOOL CALLBACK ShellProc(
  HWND hwndShell,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

//�ӿǺ���
DWORD CreateShell();