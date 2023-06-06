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
				//判断单选框是否被选中  远程线程
				if( SendMessage(GetDlgItem(hwndInject, IDC_RADIO_RemoteThread), BM_GETCHECK, 0, 0) == BST_CHECKED )
				{
					if(RemoteThreadInjectDll(hwndInject) != 0)
					{
						MessageBox(hwndInject, TEXT("注入失败！"), TEXT("Error"), MB_OK);
					}
					else
					{
						MessageBox(hwndInject, TEXT("注入成功！"), TEXT("Success"), MB_OK);
					}
				}//隐藏模块注入
				else if( SendMessage(GetDlgItem(hwndInject, IDC_RADIO_HiddenModule_RemoteThread), BM_GETCHECK, 0, 0) == BST_CHECKED )
				{
					if (HiddenModuleInjectDll_RemoteThread(hwndInject) != 0)
					{
						MessageBox(hwndInject, TEXT("注入失败！"), TEXT("Error"), MB_OK);
					}
					else
					{
						MessageBox(hwndInject, TEXT("注入成功！"), TEXT("Success"), MB_OK);
					}
				}
				else if (SendMessage(GetDlgItem(hwndInject, IDC_RADIO_HiddenModule_SuspendThread), BM_GETCHECK, 0, 0) == BST_CHECKED)
				{
					if (HiddenModuleInjectDll_SuspendThread(hwndInject) != 0)
					{
						MessageBox(hwndInject, TEXT("注入失败！"), TEXT("Error"), MB_OK);
					}
					else
					{
						MessageBox(hwndInject, TEXT("注入成功！"), TEXT("Success"), MB_OK);
					}
				}
				else if (SendMessage(GetDlgItem(hwndInject, IDC_RADIO_LoadProcess), BM_GETCHECK, 0, 0) == BST_CHECKED)
				{
					if (LoadProcessInject(hwndInject) != 0)
					{
						MessageBox(hwndInject, TEXT("注入失败！"), TEXT("Error"), MB_OK);
					}
					else
					{
						MessageBox(hwndInject, TEXT("注入成功！"), TEXT("Success"), MB_OK);
					}
				}
				else
				{
					MessageBox(NULL, "Please Selected Inject Mode", "Wrong", MB_OK);
				}
				return true;

			case IDC_BUTTON_Uninstall:
				//判断单选框是否被选中
				if(SendMessage(GetDlgItem(hwndInject, IDC_RADIO_RemoteThread), BM_GETCHECK, 0, 0) == BST_CHECKED )
				{
					if(RemoteThreadUninstallDll(hwndInject) != 0)
					{
						MessageBox(hwndInject, TEXT("卸载失败！"), TEXT("Error"), MB_OK);
					}
					else
					{
						MessageBox(hwndInject, TEXT("卸载成功！"), TEXT("Success"), MB_OK);
					}
				}
				else if( SendMessage(GetDlgItem(hwndInject, IDC_RADIO_HiddenModule), BM_GETCHECK, 0, 0) == BST_CHECKED )
				{
					if(HiddenModuleUninstallDll(hwndInject) != 0)
					{
						MessageBox(hwndInject, TEXT("注入失败！"), TEXT("Error"), MB_OK);
					}
					else
					{
						MessageBox(hwndInject, TEXT("注入成功！"), TEXT("Success"), MB_OK);
					}
				}
				else
				{
					MessageBox(NULL, "Please Selected Uninstall Mode", "Wrong", MB_OK);
				}
				return true;				
			
			
			case IDC_BUTTON_DLL:
				//设置打开文件
				OPENFILENAME stOpenFile;
				//文件过滤器
				TCHAR szPeFileDll[200] = "(*.dll)\0*.dll\0(*.exe)\0*.exe\0";
											//设定过滤器要用空字符结尾\0   格式：(*.exe)\0*.exe\0
				
				memset(szPePath, 0, MAX_PATH);
				memset(&stOpenFile, 0, sizeof(OPENFILENAME));

				stOpenFile.lStructSize = sizeof(OPENFILENAME);
				stOpenFile.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
				stOpenFile.hwndOwner = hwndInject;
				stOpenFile.lpstrFilter = szPeFileDll;
				stOpenFile.lpstrFile = szPePath;
				stOpenFile.nMaxFile = MAX_PATH;//最少256字节/字符
				
				//获取文件路径
				//The GetOpenFileName function creates an Open dialog box that lets the 
				//user specify the drive, directory, and the name of a file or set of files to open.
				if(GetOpenFileName(&stOpenFile))
				{ 
					HWND hwndDllPath = GetDlgItem(hwndInject, IDC_EDIT_DLL);
					SetWindowText(hwndDllPath, szPePath);
				}
				else
				{
					//没有选择文件
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

	//获取PID信息
	HWND hwndPID = GetDlgItem(hwndInject, IDC_EDIT_PID);
	GetWindowText(hwndPID, szProcessID, MAX_PATH);

	if(strlen(szProcessID) ==  0)
	{
		ret = -22;
		MessageBox(NULL, TEXT("请输入进程ID"), TEXT("ERROR"), MB_OK);
		return ret;
	}
	 
	if(strlen(szPePath) ==  0)
	{
		ret = -22;
		MessageBox(NULL, TEXT("请输入DLL路径"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	DWORD dwPID = 0;
	sscanf(szProcessID, "%d", &dwPID);

	//获取进程句柄
	HANDLE hProcess = ::OpenProcess( PROCESS_ALL_ACCESS, false, dwPID);
	if(hProcess == NULL)
	{
		ret = -3;
		MessageBox(NULL, TEXT("InjectDll OpenProcess error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//远程分配空间
	LPVOID pDllPathAddr = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if(pDllPathAddr == NULL)
	{
		ret = -4;
		MessageBox(NULL, TEXT("InjectDll VirtualAllocEx error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//写入进程内存
	if(WriteProcessMemory(hProcess, pDllPathAddr, szPePath, MAX_PATH, NULL) == 0)
	{
		ret = -5;
		MessageBox(NULL, TEXT("InjectDll WriteProcessMemory error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//可以直接传入函数名， 也可以获取LoadLibraryA的地址
	//HMODULE hModule = GetModuleHandle("kernel32.dll");
	//LPVOID lpBaseAddress = (LPVOID)GetProcAddress(hModule,"LoadLibraryA");

	//创建远程线程
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

	//等待线程结束！
	::WaitForSingleObject(hRemoteThread, INFINITE);
		
	//获取线程退出码,即LoadLibrary的返回值，即dll的首地址  
	DWORD dwExitCode = 0; 
	GetExitCodeThread(hRemoteThread, &dwExitCode);

	//释放为DLL名字申请的空间		
	if(VirtualFreeEx(hProcess, pDllPathAddr, MAX_PATH, MEM_DECOMMIT) == 0)
	{
		ret = -6;
		MessageBox(NULL, TEXT("InjectDll VirtualFreeEx error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//关闭句柄	
	::CloseHandle(hRemoteThread);
	::CloseHandle(hProcess);
	hProcess = NULL;
	return ret;
}


DWORD RemoteThreadUninstallDll(HWND hwndInject)
{
	
	DWORD ret = 0;
	memset(szProcessID, 0, MAX_PATH);

	//获取PID信息
	HWND hwndPID = GetDlgItem(hwndInject, IDC_EDIT_PID);
	GetWindowText(hwndPID, szProcessID, MAX_PATH);

	//进程参数
	if(strlen(szProcessID) ==  0)
	{
		ret = -22;
		MessageBox(NULL, TEXT("请输入进程ID"), TEXT("ERROR"), MB_OK);
		return ret;
	}
	 
	if(strlen(szPePath) ==  0)
	{
		ret = -22;
		MessageBox(NULL, TEXT("请输入DLL路径"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	DWORD dwPID = 0;
	sscanf(szProcessID, "%d", &dwPID);
	
	//获取进程句柄
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwPID);
	if(hProcess == NULL)
	{
		ret = -1;
		MessageBox(hwndInject, TEXT("UninstallDll OpenProcess error!"), TEXT("Error"), MB_OK);
		return ret;
	}


	//远程分配空间
	LPVOID pDllPathAddr = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if(pDllPathAddr == NULL)
	{
		ret = -4;
		MessageBox(NULL, TEXT("UninstallDll VirtualAllocEx error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//写入进程内存
	if(WriteProcessMemory(hProcess, pDllPathAddr, szPePath, MAX_PATH, NULL) == 0)
	{
		ret = -5;
		MessageBox(NULL, TEXT("UninstallDll WriteProcessMemory error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//创建远程线程 获取模块句柄
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

	//等待函数完成
	::WaitForSingleObject(hRemoteThread,INFINITE);

	// 获得GetModuleHandle的返回值
	DWORD dwMoudle = 0;//(目标模块的句柄)
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

	//等待结束
	::WaitForSingleObject(hRemoteThread,INFINITE);
	
	//释放分配的空间
	VirtualFreeEx(hProcess,pDllPathAddr,MAX_PATH,MEM_COMMIT);
	
	//关闭句柄
	CloseHandle(hRemoteThread);	
	CloseHandle(hProcess);

	return ret;
}

//*********************************************************************************
//****************创建远程线程 实现隐藏注入****************************************
//*********************************************************************************
//缺点容易被检测出

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

	//1、获取自身进程句柄
	HANDLE hProcess = GetCurrentProcess();
	DWORD dwPID = 0;
	if(hProcess == NULL)
	{
		ret = -1;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll GetCurrentProcess Error"), TEXT("Error"), MB_OK);
		return ret;
	}
	//获取PID
	dwPID = GetCurrentProcessId();

	//2、获取自己的ImageBase和SizeOfImage
	DWORD dwImageBase = 0;			//镜像基址
	DWORD dwSizeOfImage = 0;		//镜像大小
	TCHAR szImageBase[10] = {0};     //镜像基址
	TCHAR szSizeOfImage[10] = {0};   //镜像大小
	HANDLE hModSnapshot=NULL;		//模块快照句柄
	MODULEENTRY32 MoudleEntry = {0};//This structure describes an entry from a list 
									//that enumerates the modules used by a specified process.

	//获取相应进程的模块快照
	hModSnapshot =CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (hModSnapshot ==INVALID_HANDLE_VALUE)
	{
		ret = -3;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll CreateToolhelp32Snapshot Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	ZeroMemory(&MoudleEntry,sizeof(MoudleEntry));
	MoudleEntry.dwSize = sizeof(MoudleEntry);

	//此函数检索与进程相关联的第一个模块的信息
	if(Module32First(hModSnapshot, &MoudleEntry))
	{
		//将进程的ImageBase ，SizeOfImage  放入缓存
		sprintf(szImageBase,"%08X",MoudleEntry.modBaseAddr);
		sprintf(szSizeOfImage,"%08X",MoudleEntry.modBaseSize);
		
		sscanf(szImageBase, "%X", &dwImageBase);
		sscanf(szSizeOfImage, "%X", &dwSizeOfImage);
	}
	

	//3、打开要注入的A进程
	//获取目标进程PID
	HWND hwndPID = GetDlgItem(hwndInject, IDC_EDIT_PID);
	GetWindowText(hwndPID, szProcessID, MAX_PATH);
	DWORD dwObjPID = 0;
	sscanf(szProcessID, "%d", &dwObjPID);
	
	//打开进程
	HANDLE hObjProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwObjPID);
	if(hObjProcess == NULL)
	{
		ret = -5;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll OpenProcess Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//4、在A进程中申请内存，大小就是SizeOfImage		多分配一点空间保存线程返回值
	LPVOID pObjAddr = VirtualAllocEx(hObjProcess, NULL, dwSizeOfImage, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (pObjAddr == NULL)
	{
		ret = -7;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll VirtualAllocEx error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}
	NewBase = pObjAddr;
	dwImageSize = dwSizeOfImage;

	//5、创建一个新的缓冲区，将自己复制进去	   已拉伸
	LPVOID pDataAdd = malloc(dwSizeOfImage);
	if (pDataAdd == NULL)
	{
		ret = -4;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll malloc Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}
	ZeroMemory(pDataAdd, dwSizeOfImage);
	memcpy(pDataAdd, (LPVOID)dwImageBase, dwSizeOfImage);
	//6、修复将要Copy数据的重定位表
	ret = RecoveryRelocation2(pDataAdd, dwImageBase, (DWORD)pObjAddr);
	if(ret != 0)
	{
		ret = -8;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll RecoveryRelocation error!"), TEXT("ERROR"), MB_OK);
		free(pDataAdd);
		return ret;
	}

	//7、将修复后的数据，复制到A的内存中
	if(!WriteProcessMemory(hObjProcess, pObjAddr, pDataAdd, dwSizeOfImage, NULL))
	{
		ret = -8;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll WriteProcessMemory error!"), TEXT("ERROR"), MB_OK);
		free(pDataAdd);
		return ret;
	}

	//8、创建一个远程线程，执行Entry
	HANDLE hRemoteThread = CreateRemoteThread(
								hObjProcess,			// handle to process
								NULL,					// SD
								0,                      // initial stack size
								(LPTHREAD_START_ROUTINE)entry_RemoteThreat,// thread function
								NULL,				// thread argument 将分配空间的地址传入
								CREATE_SUSPENDED,		// creation option
								NULL					// thread identifier
								);

	CONTEXT context_New;   //保存将要跳转的地址

	context_New.ContextFlags = CONTEXT_FULL;	   //设置权限
	
	GetThreadContext(hRemoteThread, &context_New);	   //获取

	//重定位IAT entry的地址 = 原地址 - 旧的基址 + 新的基址
	DWORD dwIATentry = (DWORD)entry_RemoteThreat - dwImageBase + (DWORD)pObjAddr;
	context_New.Eip = dwIATentry;

	SetThreadContext(hRemoteThread, &context_New);

	ResumeThread(hRemoteThread);		//恢复
	
	//释放句柄
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
//****************挂起远程线程 实现隐藏注入****************************************
//*********************************************************************************
//缺点容易影响程序执行

DWORD dwRetAddress;
extern "C" VOID __declspec(naked) entry()
{
	//获取返回值
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

	//1、获取自身进程句柄
	HANDLE hProcess = GetCurrentProcess();
	DWORD dwPID = 0;
	if (hProcess == NULL)
	{
		ret = -1;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll GetCurrentProcess Error"), TEXT("Error"), MB_OK);
		return ret;
	}
	//获取PID
	dwPID = GetCurrentProcessId();

	//2、获取自己的ImageBase和SizeOfImage
	DWORD dwImageBase = 0;			//镜像基址
	DWORD dwSizeOfImage = 0;		//镜像大小
	TCHAR szImageBase[10] = { 0 };     //镜像基址
	TCHAR szSizeOfImage[10] = { 0 };   //镜像大小
	HANDLE hModSnapshot = NULL;		//模块快照句柄
	MODULEENTRY32 MoudleEntry = { 0 };//This structure describes an entry from a list 
	//that enumerates the modules used by a specified process.

	//获取相应进程的模块快照
	hModSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (hModSnapshot == INVALID_HANDLE_VALUE)
	{
		ret = -3;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll CreateToolhelp32Snapshot Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	ZeroMemory(&MoudleEntry, sizeof(MoudleEntry));
	MoudleEntry.dwSize = sizeof(MoudleEntry);

	//此函数检索与进程相关联的第一个模块的信息
	if (Module32First(hModSnapshot, &MoudleEntry))
	{
		//将进程的ImageBase ，SizeOfImage  放入缓存
		sprintf(szImageBase, "%08X", MoudleEntry.modBaseAddr);
		sprintf(szSizeOfImage, "%08X", MoudleEntry.modBaseSize);

		sscanf(szImageBase, "%X", &dwImageBase);
		sscanf(szSizeOfImage, "%X", &dwSizeOfImage);
	}


	//3、打开要注入的A进程
	//获取目标进程PID
	HWND hwndPID = GetDlgItem(hwndInject, IDC_EDIT_PID);
	GetWindowText(hwndPID, szProcessID, MAX_PATH);
	DWORD dwObjPID = 0;
	sscanf(szProcessID, "%d", &dwObjPID);

	////打开进程
	HANDLE hObjProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwObjPID);
	if (hObjProcess == NULL)
	{
		ret = -5;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll OpenProcess Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//4、获取该进程的一个线程
	DWORD dwObjThreadID = 0;
	ret = GetThreadByPorcessID(dwObjPID, &dwObjThreadID);
	if (ret != 0)
	{
		MessageBox(NULL, TEXT("HiddenModuleInjectDll GetThreadByPorcessID Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}
	//	获取线程句柄
	HANDLE hObjThread = OpenThread(THREAD_ALL_ACCESS, FALSE, dwObjThreadID);
	if (hObjThread == NULL)
	{
		ret = -33;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll OpenThread Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}

	//5、挂起线程
	SuspendThread(hObjThread);

	//6、在目标进程中分配空间
	LPVOID pObjAddr = VirtualAllocEx(hObjProcess, NULL, dwSizeOfImage + 0x10, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (pObjAddr == NULL)
	{
		ret = -7;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll VirtualAllocEx error!"), TEXT("ERROR"), MB_OK);
		return ret;
	}
	NewBase = pObjAddr;
	dwImageSize = dwSizeOfImage;

	//7、创建一个新的缓冲区，将自己复制进去	   已拉伸
	LPVOID pDataAdd = malloc(dwSizeOfImage);
	if (pDataAdd == NULL)
	{
		ret = -4;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll malloc Error"), TEXT("ERROR"), MB_OK);
		return ret;
	}
	ZeroMemory(pDataAdd, dwSizeOfImage);
	memcpy(pDataAdd, (LPVOID)dwImageBase, dwSizeOfImage);

	//8、修复将要Copy数据的重定位表
	ret = RecoveryRelocation2(pDataAdd, dwImageBase, (DWORD)pObjAddr);
	if (ret != 0)
	{
		ret = -8;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll RecoveryRelocation error!"), TEXT("ERROR"), MB_OK);
		free(pDataAdd);
		return ret;
	}

	//9、将修复后的数据，复制到A的内存中
	if (!WriteProcessMemory(hObjProcess, pObjAddr, pDataAdd, dwSizeOfImage, NULL))
	{
		ret = -8;
		MessageBox(NULL, TEXT("HiddenModuleInjectDll WriteProcessMemory error!"), TEXT("ERROR"), MB_OK);
		free(pDataAdd);
		return ret;
	}

	//修改线程上下文
	CONTEXT context_New;   //保存将要跳转的地址
	CONTEXT context_Old;	 //保存线程返回的地址

	context_New.ContextFlags = CONTEXT_FULL;	   //设置权限
	context_Old.ContextFlags = CONTEXT_FULL;

	GetThreadContext(hObjThread, &context_New);	   //获取
	GetThreadContext(hObjThread, &context_Old);	   //获取

	//将当前的线程EIP保存
	WriteProcessMemory(hObjProcess, (LPVOID)((BYTE*)NewBase + dwImageSize + 0x4), (&context_Old.Eip), sizeof(DWORD), NULL);

	//重定位IAT entry的地址 = 原地址 - 旧的基址 + 新的基址
	DWORD dwIATentry = (DWORD)entry - dwImageBase + (DWORD)pObjAddr;
	context_New.Eip = dwIATentry;

	SetThreadContext(hObjThread, &context_New);

	ResumeThread(hObjThread);		//恢复

	//释放句柄
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
		//遍历进程  获取相应的进程信息
		do
		{
			//获取线程ID
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

//注入后执行
VOID WINAPI EntryProc()
{
	DWORD ret = 0;

	LPVOID pDataBase = NewBase;

	//获取LoadLibraryA的地址
	typedef HMODULE(WINAPI * PLoadLibrary)(LPCTSTR);
	HMODULE hKernel = GetModuleHandle(TEXT("Kernel32.dll"));
	PLoadLibrary pLoadLibrary = (PLoadLibrary)GetProcAddress(hKernel, "LoadLibraryA");

	//修复IAT表		通过IAT = loadlibary(.dll, hint/name);
	//1、初始化PE文件头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pDataBase;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pDataBase + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		ret = -3;
		MessageBox(0, TEXT("不是有效的MZ标志"), TEXT("Error"), MB_OK);
		//return ret;
	}
	//判断是否是有效的PE 
	if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		ret = -3;
		MessageBox(0, TEXT("不是有效的PE标志"), TEXT("Error"), MB_OK);
		//return ret;
	}

	//定位导入表  由于已经在内存中展开，所以不需要转换
	PIMAGE_IMPORT_DESCRIPTOR pImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)((PBYTE)pDataBase + pNTHeader->OptionalHeader.DataDirectory[1].VirtualAddress);

	//循环导入表
	while (pImportDirectory->Name != NULL)
	{
		TCHAR *pDllName = NULL;//模块名
		TCHAR *pApiName = NULL;//函数名

		//获取DLL名
		pDllName = (TCHAR *)((PBYTE)pDataBase + pImportDirectory->Name);

		//判断模块是否存在，如果不存在则手动Load
		HMODULE hModule;
		if ((hModule = GetModuleHandleA(pDllName)) == NULL)
		{
			hModule = pLoadLibrary(pDllName);
		}

		//对IAT表和INT表赋值  为方便就用DWORD类型
		PDWORD pThunkINT = (PDWORD)((PBYTE)pDataBase + pImportDirectory->OriginalFirstThunk);
		PDWORD pThunkIAT = (PDWORD)((PBYTE)pDataBase + pImportDirectory->FirstThunk);

		//循环遍历模块API
		while ((*pThunkINT) != 0)
		{
			//判断是函数序号导入，还是名称导入
			if ((*pThunkINT) & 0x80000000)//最高位是1为导入序号
			{
				//获取序号
				DWORD dwOrdinal = (*pThunkINT) & 0x7FFFFFFF;
				//获取函数地址
				(*pThunkIAT) = (DWORD)GetProcAddress(hModule, (LPCSTR)dwOrdinal);
				if ((*pThunkIAT) == NULL)
				{
					ret = -9;
					MessageBox(0, TEXT("ThreadProc GetProcAddress By Ordinal Error"), TEXT("Error"), MB_OK);
					//			return ret;
				}
			}
			else//函数名
			{
				//获取函数名地址
				PIMAGE_IMPORT_BY_NAME pImportByName = (PIMAGE_IMPORT_BY_NAME)((PBYTE)pDataBase + (*pThunkINT));
				pApiName = (TCHAR *)pImportByName->Name;

				//获取函数地址
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
		//后移
		pImportDirectory++;
	}

	//执行的代码	  无返回值
	//**********************************************************************************************************
	//while(true)
	//{
	MessageBox(NULL, TEXT("HiddenModuleInjectDll !"), TEXT("Success"), MB_OK);
	//	Sleep(3000);
	//}
	//************************************************************************************************************
}

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&   加 载 进 程 注 入   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


DWORD LoadProcessInject(HWND hwndInject)
{
	DWORD ret = 0;


	MessageBox(NULL, TEXT("该方法没有实现！"), TEXT("ERROR"), MB_OK);
	return ret;
	//获取要载入文件的路径
	if (strlen(szPePath) == 0)
	{
		ret = -22;
		MessageBox(NULL, TEXT("请输入EXE文件路径"), TEXT("ERROR"), MB_OK);
		return ret;
	}


	return ret;
}
