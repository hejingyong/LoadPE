#include "Inject.h"

BOOL CALLBACK InjectProc(
  HWND hwndInject,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_Inject:
				//�жϵ�ѡ���Ƿ�ѡ��  Զ���߳�
				if( SendMessage(GetDlgItem(hwndInject, IDC_RADIO_RemoteThread), BM_GETCHECK, 0, 0) == BST_CHECKED )
				{
					if(RemoteThreadInjectDll(hwndInject) != 0)
					{
						MessageBox(hwndInject, TEXT("ע��ʧ�ܣ�"), TEXT("Error"), MB_OK);
					}
					else
					{
						MessageBox(hwndInject, TEXT("ע��ɹ���"), TEXT("Success"), MB_OK);
					}
				}//����ģ��ע��
				else if( SendMessage(GetDlgItem(hwndInject, IDC_RADIO_HiddenModule_RemoteThread), BM_GETCHECK, 0, 0) == BST_CHECKED )
				{
					if (HiddenModuleInjectDll_RemoteThread(hwndInject) != 0)
					{
						MessageBox(hwndInject, TEXT("ע��ʧ�ܣ�"), TEXT("Error"), MB_OK);
					}
					else
					{
						MessageBox(hwndInject, TEXT("ע��ɹ���"), TEXT("Success"), MB_OK);
					}
				}
				else if (SendMessage(GetDlgItem(hwndInject, IDC_RADIO_HiddenModule_SuspendThread), BM_GETCHECK, 0, 0) == BST_CHECKED)
				{
					if (HiddenModuleInjectDll_SuspendThread(hwndInject) != 0)
					{
						MessageBox(hwndInject, TEXT("ע��ʧ�ܣ�"), TEXT("Error"), MB_OK);
					}
					else
					{
						MessageBox(hwndInject, TEXT("ע��ɹ���"), TEXT("Success"), MB_OK);
					}
				}
				else if (SendMessage(GetDlgItem(hwndInject, IDC_RADIO_LoadProcess), BM_GETCHECK, 0, 0) == BST_CHECKED)
				{
					if (LoadProcessInject(hwndInject) != 0)
					{
						MessageBox(hwndInject, TEXT("ע��ʧ�ܣ�"), TEXT("Error"), MB_OK);
					}
					else
					{
						MessageBox(hwndInject, TEXT("ע��ɹ���"), TEXT("Success"), MB_OK);
					}
				}
				else
				{
					MessageBox(NULL, "Please Selected Inject Mode", "Wrong", MB_OK);
				}
				return true;

			case IDC_BUTTON_Uninstall:
				//�жϵ�ѡ���Ƿ�ѡ��
				if(SendMessage(GetDlgItem(hwndInject, IDC_RADIO_RemoteThread), BM_GETCHECK, 0, 0) == BST_CHECKED )
				{
					if(RemoteThreadUninstallDll(hwndInject) != 0)
					{
						MessageBox(hwndInject, TEXT("ж��ʧ�ܣ�"), TEXT("Error"), MB_OK);
					}
					else
					{
						MessageBox(hwndInject, TEXT("ж�سɹ���"), TEXT("Success"), MB_OK);
					}
				}
				else if( SendMessage(GetDlgItem(hwndInject, IDC_RADIO_HiddenModule), BM_GETCHECK, 0, 0) == BST_CHECKED )
				{
					if(HiddenModuleUninstallDll(hwndInject) != 0)
					{
						MessageBox(hwndInject, TEXT("ע��ʧ�ܣ�"), TEXT("Error"), MB_OK);
					}
					else
					{
						MessageBox(hwndInject, TEXT("ע��ɹ���"), TEXT("Success"), MB_OK);
					}
				}
				else
				{
					MessageBox(NULL, "Please Selected Uninstall Mode", "Wrong", MB_OK);
				}
				return true;				
			
			
			case IDC_BUTTON_DLL:
				//���ô��ļ�
				OPENFILENAME stOpenFile;
				//�ļ�������
				TCHAR szPeFileDll[200] = "(*.dll)\0*.dll\0(*.exe)\0*.exe\0";
											//�趨������Ҫ�ÿ��ַ���β\0   ��ʽ��(*.exe)\0*.exe\0
				
				memset(szPePath, 0, MAX_PATH);
				memset(&stOpenFile, 0, sizeof(OPENFILENAME));

				stOpenFile.lStructSize = sizeof(OPENFILENAME);
				stOpenFile.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
				stOpenFile.hwndOwner = hwndInject;
				stOpenFile.lpstrFilter = szPeFileDll;
				stOpenFile.lpstrFile = szPePath;
				stOpenFile.nMaxFile = MAX_PATH;//����256�ֽ�/�ַ�
				
				//��ȡ�ļ�·��
				//The GetOpenFileName function creates an Open dialog box that lets the 
				//user specify the drive, directory, and the name of a file or set of files to open.
				if(GetOpenFileName(&stOpenFile))
				{ 
					HWND hwndDllPath = GetDlgItem(hwndInject, IDC_EDIT_DLL);
					SetWindowText(hwndDllPath, szPePath);
				}
				else
				{
					//û��ѡ���ļ�
					MessageBox(NULL, TEXT("Open File Failed"), TEXT("Error"), MB_OK);
				}
				return true;			
			}
			break;
		}
	case WM_CLOSE:
		EndDialog(hwndInject, 0);
		return true;
		
	}
	return false;
}


DWORD RemoteThreadInjectDll(HWND hwndInject)
{
	DWORD ret = 0;
	memset(szProcessID, 0, MAX_PATH);

	//��ȡPID��Ϣ
	HWND hwndPID = GetDlgItem(hwndInject, IDC_EDIT_PID);
	GetWindowText(hwndPID, szProcessID, MAX_PATH);

	if(strlen(szProcessID) ==  0)
	{
		ret = -22;
		MessageBox(NULL, TEXT("���������ID"), TEXT("ERROR"), MB_OK);
		return ret;
	}
	 
	if(strlen(szPePath) ==  0)
	{
		ret = -22;
		MessageBox(NULL, TEXT("������DLL·��"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	DWORD dwPID = 0;
	sscanf(szProcessID, "%d", &dwPID);

	//��ȡ���̾��
	HANDLE hProcess = ::OpenProcess( PROCESS_ALL_ACCESS, false, dwPID);
	if(hProcess == NULL)
	{
		ret = -3;
		MessageBox(NULL, TEXT("InjectDll OpenProcess error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//Զ�̷���ռ�
	LPVOID pDllPathAddr = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if(pDllPathAddr == NULL)
	{
		ret = -4;
		MessageBox(NULL, TEXT("InjectDll VirtualAllocEx error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//д������ڴ�
	if(WriteProcessMemory(hProcess, pDllPathAddr, szPePath, MAX_PATH, NULL) == 0)
	{
		ret = -5;
		MessageBox(NULL, TEXT("InjectDll WriteProcessMemory error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//����ֱ�Ӵ��뺯������ Ҳ���Ի�ȡLoadLibraryA�ĵ�ַ
	//HMODULE hModule = GetModuleHandle("kernel32.dll");
	//LPVOID lpBaseAddress = (LPVOID)GetProcAddress(hModule,"LoadLibraryA");

	//����Զ���߳�
	HANDLE hRemoteThread = CreateRemoteThread(	
								hProcess,				// handle to process
								NULL,					// SD
								0,                      // initial stack size
								(LPTHREAD_START_ROUTINE)LoadLibrary,// thread function
								pDllPathAddr,           // thread argument
								NULL,					// creation option
								NULL					// thread identifier
								);
	if(hRemoteThread == NULL)
	{
		ret = -8;
		MessageBox(NULL, TEXT("InjectDll CreateRemoteThread error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//�ȴ��߳̽�����
	::WaitForSingleObject(hRemoteThread, INFINITE);
		
	//��ȡ�߳��˳���,��LoadLibrary�ķ���ֵ����dll���׵�ַ  
	DWORD dwExitCode = 0; 
	GetExitCodeThread(hRemoteThread, &dwExitCode);

	//�ͷ�ΪDLL��������Ŀռ�		
	if(VirtualFreeEx(hProcess, pDllPathAddr, MAX_PATH, MEM_DECOMMIT) == 0)
	{
		ret = -6;
		MessageBox(NULL, TEXT("InjectDll VirtualFreeEx error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//�رվ��	
	::CloseHandle(hRemoteThread);
	::CloseHandle(hProcess);
	hProcess = NULL;
	return ret;
}


DWORD RemoteThreadUninstallDll(HWND hwndInject)
{
	
	DWORD ret = 0;
	memset(szProcessID, 0, MAX_PATH);

	//��ȡPID��Ϣ
	HWND hwndPID = GetDlgItem(hwndInject, IDC_EDIT_PID);
	GetWindowText(hwndPID, szProcessID, MAX_PATH);

	//���̲���
	if(strlen(szProcessID) ==  0)
	{
		ret = -22;
		MessageBox(NULL, TEXT("���������ID"), TEXT("ERROR"), MB_OK);
		return ret;
	}
	 
	if(strlen(szPePath) ==  0)
	{
		ret = -22;
		MessageBox(NULL, TEXT("������DLL·��"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	DWORD dwPID = 0;
	sscanf(szProcessID, "%d", &dwPID);
	
	//��ȡ���̾��
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwPID);
	if(hProcess == NULL)
	{
		ret = -1;
		MessageBox(hwndInject, TEXT("UninstallDll OpenProcess error!"), TEXT("Error"), MB_OK);
		return ret;
	}


	//Զ�̷���ռ�
	LPVOID pDllPathAddr = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if(pDllPathAddr == NULL)
	{
		ret = -4;
		MessageBox(NULL, TEXT("UninstallDll VirtualAllocEx error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//д������ڴ�
	if(WriteProcessMemory(hProcess, pDllPathAddr, szPePath, MAX_PATH, NULL) == 0)
	{
		ret = -5;
		MessageBox(NULL, TEXT("UninstallDll WriteProcessMemory error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//����Զ���߳� ��ȡģ����
	HANDLE hRemoteThread = CreateRemoteThread(	
								hProcess,				// handle to process
								NULL,					// SD
								0,                      // initial stack size
								(LPTHREAD_START_ROUTINE)GetModuleHandle,// thread function
								pDllPathAddr,           // thread argument
								NULL,					// creation option
								NULL					// thread identifier
								);
	if(hRemoteThread == NULL)
	{
		ret = -8;
		MessageBox(NULL, TEXT("UninstallDll CreateRemoteThread error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//�ȴ��������
	::WaitForSingleObject(hRemoteThread,INFINITE);

	// ���GetModuleHandle�ķ���ֵ
	DWORD dwMoudle = 0;//(Ŀ��ģ��ľ��)
	GetExitCodeThread(hRemoteThread, &dwMoudle);
	::CloseHandle(hRemoteThread);

	hRemoteThread = CreateRemoteThread(	
								hProcess,				// handle to process
								NULL,					// SD
								0,                      // initial stack size
								(LPTHREAD_START_ROUTINE)FreeLibrary,// thread function
								(LPVOID)dwMoudle,           // thread argument
								NULL,					// creation option
								NULL					// thread identifier
								);
	if(hRemoteThread == NULL)
	{
		ret = -11;
		MessageBox(NULL, TEXT("UninstallDll CreateRemoteThread error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//�ȴ�����
	::WaitForSingleObject(hRemoteThread,INFINITE);
	
	//�ͷŷ���Ŀռ�
	VirtualFreeEx(hProcess,pDllPathAddr,MAX_PATH,MEM_COMMIT);
	
	//�رվ��
	CloseHandle(hRemoteThread);	
	CloseHandle(hProcess);

	return ret;
}

//*********************************************************************************
//****************����Զ���߳� ʵ������ע��****************************************
//*********************************************************************************
//ȱ�����ױ�����

extern "C" VOID __declspec(naked) entry_RemoteThreat()
{
	__asm
	{
		push		ebp;
		mov		ebp, esp;
		sub		esp, 0xc0;
		pushad;
		pushfd;
		lea		edi, DWORD PTR SS : [ebp - 0xc0];
		mov		ecx, 0x30;
		mov		eax, 0xcccccccc;
		rep		stos DWORD PTR ES : [edi];

		call		EntryProc;

		popfd;
		popad;
		mov		esp, ebp;
		pop		ebp;
		//ret;
	}
	TerminateThread(GetCurrentThread(), 7);
	SuspendThread(GetCurrentThread());
}

DWORD HiddenModuleInjectDll_RemoteThread(HWND hwndInject)
{
	DWORD ret = 0;

	//1����ȡ������̾��
	HANDLE hProcess = GetCurrentProcess();
	DWORD dwPID = 0;
	if(hProcess == NULL)
	{
		ret = -1;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll GetCurrentProcess Error"), TEXT("Error"), MB_OK);
		return ret;
	}
	//��ȡPID
	dwPID = GetCurrentProcessId();

	//2����ȡ�Լ���ImageBase��SizeOfImage
	DWORD dwImageBase = 0;			//�����ַ
	DWORD dwSizeOfImage = 0;		//�����С
	TCHAR szImageBase[10] = {0};     //�����ַ
	TCHAR szSizeOfImage[10] = {0};   //�����С
	HANDLE hModSnapshot=NULL;		//ģ����վ��
	MODULEENTRY32 MoudleEntry = {0};//This structure describes an entry from a list 
									//that enumerates the modules used by a specified process.

	//��ȡ��Ӧ���̵�ģ�����
	hModSnapshot =CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (hModSnapshot ==INVALID_HANDLE_VALUE)
	{
		ret = -3;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll CreateToolhelp32Snapshot Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	ZeroMemory(&MoudleEntry,sizeof(MoudleEntry));
	MoudleEntry.dwSize = sizeof(MoudleEntry);

	//�˺������������������ĵ�һ��ģ�����Ϣ
	if(Module32First(hModSnapshot, &MoudleEntry))
	{
		//�����̵�ImageBase ��SizeOfImage  ���뻺��
		sprintf(szImageBase,"%08X",MoudleEntry.modBaseAddr);
		sprintf(szSizeOfImage,"%08X",MoudleEntry.modBaseSize);
		
		sscanf(szImageBase, "%X", &dwImageBase);
		sscanf(szSizeOfImage, "%X", &dwSizeOfImage);
	}
	

	//3����Ҫע���A����
	//��ȡĿ�����PID
	HWND hwndPID = GetDlgItem(hwndInject, IDC_EDIT_PID);
	GetWindowText(hwndPID, szProcessID, MAX_PATH);
	DWORD dwObjPID = 0;
	sscanf(szProcessID, "%d", &dwObjPID);
	
	//�򿪽���
	HANDLE hObjProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwObjPID);
	if(hObjProcess == NULL)
	{
		ret = -5;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll OpenProcess Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//4����A�����������ڴ棬��С����SizeOfImage		�����һ��ռ䱣���̷߳���ֵ
	LPVOID pObjAddr = VirtualAllocEx(hObjProcess, NULL, dwSizeOfImage, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (pObjAddr == NULL)
	{
		ret = -7;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll VirtualAllocEx error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}
	NewBase = pObjAddr;
	dwImageSize = dwSizeOfImage;

	//5������һ���µĻ����������Լ����ƽ�ȥ	   ������
	LPVOID pDataAdd = malloc(dwSizeOfImage);
	if (pDataAdd == NULL)
	{
		ret = -4;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll malloc Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}
	ZeroMemory(pDataAdd, dwSizeOfImage);
	memcpy(pDataAdd, (LPVOID)dwImageBase, dwSizeOfImage);
	//6���޸���ҪCopy���ݵ��ض�λ��
	ret = RecoveryRelocation2(pDataAdd, dwImageBase, (DWORD)pObjAddr);
	if(ret != 0)
	{
		ret = -8;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll RecoveryRelocation error!"), TEXT("ERROR"), MB_OK);
		free(pDataAdd);
		return ret;
	}

	//7�����޸�������ݣ����Ƶ�A���ڴ���
	if(!WriteProcessMemory(hObjProcess, pObjAddr, pDataAdd, dwSizeOfImage, NULL))
	{
		ret = -8;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll WriteProcessMemory error!"), TEXT("ERROR"), MB_OK);
		free(pDataAdd);
		return ret;
	}

	//8������һ��Զ���̣߳�ִ��Entry
	HANDLE hRemoteThread = CreateRemoteThread(
								hObjProcess,			// handle to process
								NULL,					// SD
								0,                      // initial stack size
								(LPTHREAD_START_ROUTINE)entry_RemoteThreat,// thread function
								NULL,				// thread argument ������ռ�ĵ�ַ����
								CREATE_SUSPENDED,		// creation option
								NULL					// thread identifier
								);

	CONTEXT context_New;   //���潫Ҫ��ת�ĵ�ַ

	context_New.ContextFlags = CONTEXT_FULL;	   //����Ȩ��
	
	GetThreadContext(hRemoteThread, &context_New);	   //��ȡ

	//�ض�λIAT entry�ĵ�ַ = ԭ��ַ - �ɵĻ�ַ + �µĻ�ַ
	DWORD dwIATentry = (DWORD)entry_RemoteThreat - dwImageBase + (DWORD)pObjAddr;
	context_New.Eip = dwIATentry;

	SetThreadContext(hRemoteThread, &context_New);

	ResumeThread(hRemoteThread);		//�ָ�
	
	//�ͷž��
	CloseHandle(hRemoteThread);
	CloseHandle(hObjProcess);

	free(pDataAdd);

	return ret;
}

DWORD HiddenModuleUninstallDll(HWND hwndInject)
{
	DWORD ret = 0;


	return ret;
}



//*********************************************************************************
//****************����Զ���߳� ʵ������ע��****************************************
//*********************************************************************************
//ȱ������Ӱ�����ִ��

DWORD dwRetAddress;
extern "C" VOID __declspec(naked) entry()
{
	//��ȡ����ֵ
	dwRetAddress = *(PDWORD)((PBYTE)NewBase + dwImageSize + 0x4);
	__asm
	{
		push		dwRetAddress;
		push		ebp;
		mov		ebp, esp;
		sub		esp, 0xc0;
		pushad;
		pushfd;
		lea		edi, DWORD PTR SS : [ebp - 0xc0];
		mov		ecx, 0x30;
		mov		eax, 0xcccccccc;
		rep		stos DWORD PTR ES : [edi];

		call		EntryProc;

		popfd;
		popad;
		mov		esp, ebp;
		pop		ebp;
		ret;
	}
}

DWORD HiddenModuleInjectDll_SuspendThread(HWND hwndInject)
{
	DWORD ret = 0;

	//1����ȡ������̾��
	HANDLE hProcess = GetCurrentProcess();
	DWORD dwPID = 0;
	if (hProcess == NULL)
	{
		ret = -1;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll GetCurrentProcess Error"), TEXT("Error"), MB_OK);
		return ret;
	}
	//��ȡPID
	dwPID = GetCurrentProcessId();

	//2����ȡ�Լ���ImageBase��SizeOfImage
	DWORD dwImageBase = 0;			//�����ַ
	DWORD dwSizeOfImage = 0;		//�����С
	TCHAR szImageBase[10] = { 0 };     //�����ַ
	TCHAR szSizeOfImage[10] = { 0 };   //�����С
	HANDLE hModSnapshot = NULL;		//ģ����վ��
	MODULEENTRY32 MoudleEntry = { 0 };//This structure describes an entry from a list 
	//that enumerates the modules used by a specified process.

	//��ȡ��Ӧ���̵�ģ�����
	hModSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (hModSnapshot == INVALID_HANDLE_VALUE)
	{
		ret = -3;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll CreateToolhelp32Snapshot Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	ZeroMemory(&MoudleEntry, sizeof(MoudleEntry));
	MoudleEntry.dwSize = sizeof(MoudleEntry);

	//�˺������������������ĵ�һ��ģ�����Ϣ
	if (Module32First(hModSnapshot, &MoudleEntry))
	{
		//�����̵�ImageBase ��SizeOfImage  ���뻺��
		sprintf(szImageBase, "%08X", MoudleEntry.modBaseAddr);
		sprintf(szSizeOfImage, "%08X", MoudleEntry.modBaseSize);

		sscanf(szImageBase, "%X", &dwImageBase);
		sscanf(szSizeOfImage, "%X", &dwSizeOfImage);
	}


	//3����Ҫע���A����
	//��ȡĿ�����PID
	HWND hwndPID = GetDlgItem(hwndInject, IDC_EDIT_PID);
	GetWindowText(hwndPID, szProcessID, MAX_PATH);
	DWORD dwObjPID = 0;
	sscanf(szProcessID, "%d", &dwObjPID);

	////�򿪽���
	HANDLE hObjProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwObjPID);
	if (hObjProcess == NULL)
	{
		ret = -5;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll OpenProcess Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//4����ȡ�ý��̵�һ���߳�
	DWORD dwObjThreadID = 0;
	ret = GetThreadByPorcessID(dwObjPID, &dwObjThreadID);
	if (ret != 0)
	{
		MessageBox(NULL, TEXT("HiddenModuleInjectDll GetThreadByPorcessID Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}
	//	��ȡ�߳̾��
	HANDLE hObjThread = OpenThread(THREAD_ALL_ACCESS, FALSE, dwObjThreadID);
	if (hObjThread == NULL)
	{
		ret = -33;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll OpenThread Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//5�������߳�
	SuspendThread(hObjThread);

	//6����Ŀ������з���ռ�
	LPVOID pObjAddr = VirtualAllocEx(hObjProcess, NULL, dwSizeOfImage + 0x10, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (pObjAddr == NULL)
	{
		ret = -7;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll VirtualAllocEx error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}
	NewBase = pObjAddr;
	dwImageSize = dwSizeOfImage;

	//7������һ���µĻ����������Լ����ƽ�ȥ	   ������
	LPVOID pDataAdd = malloc(dwSizeOfImage);
	if (pDataAdd == NULL)
	{
		ret = -4;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll malloc Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}
	ZeroMemory(pDataAdd, dwSizeOfImage);
	memcpy(pDataAdd, (LPVOID)dwImageBase, dwSizeOfImage);

	//8���޸���ҪCopy���ݵ��ض�λ��
	ret = RecoveryRelocation2(pDataAdd, dwImageBase, (DWORD)pObjAddr);
	if (ret != 0)
	{
		ret = -8;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll RecoveryRelocation error!"), TEXT("ERROR"), MB_OK);
		free(pDataAdd);
		return ret;
	}

	//9�����޸�������ݣ����Ƶ�A���ڴ���
	if (!WriteProcessMemory(hObjProcess, pObjAddr, pDataAdd, dwSizeOfImage, NULL))
	{
		ret = -8;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll WriteProcessMemory error!"), TEXT("ERROR"), MB_OK);
		free(pDataAdd);
		return ret;
	}

	//�޸��߳�������
	CONTEXT context_New;   //���潫Ҫ��ת�ĵ�ַ
	CONTEXT context_Old;	 //�����̷߳��صĵ�ַ

	context_New.ContextFlags = CONTEXT_FULL;	   //����Ȩ��
	context_Old.ContextFlags = CONTEXT_FULL;

	GetThreadContext(hObjThread, &context_New);	   //��ȡ
	GetThreadContext(hObjThread, &context_Old);	   //��ȡ

	//����ǰ���߳�EIP����
	WriteProcessMemory(hObjProcess, (LPVOID)((BYTE*)NewBase + dwImageSize + 0x4), (&context_Old.Eip), sizeof(DWORD), NULL);

	//�ض�λIAT entry�ĵ�ַ = ԭ��ַ - �ɵĻ�ַ + �µĻ�ַ
	DWORD dwIATentry = (DWORD)entry - dwImageBase + (DWORD)pObjAddr;
	context_New.Eip = dwIATentry;

	SetThreadContext(hObjThread, &context_New);

	ResumeThread(hObjThread);		//�ָ�

	//�ͷž��
	CloseHandle(hObjThread);
	CloseHandle(hObjProcess);

	free(pDataAdd);
	return ret;
}


DWORD GetThreadByPorcessID(DWORD dwObjPID, PDWORD pObjThreadID)
{
	DWORD ret = 0;

	THREADENTRY32 th32;
	th32.dwSize = sizeof(THREADENTRY32);
	HANDLE hdTool = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	if (Thread32First(hdTool, &th32))
	{
		//��������  ��ȡ��Ӧ�Ľ�����Ϣ
		do
		{
			//��ȡ�߳�ID
			if (th32.th32OwnerProcessID == dwObjPID)
			{
				*pObjThreadID = th32.th32ThreadID;
				break;
			}
		} while (Thread32Next(hdTool, &th32));
	}
	else
	{
		ret = -44;
		return ret;
	}

	return ret;
}

//ע���ִ��
VOID WINAPI EntryProc()
{
	DWORD ret = 0;

	LPVOID pDataBase = NewBase;

	//��ȡLoadLibraryA�ĵ�ַ
	typedef HMODULE(WINAPI * PLoadLibrary)(LPCTSTR);
	HMODULE hKernel = GetModuleHandle(TEXT("Kernel32.dll"));
	PLoadLibrary pLoadLibrary = (PLoadLibrary)GetProcAddress(hKernel, "LoadLibraryA");

	//�޸�IAT��		ͨ��IAT = loadlibary(.dll, hint/name);
	//1����ʼ��PE�ļ�ͷ
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pDataBase;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pDataBase + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//�ж�MZ��־
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		ret = -3;
		MessageBox(0, TEXT("������Ч��MZ��־"), TEXT("Error"), MB_OK);
		//return ret;
	}
	//�ж��Ƿ�����Ч��PE 
	if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		ret = -3;
		MessageBox(0, TEXT("������Ч��PE��־"), TEXT("Error"), MB_OK);
		//return ret;
	}

	//��λ�����  �����Ѿ����ڴ���չ�������Բ���Ҫת��
	PIMAGE_IMPORT_DESCRIPTOR pImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)((PBYTE)pDataBase + pNTHeader->OptionalHeader.DataDirectory[1].VirtualAddress);

	//ѭ�������
	while (pImportDirectory->Name != NULL)
	{
		TCHAR *pDllName = NULL;//ģ����
		TCHAR *pApiName = NULL;//������

		//��ȡDLL��
		pDllName = (TCHAR *)((PBYTE)pDataBase + pImportDirectory->Name);

		//�ж�ģ���Ƿ���ڣ�������������ֶ�Load
		HMODULE hModule;
		if ((hModule = GetModuleHandleA(pDllName)) == NULL)
		{
			hModule = pLoadLibrary(pDllName);
		}

		//��IAT���INT��ֵ  Ϊ�������DWORD����
		PDWORD pThunkINT = (PDWORD)((PBYTE)pDataBase + pImportDirectory->OriginalFirstThunk);
		PDWORD pThunkIAT = (PDWORD)((PBYTE)pDataBase + pImportDirectory->FirstThunk);

		//ѭ������ģ��API
		while ((*pThunkINT) != 0)
		{
			//�ж��Ǻ�����ŵ��룬�������Ƶ���
			if ((*pThunkINT) & 0x80000000)//���λ��1Ϊ�������
			{
				//��ȡ���
				DWORD dwOrdinal = (*pThunkINT) & 0x7FFFFFFF;
				//��ȡ������ַ
				(*pThunkIAT) = (DWORD)GetProcAddress(hModule, (LPCSTR)dwOrdinal);
				if ((*pThunkIAT) == NULL)
				{
					ret = -9;
					MessageBox(0, TEXT("ThreadProc GetProcAddress By Ordinal Error"), TEXT("Error"), MB_OK);
					//			return ret;
				}
			}
			else//������
			{
				//��ȡ��������ַ
				PIMAGE_IMPORT_BY_NAME pImportByName = (PIMAGE_IMPORT_BY_NAME)((PBYTE)pDataBase + (*pThunkINT));
				pApiName = (TCHAR *)pImportByName->Name;

				//��ȡ������ַ
				(*pThunkIAT) = (DWORD)GetProcAddress(hModule, (LPCSTR)pApiName);
				if ((*pThunkIAT) == NULL)
				{
					ret = -9;
					MessageBox(0, TEXT("ThreadProc GetProcAddress By Name Error"), TEXT("Error"), MB_OK);
					//			return ret;
				}

			}
			pThunkINT++;
			pThunkIAT++;
		}
		//����
		pImportDirectory++;
	}

	//ִ�еĴ���	  �޷���ֵ
	//**********************************************************************************************************
	//while(true)
	//{
	MessageBox(NULL, TEXT("HiddenModuleInjectDll !"), TEXT("Success"), MB_OK);
	//	Sleep(3000);
	//}
	//************************************************************************************************************
}

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&   �� �� �� �� ע ��   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


DWORD LoadProcessInject(HWND hwndInject)
{
	DWORD ret = 0;


	MessageBox(NULL, TEXT("�÷���û��ʵ�֣�"), TEXT("ERROR"), MB_OK);
	return ret;
	//��ȡҪ�����ļ���·��
	if (strlen(szPePath) == 0)
	{
		ret = -22;
		MessageBox(NULL, TEXT("������EXE�ļ�·��"), TEXT("ERROR"), MB_OK);
		return ret;
	}


	return ret;
}
