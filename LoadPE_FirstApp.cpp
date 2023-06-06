// LoadPE_FirstApp.cpp : Defines the entry point for the application.
//
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include "resource.h"
#include "stdio.h"
#include "PeOperator.h"
#include "stdlib.h"
#include <time.h>
#include "Shell.h"
#include "Inject.h"


/*
Windows通用控件,代码包含在Comctrl32.dll				
使用前必须包含				
 */
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#include <Commdlg.h>
#pragma comment(lib, "Comdlg32.lib")

//遍历进程API
#include <Tlhelp32.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib ")


LPVOID NewBase;
DWORD dwImageSize;


HINSTANCE hAppInstance;				//本程序的实例句柄
TCHAR szFileName[MAX_PATH] = {0};	//打开文件的完整文件名


TCHAR szExeName[MAX_PATH] = {0};	//需要加壳的EXE 完整文件名
TCHAR szShellName[MAX_PATH] = {0};	//壳子程序的完整文件名
TCHAR szNewExeName[MAX_PATH] = {0};	//需要加壳后的EXE 完整文件名


TCHAR szProcessID[MAX_PATH] = {0};//待注入进程的PID
TCHAR szPePath[MAX_PATH] = {0};//待注入的DLL的路径


//初始化函数
VOID InitProcessListView(HWND hwndDlg);		
VOID InitMoudelListView(HWND hwndDlg);
VOID InitSectionTableListView(HWND hwndDlg);
VOID InitPeEdit(HWND hwndEditor,LPSTR szFileName);
VOID InitDirectoryTable(HWND hwndDirectory,LPSTR szFileName);

VOID InitExprotInFormation(HWND hwndDirectoryDetail);		   
VOID InitTableListView(HWND hwndDirectoryDetail);

VOID InitDllListView(HWND hwndDirectoryDetail);
VOID InitApiListView(HWND hwndDirectoryDetail);

VOID InitBlocksListView(HWND hwndDirectoryDetail);
VOID InitBlockItemListView(HWND hwndDirectoryDetail);

VOID InitResourceTreeView(HWND hwndDirectoryDetail);
VOID InitResourceEditInfo(HWND hwndTreeView, LPNMHDR lParam);//未完待续。。。。（无法获取被选中的节点信息）

VOID InitBoundImportTreeView(HWND hwndDirectoryDetail);

VOID InitDebugTableList(HWND hwndDirectoryDetail);

VOID InitTlsList(HWND hwndDirectoryDetail);


//遍历系统进程
VOID EnumProcess(HWND hListProcess);
//遍历进程模块
VOID EnumMoudle(HWND hListProcess, HWND hListModule);
//遍历节区表
VOID EnumSectionTable(HWND hListSectionTable);
//遍历Dll名
VOID EnumDllName(HWND hListDllList);
//遍历Api名
VOID EnumApiName(HWND hListDllList, HWND hListApiList);		   //在导入表
//遍历function表信息
VOID EnumFuncInformation(HWND hListTableList);
//遍历Blocks
VOID EnumBlocks(HWND hListBlocksList);
//遍历BlockItem
VOID EnumBlockItem(HWND hListBlocksList, HWND hListBlockItemList);
//遍历DebugTable
VOID EnumDebugTable(HWND hListDebugTable);


/////////////////////////////////////////////////////
//主页面的消息处理函数
BOOL CALLBACK DialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

BOOL CALLBACK EditorProc(
  HWND hwndEditor,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

BOOL CALLBACK AboutProc(
  HWND hwndAbout,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

BOOL CALLBACK DirectoryProc(
  HWND hwndDirectory,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

BOOL CALLBACK SectionProc(
  HWND hwndSection,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

BOOL CALLBACK ImprotTableDetailProc(
  HWND hwndDirectoryDetail,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

BOOL CALLBACK ExprotTableDetailProc(
  HWND hwndDirectoryDetail,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

BOOL CALLBACK RelocationTableDetailProc(
  HWND hwndDirectoryDetail,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

BOOL CALLBACK ResourceTableDetailProc(
  HWND hwndDirectoryDetail,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

BOOL CALLBACK BoundImportDetailProc(
  HWND hwndDirectoryDetail,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

BOOL CALLBACK DebugDetailProc(
  HWND hwndDirectoryDetail,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

BOOL CALLBACK TlsDetailProc(
  HWND hwndDirectoryDetail,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

/////////////////////////////////////////////////////////////////////
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
/////////////////////////////////////////////////////////////////////


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	//通用控件在使用前，需要通过INITCOMMONCONTROLSEX进行初始化								
	//只要在您的程序中的任意地方引用了该函数就、会使得WINDOWS的程序加载器PE Loader加载该库				
				
	INITCOMMONCONTROLSEX icex;				
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);				
	icex.dwICC = ICC_WIN95_CLASSES;				
	InitCommonControlsEx(&icex);				

	hAppInstance = hInstance;

	DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN) , NULL, DialogProc);
	return 0;
}

////////////////////////////////////////////////////////////////////////////
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch(uMsg)
	{
	//右上角的八叉按钮
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return true;

	//初始化Dialog
	case WM_INITDIALOG:
		{
			//插入图标
			HICON hBigIcon;
			HICON hSmallIcon;
			hBigIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_ICON_Big));
			hSmallIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_ICON_Small));
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (DWORD)hBigIcon);
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (DWORD)hSmallIcon);

			//初始化ListView
			InitProcessListView(hwndDlg);//设置ProcessListView
			InitMoudelListView(hwndDlg);//设置MoudelListView
			return true;
		}
	case WM_NOTIFY:
		{
			NMHDR *pNMHDR = (NMHDR *)lParam;
			if(wParam == IDC_LIST_Process && pNMHDR->code == NM_CLICK/*The user has clicked the left mouse button within the control*/)
			{
				EnumMoudle(GetDlgItem(hwndDlg, IDC_LIST_Process), GetDlgItem(hwndDlg, IDC_LIST_Moudle));
			}
			return true;
		}

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_PE_Edit:
				{
					//设置打开文件
					OPENFILENAME stOpenFile;
					//文件过滤器
					TCHAR szPeFileExt[200] =	"(*.exe)\0*.exe\0(*.dll)\0*.dll\0\
												(*.scr)\0*.scr\0(*.drv)\0*.drv\0(*.sys)\0*.sys\0";
												//设定过滤器要用空字符结尾\0   格式：(*.exe)\0*.exe\0
					
					memset(szFileName, 0, MAX_PATH);
					memset(&stOpenFile, 0, sizeof(OPENFILENAME));

					stOpenFile.lStructSize = sizeof(OPENFILENAME);
					stOpenFile.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
					stOpenFile.hwndOwner = hwndDlg;
					stOpenFile.lpstrFilter = szPeFileExt;
					stOpenFile.lpstrFile = szFileName;
					stOpenFile.nMaxFile = MAX_PATH;//最少256字节/字符
					
					//获取文件路径
					//The GetOpenFileName function creates an Open dialog box that lets the 
					//user specify the drive, directory, and the name of a file or set of files to open.
					if(GetOpenFileName(&stOpenFile))
					{
						 
						//成功选择文件 提醒文件路径，打开新的窗口
						MessageBox(NULL, szFileName, TEXT("FilePath"), MB_OK);
						
						DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_Editor) , hwndDlg, EditorProc);
					}
					else
					{
						//没有选择文件
						MessageBox(NULL, TEXT("Open File Failed"), TEXT("Error"), MB_OK);
					}										
					return true;
				}
			case IDC_BUTTON_About:
				//打开新的对话框
				DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_About), hwndDlg, AboutProc);
				return true;

			case IDC_BUTTON_Exit:
				EndDialog(hwndDlg, 0);
				return true;

			case IDC_BUTTON_Create_Shell:
				DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_CreateShell), hwndDlg, ShellProc);
				return true;

			case IDC_BUTTON_Inject_DLL:
				DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_Inject), hwndDlg, InjectProc);
				return true;
			}
			return true;
		}
	}
	return false;
}

BOOL CALLBACK EditorProc(
  HWND hwndEditor,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		InitPeEdit(hwndEditor, szFileName);
		return true;
		
	//右上角的八叉按钮
	case WM_CLOSE:
		EndDialog(hwndEditor, 0);
		return true;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_Sections:
				DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_SectionTable) , hwndEditor, SectionProc);
				return true;

			case IDC_BUTTON_Directories:
				DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_DirectoryTable) , hwndEditor, DirectoryProc);
				return true;

			case IDC_BUTTON_Close:
				EndDialog(hwndEditor, 0);
				return true;
			}
		}
	}
	return false;
}

BOOL CALLBACK AboutProc(
  HWND hwndAbout,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch(uMsg)
	{
	//右上角的八叉按钮
	case WM_CLOSE:
		EndDialog(hwndAbout, 0);
		return true;
	}
	return false;
}

BOOL CALLBACK DirectoryProc(
  HWND hwndDirectory,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	DWORD ret = 0;
	//文件读取	
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if(ret != 0)
	{
		MessageBox(hwndDirectory, TEXT("File Read Error !"), TEXT("Error"), MB_OK);
		return ret;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	switch(uMsg)
	{
	case WM_INITDIALOG:
		InitDirectoryTable(hwndDirectory, szFileName);
		return true;

	//右上角的八叉按钮
	case WM_CLOSE:
		EndDialog(hwndDirectory, 0);
		return true;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_OK:
				EndDialog(hwndDirectory, 0);
				free(pFileAdd);
				return true;
			case IDC_BUTTON_ExportTable_Detail:
				//判断是否存在
				if(pNTHeader->OptionalHeader.DataDirectory[0].VirtualAddress == 0)
				{
					MessageBox(NULL, TEXT("ExprotTable is not exist!"), TEXT("Error"), MB_OK);
					free(pFileAdd);
					return true;
				}
				else
				{
					DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_ExportTable_Detail), hwndDirectory, ExprotTableDetailProc);
					return true;
				}

			case IDC_BUTTON_ImprotTable_Detail:
				//判断是否存在
				if(pNTHeader->OptionalHeader.DataDirectory[1].VirtualAddress == 0)
				{
					MessageBox(NULL, TEXT("ImprotTable is not exist!"), TEXT("Error"), MB_OK);
					free(pFileAdd);
					return true;
				}
				else
				{
					DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_ImportTable_Detail), hwndDirectory, ImprotTableDetailProc);
					return true;
				}
			case IDC_BUTTON_Relocation_Detail:
				//判断是否存在
				if(pNTHeader->OptionalHeader.DataDirectory[5].VirtualAddress == 0)
				{
					MessageBox(NULL, TEXT("RelocationTable is not exist!"), TEXT("Error"), MB_OK);
					free(pFileAdd);
					return true;
				}
				else
				{
					DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_RelocationTable_Detail), hwndDirectory, RelocationTableDetailProc);
					return true;
				}
			case IDC_BUTTON_ResourceTable_Detail:
				if(pNTHeader->OptionalHeader.DataDirectory[2].VirtualAddress == 0)
				{
					MessageBox(NULL, TEXT("ResourceTable is not exist!"), TEXT("Error"), MB_OK);
					free(pFileAdd);
					return true;
				}
				else
				{
					DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_ResourceTable_Detail), hwndDirectory, ResourceTableDetailProc);
					return true;
				}
			case IDC_BUTTON_BoundImport_Detail:
				if(pNTHeader->OptionalHeader.DataDirectory[11].VirtualAddress == 0)
				{
					MessageBox(NULL, TEXT("BoundImport is not exist!"), TEXT("Error"), MB_OK);
					free(pFileAdd);
					return true;
				}
				else
				{
					DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_BoundImport_Detail), hwndDirectory, BoundImportDetailProc);
					return true;
				}
			case IDC_BUTTON_Debug_Detail:
				if(pNTHeader->OptionalHeader.DataDirectory[12].VirtualAddress == 0)
				{
					MessageBox(NULL, TEXT("DebugTable is not exist!"), TEXT("Error"), MB_OK);
					free(pFileAdd);
					return true;
				}
				else
				{
					DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_DebugTable_Detail), hwndDirectory, DebugDetailProc);
					return true;
				}
			case IDC_BUTTON_TlsTable_Detail:
				if(pNTHeader->OptionalHeader.DataDirectory[9].VirtualAddress == 0)
				{
					MessageBox(NULL, TEXT("TlsTable is not exist!"), TEXT("Error"), MB_OK);
					free(pFileAdd);
					return true;
				}
				else
				{
					DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_TLS_Detail), hwndDirectory, TlsDetailProc);
					return true;
				}
			case IDC_BUTTON_Copyright_Detail:
				if(pNTHeader->OptionalHeader.DataDirectory[7].VirtualAddress == 0)
				{
					MessageBox(NULL, TEXT("DebugTable is not exist!"), TEXT("Error"), MB_OK);
					free(pFileAdd);
					return true;
				}
				else
				{
					DWORD CopyrightAddress_RVA = pNTHeader->OptionalHeader.DataDirectory[7].VirtualAddress;
					DWORD CopyrightAddress_FOA = 0;
					RVA_TO_FOA(CopyrightAddress_RVA, &CopyrightAddress_FOA, pFileAdd);

					TCHAR *szCopyright = (TCHAR *)((PBYTE)pFileAdd + CopyrightAddress_FOA);

					MessageBox(NULL, szCopyright, TEXT("Copyright"), MB_OK);
					free(pFileAdd);
					return true;
				}
			}
		}
		free(pFileAdd);
		return true;
	}
	free(pFileAdd);
	return false;
}

BOOL CALLBACK SectionProc(
  HWND hwndSection,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch(uMsg)
	{
	//右上角的八叉按钮
	case WM_CLOSE:
		EndDialog(hwndSection, 0);
		return true;
	
	//初始化Dialog
	case WM_INITDIALOG:
		InitSectionTableListView(hwndSection);//设置ListView
		return true;
	}
	return false;
}


BOOL CALLBACK ImprotTableDetailProc(
  HWND hwndDirectoryDetail,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch(uMsg)
	{
	case WM_NOTIFY:
	{
		NMHDR *pNMHDR = (NMHDR *)lParam;
		if(wParam == IDC_LIST_DLL_List && pNMHDR->code == NM_CLICK/*The user has clicked the left mouse button within the control*/)
		{
			EnumApiName(GetDlgItem(hwndDirectoryDetail, IDC_LIST_DLL_List), GetDlgItem(hwndDirectoryDetail, IDC_LIST_API_List));
		}
		return true;
	}
	case WM_INITDIALOG:
		InitDllListView(hwndDirectoryDetail);
		InitApiListView(hwndDirectoryDetail);
		return true;
	case WM_CLOSE:
		EndDialog(hwndDirectoryDetail, 0);
		return true; 
	}

	return DefWindowProc(hwndDirectoryDetail, uMsg, wParam, lParam);
}

BOOL CALLBACK ExprotTableDetailProc(
  HWND hwndDirectoryDetail,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		InitExprotInFormation(hwndDirectoryDetail);
		InitTableListView(hwndDirectoryDetail);
		return true;
	case WM_CLOSE:
		EndDialog(hwndDirectoryDetail, 0);
		return true;
	}
	return DefWindowProc(hwndDirectoryDetail, uMsg, wParam, lParam);
}


BOOL CALLBACK RelocationTableDetailProc(
  HWND hwndDirectoryDetail,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch(uMsg)
	{
	case WM_NOTIFY:
	{
		NMHDR *pNMHDR = (NMHDR *)lParam;
		if(wParam == IDC_LIST_Blocks && pNMHDR->code == NM_CLICK/*The user has clicked the left mouse button within the control*/)
		{
			EnumBlockItem(GetDlgItem(hwndDirectoryDetail, IDC_LIST_Blocks), GetDlgItem(hwndDirectoryDetail, IDC_LIST_BlockItem));
		}
		return true;
	}
	case WM_CLOSE:
		EndDialog(hwndDirectoryDetail, 0);
		return true;
	case WM_INITDIALOG:
		InitBlocksListView(hwndDirectoryDetail);
		InitBlockItemListView(hwndDirectoryDetail);
		return true;
	}
	return DefWindowProc(hwndDirectoryDetail, uMsg, wParam, lParam);
}

BOOL CALLBACK ResourceTableDetailProc(
  HWND hwndDirectoryDetail,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		InitResourceTreeView(hwndDirectoryDetail);
		return 0;

	case WM_NOTIFY:
		{
			NMHDR *pNMHDR = (NMHDR *)lParam;

			if(wParam == IDC_TREE_Resource && (pNMHDR->code == NM_CLICK || pNMHDR->code == NM_DBLCLK)/*The user has clicked the left mouse button within the control 单击或双击*/)
			{
				TCHAR szBuffer[30] = {0};

				//InitResourceEditInfo(GetDlgItem(hwndDirectoryDetail, IDC_TREE_Resource), pNMHDR);
			}
		return true;
		}
	case WM_CLOSE:
		EndDialog(hwndDirectoryDetail, 0);
		return true;
	}
	return DefWindowProc(hwndDirectoryDetail, uMsg, wParam, lParam);
}

BOOL CALLBACK BoundImportDetailProc(
  HWND hwndDirectoryDetail,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		InitBoundImportTreeView(hwndDirectoryDetail);
		return 0;
	case WM_CLOSE:
		EndDialog(hwndDirectoryDetail, 0);
		return true;
	}
	return DefWindowProc(hwndDirectoryDetail, uMsg, wParam, lParam);
}

BOOL CALLBACK DebugDetailProc(
  HWND hwndDirectoryDetail,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		InitDebugTableList(hwndDirectoryDetail);
		return 0;
	case WM_CLOSE:
		EndDialog(hwndDirectoryDetail, 0);
		return true;
	}
	return DefWindowProc(hwndDirectoryDetail, uMsg, wParam, lParam);
}

BOOL CALLBACK TlsDetailProc(
  HWND hwndDirectoryDetail,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		InitTlsList(hwndDirectoryDetail);
		return true;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDOK:
				{
					EndDialog(hwndDirectoryDetail, 0);
					return true;
				}
			}
			return true;
		}
	case WM_CLOSE:
		EndDialog(hwndDirectoryDetail, 0);
		return true;
	}
	return DefWindowProc(hwndDirectoryDetail, uMsg, wParam, lParam);
}


////////////////////////////////////////////////////////////////////////////
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@//
////////////////////////////////////////////////////////////////////////////

VOID InitProcessListView(HWND hwndDlg)
{
	LV_COLUMN lv;
	HWND hListProcess;

	//初始化
	memset(&lv, 0, sizeof(LV_COLUMN));
	//获取IDC_LIST_Process的句柄
	hListProcess = GetDlgItem(hwndDlg,  IDC_LIST_Process);
	//设置整行选中   点击某一行该行全部选中
	SendMessage(hListProcess, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	//第一列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("No.");//列标题
	lv.cx = 40;//列宽
	lv.iSubItem = 0;//列号
	//SendMessage(hListProcess,LVM_INSERTCOLUMN,0,(DWORD)&lv);
	ListView_InsertColumn(hListProcess, 0, &lv);

	//第2列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Process");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 1;//列号
	//SendMessage(hListProcess,LVM_INSERTCOLUMN,0,(DWORD)&lv);
	ListView_InsertColumn(hListProcess, 1, &lv);
	
	//第3列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("PID");//列标题
	lv.cx = 120;//列宽
	lv.iSubItem = 2;//列号
	ListView_InsertColumn(hListProcess, 2, &lv);

	//第4列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("ImageBase");//列标题
	lv.cx = 120;//列宽
	lv.iSubItem = 3;//列号
	ListView_InsertColumn(hListProcess, 3, &lv);

	//第5列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("SizeOfImage");//列标题
	lv.cx = 120;//列宽
	lv.iSubItem = 4;//列号
	ListView_InsertColumn(hListProcess, 4, &lv);

	//添加进程到LVM_INSERTCOLUMN
	EnumProcess(hListProcess);
}

VOID InitMoudelListView(HWND hwndDlg)
{
	LV_COLUMN lv;
	HWND hListMoudel;

	//初始化
	memset(&lv, 0, sizeof(LV_COLUMN));
	//获取IDC_LIST_Moudle的句柄
	hListMoudel = GetDlgItem(hwndDlg,  IDC_LIST_Moudle);
	//设置整行选中   点击某一行该行全部选中
	SendMessage(hListMoudel, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	//第一列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("MoudelName");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 0;//列号
	//SendMessage(hListProcess,LVM_INSERTCOLUMN,0,(DWORD)&lv);
	ListView_InsertColumn(hListMoudel, 1, &lv);
	
	//第二列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("MoudelLocation");//列标题
	lv.cx = 300;//列宽
	lv.iSubItem = 1;//列号
	ListView_InsertColumn(hListMoudel, 1, &lv);

}

VOID InitPeEdit(HWND hwndEditor,LPSTR szFileName)
{
	DWORD ret = 0;
	
	//文件读取	
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if(ret != 0)
	{
		MessageBox(hwndEditor, TEXT("File Read Error !"), TEXT("Error"), MB_OK);
		return;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的MZ标志"),TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	//判断是否是有效的PE 
	if(pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的PE标志"),TEXT("Error"),MB_OK);
		free(pFileAdd);
		return;
	}

	//向编辑框发送消息
	//1、获得编辑框句柄
	//2、将数字转化成字符串
	//3、发送消息
	HWND hEntryPoint = GetDlgItem(hwndEditor, IDC_EDIT_EntryPoint);
	TCHAR tEntryPoint[10] = {0};
	sprintf(tEntryPoint, "%08X", pNTHeader->OptionalHeader.AddressOfEntryPoint);
	SendMessage(hEntryPoint, WM_SETTEXT, 0, (long)tEntryPoint);

	HWND hImageBase = GetDlgItem(hwndEditor, IDC_EDIT_ImageBase);
	TCHAR tImageBase[10] = {0};
	sprintf(tImageBase, "%08X", pNTHeader->OptionalHeader.ImageBase);
	SendMessage(hImageBase, WM_SETTEXT, 0, (long)tImageBase);
	
	HWND hSizeOfImage = GetDlgItem(hwndEditor, IDC_EDIT_SizeOfImage);
	TCHAR tSizeOfImage[10] = {0};
	sprintf(tSizeOfImage, "%08X", pNTHeader->OptionalHeader.SizeOfImage);
	SendMessage(hSizeOfImage , WM_SETTEXT, 0, (long)tSizeOfImage );
	
	HWND hBaseOfCode = GetDlgItem(hwndEditor, IDC_EDIT_BaseOfCode);
	TCHAR tBaseOfCode[10] = {0};
	sprintf(tBaseOfCode, "%08X", pNTHeader->OptionalHeader.BaseOfCode);
	SendMessage(hBaseOfCode, WM_SETTEXT, 0, (long)tBaseOfCode);

	HWND hBaseOfData = GetDlgItem(hwndEditor, IDC_EDIT_BaseOfData);
	TCHAR tBaseOfData[10] = {0};
	sprintf(tBaseOfData, "%08X", pNTHeader->OptionalHeader.BaseOfData);
	SendMessage(hBaseOfData, WM_SETTEXT, 0, (long)tBaseOfData);

	HWND hSectionAlignment = GetDlgItem(hwndEditor, IDC_EDIT_SectionAlignment);
	TCHAR tSectionAlignment[10] = {0};
	sprintf(tSectionAlignment , "%08X", pNTHeader->OptionalHeader.SectionAlignment );
	SendMessage(hSectionAlignment , WM_SETTEXT, 0, (long)tSectionAlignment);
	
	HWND hFileAlignment = GetDlgItem(hwndEditor, IDC_EDIT_FileAlignment);
	TCHAR tFileAlignment[10] = {0};
	sprintf(tFileAlignment, "%08X", pNTHeader->OptionalHeader.FileAlignment);
	SendMessage(hFileAlignment , WM_SETTEXT, 0, (long)tFileAlignment );
	
	HWND hMagic = GetDlgItem(hwndEditor, IDC_EDIT_Magic);
	TCHAR tMagic[10] = {0};
	sprintf(tMagic, "%04X", pNTHeader->OptionalHeader.Magic);
	SendMessage(hMagic, WM_SETTEXT, 0, (long)tMagic);
	
	HWND hSubsystem = GetDlgItem(hwndEditor, IDC_EDIT_Subsystem);
	TCHAR tSubsystem[10] = {0};
	sprintf(tSubsystem, "%04X", pNTHeader->OptionalHeader.Subsystem);
	SendMessage(hSubsystem, WM_SETTEXT, 0, (long)tSubsystem);

	HWND hNumberOfSections = GetDlgItem(hwndEditor, IDC_EDIT_NumberOfSections);
	TCHAR tNumberOfSections[10] = {0};
	sprintf(tNumberOfSections, "%04X", pNTHeader->FileHeader.NumberOfSections);
	SendMessage(hNumberOfSections, WM_SETTEXT, 0, (long)tNumberOfSections);
	
	HWND hTimeDataStamp = GetDlgItem(hwndEditor, IDC_EDIT_TimeDataStamp);
	TCHAR tTimeDataStamp[10] = {0};
	sprintf(tTimeDataStamp, "%08X", pNTHeader->FileHeader.TimeDateStamp);
	SendMessage(hTimeDataStamp , WM_SETTEXT, 0, (long)tTimeDataStamp );
	
	HWND hSizeOfHeaders = GetDlgItem(hwndEditor, IDC_EDIT_SizeOfHeaders);
	TCHAR tSizeOfHeaders[10] = {0};
	sprintf(tSizeOfHeaders, "%08X", pNTHeader->OptionalHeader.SizeOfHeaders);
	SendMessage(hSizeOfHeaders, WM_SETTEXT, 0, (long)tSizeOfHeaders);

	HWND hCharacteristics = GetDlgItem(hwndEditor, IDC_EDIT_Characteristics);
	TCHAR tCharacteristics[10] = {0};
	sprintf(tCharacteristics, "%04X", pNTHeader->FileHeader.Characteristics);
	SendMessage(hCharacteristics, WM_SETTEXT, 0, (long)tCharacteristics);

	HWND hChecksum = GetDlgItem(hwndEditor, IDC_EDIT_Checksum);
	TCHAR tChecksum[10] = {0};
	sprintf(tChecksum , "%08X", pNTHeader->OptionalHeader.CheckSum);
	SendMessage(hChecksum , WM_SETTEXT, 0, (long)tChecksum);
	
	HWND hSizeOfOptionalHeader = GetDlgItem(hwndEditor, IDC_EDIT_SizeOfOptionalHeader);
	TCHAR tSizeOfOptionalHeader[10] = {0};
	sprintf(tSizeOfOptionalHeader, "%04X", pNTHeader->FileHeader.SizeOfOptionalHeader);
	SendMessage(hSizeOfOptionalHeader , WM_SETTEXT, 0, (long)tSizeOfOptionalHeader );
	
	HWND hNumberOfRvaAndSize = GetDlgItem(hwndEditor, IDC_EDIT_NumberOfRvaAndSize);
	TCHAR tNumberOfRvaAndSize[10] = {0};
	sprintf(tNumberOfRvaAndSize, "%08X", pNTHeader->OptionalHeader.NumberOfRvaAndSizes);
	SendMessage(hNumberOfRvaAndSize, WM_SETTEXT, 0, (long)tNumberOfRvaAndSize);

	//释放内存
	free(pFileAdd);
	return;
}

VOID InitDirectoryTable(HWND hwndDirectory,LPSTR szFileName)
{
	DWORD ret = 0;
	
	//文件读取	
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if(ret != 0)
	{
		MessageBox(hwndDirectory, TEXT("File Read Error !"), TEXT("Error"), MB_OK);
		return;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的MZ标志"),TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	//判断是否是有效的PE 
	if(pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的PE标志"),TEXT("Error"),MB_OK);
		free(pFileAdd);
		return;
	}

	//向编辑框发送消息
	//1、获得编辑框句柄
	//2、将数字转化成字符串
	//3、发送消息
	HWND hExportTable_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_ExportTable_RAV);
	TCHAR tExportTable_RAV[10] = {0};
	sprintf(tExportTable_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[0].VirtualAddress);
	SendMessage(hExportTable_RAV, WM_SETTEXT, 0, (long)tExportTable_RAV);

	HWND hExportTable_Size = GetDlgItem(hwndDirectory, IDC_EDIT_ExportTable_Size);
	TCHAR tExportTable_Size[10] = {0};
	sprintf(tExportTable_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[0].Size);
	SendMessage(hExportTable_Size, WM_SETTEXT, 0, (long)tExportTable_Size);

  	HWND hImportTable_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_ImprotTable_RAV);
	TCHAR tImportTable_RAV[10] = {0};
	sprintf(tImportTable_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[1].VirtualAddress);
	SendMessage(hImportTable_RAV, WM_SETTEXT, 0, (long)tImportTable_RAV);

	HWND hImportTable_Size = GetDlgItem(hwndDirectory, IDC_EDIT_ImprotTable_Size);
	TCHAR tImportTable_Size[10] = {0};
	sprintf(tImportTable_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[1].Size);
	SendMessage(hImportTable_Size, WM_SETTEXT, 0, (long)tImportTable_Size);
		
	HWND hResource_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_Resource_RAV);
	TCHAR tResource_RAV[10] = {0};
	sprintf(tResource_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[2].VirtualAddress);
	SendMessage(hResource_RAV, WM_SETTEXT, 0, (long)tResource_RAV);

	HWND hResource_Size = GetDlgItem(hwndDirectory, IDC_EDIT_Resource_Size);
	TCHAR tResource_Size[10] = {0};
	sprintf(tResource_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[2].Size);
	SendMessage(hResource_Size, WM_SETTEXT, 0, (long)tResource_Size);
	
	HWND hException_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_Exception_RAV);
	TCHAR tException_RAV[10] = {0};
	sprintf(tException_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[3].VirtualAddress);
	SendMessage(hException_RAV, WM_SETTEXT, 0, (long)tException_RAV);

	HWND hException_Size = GetDlgItem(hwndDirectory, IDC_EDIT_Exception_Size);
	TCHAR tException_Size[10] = {0};
	sprintf(tException_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[3].Size);
	SendMessage(hException_Size, WM_SETTEXT, 0, (long)tException_Size);

	HWND hSecurity_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_Security_RAV);
	TCHAR tSecurity_RAV[10] = {0};
	sprintf(tSecurity_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[4].VirtualAddress);
	SendMessage(hSecurity_RAV, WM_SETTEXT, 0, (long)tSecurity_RAV);

	HWND hSecurity_Size = GetDlgItem(hwndDirectory, IDC_EDIT_Security_Size);
	TCHAR tSecurity_Size[10] = {0};
	sprintf(tSecurity_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[4].Size);
	SendMessage(hSecurity_Size, WM_SETTEXT, 0, (long)tSecurity_Size);

	HWND hRelocation_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_Relocation_RAV);
	TCHAR tRelocation_RAV[10] = {0};
	sprintf(tRelocation_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[5].VirtualAddress);
	SendMessage(hRelocation_RAV, WM_SETTEXT, 0, (long)tRelocation_RAV);

	HWND hRelocation_Size = GetDlgItem(hwndDirectory, IDC_EDIT_Relocation_Size);
	TCHAR tRelocation_Size[10] = {0};
	sprintf(tRelocation_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[5].Size);
	SendMessage(hRelocation_Size, WM_SETTEXT, 0, (long)tRelocation_Size);

	HWND hDebug_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_Debug_RAV);
	TCHAR tDebug_RAV[10] = {0};
	sprintf(tDebug_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[6].VirtualAddress);
	SendMessage(hDebug_RAV, WM_SETTEXT, 0, (long)tDebug_RAV);

	HWND hDebug_Size = GetDlgItem(hwndDirectory, IDC_EDIT_Debug_Size);
	TCHAR tDebug_Size[10] = {0};
	sprintf(tDebug_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[6].Size);
	SendMessage(hDebug_Size, WM_SETTEXT, 0, (long)tDebug_Size);

	HWND hCopyright_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_Copyright_RAV);
	TCHAR tCopyright_RAV[10] = {0};
	sprintf(tCopyright_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[7].VirtualAddress);
	SendMessage(hCopyright_RAV, WM_SETTEXT, 0, (long)tCopyright_RAV);

	HWND hCopyright_Size = GetDlgItem(hwndDirectory, IDC_EDIT_Copyright_Size);
	TCHAR tCopyright_Size[10] = {0};
	sprintf(tCopyright_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[7].Size);
	SendMessage(hCopyright_Size, WM_SETTEXT, 0, (long)tCopyright_Size);

	HWND hGlobalptr_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_Globalptr_RAV);
	TCHAR tGlobalptr_RAV[10] = {0};
	sprintf(tGlobalptr_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[8].VirtualAddress);
	SendMessage(hGlobalptr_RAV, WM_SETTEXT, 0, (long)tGlobalptr_RAV);

	HWND hGlobalptr_Size = GetDlgItem(hwndDirectory, IDC_EDIT_Globalptr_Size);
	TCHAR tGlobalptr_Size[10] = {0};
	sprintf(tGlobalptr_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[8].Size);
	SendMessage(hGlobalptr_Size, WM_SETTEXT, 0, (long)tGlobalptr_Size);

	HWND hTlsTable_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_TlsTable_RAV);
	TCHAR tTlsTable_RAV[10] = {0};
	sprintf(tTlsTable_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[9].VirtualAddress);
	SendMessage(hTlsTable_RAV, WM_SETTEXT, 0, (long)tTlsTable_RAV);

	HWND hTlsTable_Size = GetDlgItem(hwndDirectory, IDC_EDIT_TlsTable_Size);
	TCHAR tTlsTable_Size[10] = {0};
	sprintf(tTlsTable_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[9].Size);
	SendMessage(hTlsTable_Size, WM_SETTEXT, 0, (long)tTlsTable_Size);

	HWND hLoadConfig_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_LoadConfig_RAV);
	TCHAR tLoadConfig_RAV[10] = {0};
	sprintf(tLoadConfig_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[10].VirtualAddress);
	SendMessage(hLoadConfig_RAV, WM_SETTEXT, 0, (long)tLoadConfig_RAV);

	HWND hLoadConfig_Size = GetDlgItem(hwndDirectory, IDC_EDIT_LoadConfig_Size);
	TCHAR tLoadConfig_Size[10] = {0};
	sprintf(tLoadConfig_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[10].Size);
	SendMessage(hLoadConfig_Size, WM_SETTEXT, 0, (long)tLoadConfig_Size);

	HWND hBoundImport_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_BoundImport_RAV);
	TCHAR tBoundImport_RAV[10] = {0};
	sprintf(tBoundImport_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[11].VirtualAddress);
	SendMessage(hBoundImport_RAV, WM_SETTEXT, 0, (long)tBoundImport_RAV);

	HWND hBoundImport_Size = GetDlgItem(hwndDirectory, IDC_EDIT_BoundImport_Size);
	TCHAR tBoundImport_Size[10] = {0};
	sprintf(tBoundImport_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[11].Size);
	SendMessage(hBoundImport_Size, WM_SETTEXT, 0, (long)tBoundImport_Size);

	HWND hIAT_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_IAT_RAV);
	TCHAR tIAT_RAV[10] = {0};
	sprintf(tIAT_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[12].VirtualAddress);
	SendMessage(hIAT_RAV, WM_SETTEXT, 0, (long)tIAT_RAV);

	HWND hIAT_Size = GetDlgItem(hwndDirectory, IDC_EDIT_IAT_Size);
	TCHAR tIAT_Size[10] = {0};
	sprintf(tIAT_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[12].Size);
	SendMessage(hIAT_Size, WM_SETTEXT, 0, (long)tIAT_Size);

	HWND hDelayImport_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_DelayImport_RAV);
	TCHAR tDelayImport_RAV[10] = {0};
	sprintf(tDelayImport_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[13].VirtualAddress);
	SendMessage(hDelayImport_RAV, WM_SETTEXT, 0, (long)tLoadConfig_RAV);

	HWND hDelayImport_Size = GetDlgItem(hwndDirectory, IDC_EDIT_DelayImport_Size);
	TCHAR tDelayImport_Size[10] = {0};
	sprintf(tDelayImport_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[13].Size);
	SendMessage(hDelayImport_Size, WM_SETTEXT, 0, (long)tDelayImport_Size);

	HWND hCOM_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_COM_RAV);
	TCHAR tCOM_RAV[10] = {0};
	sprintf(tCOM_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[14].VirtualAddress);
	SendMessage(hCOM_RAV, WM_SETTEXT, 0, (long)tCOM_RAV);

	HWND hCOM_Size = GetDlgItem(hwndDirectory, IDC_EDIT_COM_Size);
	TCHAR tCOM_Size[10] = {0};
	sprintf(tCOM_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[14].Size);
	SendMessage(hCOM_Size, WM_SETTEXT, 0, (long)tCOM_Size);

	HWND hReserved_RAV = GetDlgItem(hwndDirectory, IDC_EDIT_Reserved_RAV);
	TCHAR tReserved_RAV[10] = {0};
	sprintf(tReserved_RAV, "%08X", pNTHeader->OptionalHeader.DataDirectory[15].VirtualAddress);
	SendMessage(hReserved_RAV, WM_SETTEXT, 0, (long)tReserved_RAV);

	HWND hReserved_Size = GetDlgItem(hwndDirectory, IDC_EDIT_Reserved_Size);
	TCHAR tReserved_Size[10] = {0};
	sprintf(tReserved_Size, "%08X", pNTHeader->OptionalHeader.DataDirectory[15].Size);
	SendMessage(hReserved_Size, WM_SETTEXT, 0, (long)tReserved_Size);

	free(pFileAdd);

	return;
}

VOID InitSectionTableListView(HWND hwndDlg)
{
	LV_COLUMN lv;
	HWND hListSectionTable;

	//初始化
	memset(&lv, 0, sizeof(LV_COLUMN));
	//获取IDC_LIST_Process的句柄
	hListSectionTable = GetDlgItem(hwndDlg,  IDC_LIST_SectionTable);
	//设置整行选中   点击某一行该行全部选中
	SendMessage(hListSectionTable, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);


	//第一列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Name");//列标题
	lv.cx = 80;//列宽
	lv.iSubItem = 0;//列号
	//SendMessage(hListProcess,LVM_INSERTCOLUMN,0,(DWORD)&lv);
	ListView_InsertColumn(hListSectionTable, 0, &lv);
	
	//第二列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("VirtualAddress");//列标题
	lv.cx = 120;//列宽
	lv.iSubItem = 1;//列号
	ListView_InsertColumn(hListSectionTable, 1, &lv);

	//第三列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("VirtualSize");//列标题
	lv.cx = 120;//列宽
	lv.iSubItem = 2;//列号
	ListView_InsertColumn(hListSectionTable, 2, &lv);

	//第四列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("PointerToRawData");//列标题
	lv.cx = 120;//列宽
	lv.iSubItem = 3;//列号
	ListView_InsertColumn(hListSectionTable, 3, &lv);

	//第五列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("SzieOfRawData");//列标题
	lv.cx = 120;//列宽
	lv.iSubItem = 4;//列号
	ListView_InsertColumn(hListSectionTable, 4, &lv);

	//第六列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Characteristics");//列标题
	lv.cx = 120;//列宽
	lv.iSubItem = 5;//列号
	ListView_InsertColumn(hListSectionTable, 5, &lv);

	EnumSectionTable(hListSectionTable);

}

VOID InitExprotInFormation(HWND hwndDirectoryDetail)
{
	DWORD ret = 0;
	//文件读取	
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if (ret != 0)
	{
		MessageBox(hwndDirectoryDetail, TEXT("File Read Error !"), TEXT("Error"), MB_OK);
		return;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(0, TEXT("不是有效的MZ标志"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	//判断是否是有效的PE 
	if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(0, TEXT("不是有效的PE标志"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}

	//定位导出表
	DWORD ExportDirectoryAdd_RVA = pNTHeader->OptionalHeader.DataDirectory[0].VirtualAddress;
	DWORD ExportDirectoryAdd_FOA = 0;
	ret = RVA_TO_FOA(ExportDirectoryAdd_RVA, &ExportDirectoryAdd_FOA, pFileAdd);
	if (ret != 0)
	{
		MessageBox(hwndDirectoryDetail, TEXT("Func InitExprotInFormation RVA_TO_FOA Error"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	PIMAGE_EXPORT_DIRECTORY	pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)pFileAdd + ExportDirectoryAdd_FOA);
	
	DWORD NameAddress_RVA = pExportDirectory->Name;
	DWORD NameAddress_FOA = 0;	
	ret = RVA_TO_FOA(NameAddress_RVA, &NameAddress_FOA, pFileAdd);
	if (ret != 0)
	{
		MessageBox(hwndDirectoryDetail, TEXT("Func InitExprotInFormation RVA_TO_FOA Error"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	TCHAR *szName = (TCHAR *)((PBYTE)pFileAdd + NameAddress_FOA);


	//向编辑框发送消息
	//1、获得编辑框句柄
	//2、将数字转化成字符串
	//3、发送消息
	HWND hExportTableOffset = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_ExportTable_Offset);
	TCHAR tExportTableOffset[10] = { 0 };
	sprintf(tExportTableOffset, "%08X", ExportDirectoryAdd_FOA);
	SendMessage(hExportTableOffset, WM_SETTEXT, 0, (long)tExportTableOffset);
	
	HWND hCharacteristics = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_Characteristics);
	TCHAR tCharacteristics[10] = { 0 };
	sprintf(tCharacteristics, "%08X", pExportDirectory->Characteristics);
	SendMessage(hCharacteristics, WM_SETTEXT, 0, (long)tCharacteristics);
	 
	HWND hBase = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_Base);
	TCHAR tBase[10] = { 0 };
	sprintf(tBase, "%08X", pExportDirectory->Base);
	SendMessage(hBase, WM_SETTEXT, 0, (long)tBase);

	HWND hName = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_Name);
	TCHAR tName[10] = { 0 };
	sprintf(tName, "%08X", pExportDirectory->Name);
	SendMessage(hName, WM_SETTEXT, 0, (long)tName);

	HWND hNameString = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_NameString);
	SendMessage(hNameString, WM_SETTEXT, 0, (long)szName);

	HWND hNumberOfFunctions = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_NumberOfFunctions);
	TCHAR tNumberOfFunctions[10] = { 0 };
	sprintf(tNumberOfFunctions, "%08X", pExportDirectory->NumberOfFunctions);
	SendMessage(hNumberOfFunctions, WM_SETTEXT, 0, (long)tNumberOfFunctions);

	HWND hNumberOfNames = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_NumberOfNames);
	TCHAR tNumberOfNames[10] = { 0 };
	sprintf(tNumberOfNames, "%08X", pExportDirectory->NumberOfNames);
	SendMessage(hNumberOfNames, WM_SETTEXT, 0, (long)tNumberOfNames);

	HWND hAddressOfFunctions = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_AddressOfFunctions);
	TCHAR tAddressOfFunctions[10] = { 0 };
	sprintf(tAddressOfFunctions, "%08X", pExportDirectory->AddressOfFunctions);
	SendMessage(hAddressOfFunctions, WM_SETTEXT, 0, (long)tAddressOfFunctions);

	HWND hAddressOfNames = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_AddressOfNames);
	TCHAR tAddressOfNames[10] = { 0 };
	sprintf(tAddressOfNames, "%08X", pExportDirectory->AddressOfNames);
	SendMessage(hAddressOfNames, WM_SETTEXT, 0, (long)tAddressOfNames);

	HWND hAddressOfNameOrdinals = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_AddressOfNameOrdinals);
	TCHAR tAddressOfNameOrdinals[10] = { 0 };
	sprintf(tAddressOfNameOrdinals, "%08X", pExportDirectory->AddressOfNameOrdinals);
	SendMessage(hAddressOfNameOrdinals, WM_SETTEXT, 0, (long)tAddressOfNameOrdinals);

	free(pFileAdd);
	return ;
}

VOID InitTableListView(HWND hwndDirectoryDetail)
{
	LV_COLUMN lv;
	HWND hListTableList;

	//初始化
	memset(&lv, 0, sizeof(LV_COLUMN));
	//获取IDC_LIST_DLL_List的句柄
	hListTableList = GetDlgItem(hwndDirectoryDetail,  IDC_LIST_TableList);
	//设置整行选中   点击某一行该行全部选中
	SendMessage(hListTableList, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	
	//第一列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Ordinal");//列标题
	lv.cx = 80;//列宽
	lv.iSubItem = 0;//列号
	//SendMessage(hListProcess,LVM_INSERTCOLUMN,0,(DWORD)&lv);
	ListView_InsertColumn(hListTableList, 0, &lv);
	
	//第二列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("FuncAdd_RVA");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 1;//列号
	ListView_InsertColumn(hListTableList, 1, &lv);

	//第三列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("FuncAdd_Offset/FOA");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 2;//列号
	ListView_InsertColumn(hListTableList, 2, &lv);

	//第四列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Function Name");//列标题
	lv.cx = 250;//列宽
	lv.iSubItem = 3;//列号
	ListView_InsertColumn(hListTableList, 3, &lv);

	EnumFuncInformation(hListTableList);

	return;
}

VOID InitDllListView(HWND hwndDirectoryDetail)
{
	LV_COLUMN lv;
	HWND hListDllList;

	//初始化
	memset(&lv, 0, sizeof(LV_COLUMN));
	//获取IDC_LIST_DLL_List的句柄
	hListDllList = GetDlgItem(hwndDirectoryDetail,  IDC_LIST_DLL_List);
	//设置整行选中   点击某一行该行全部选中
	SendMessage(hListDllList, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	
	//第一列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("DllName");//列标题
	lv.cx = 120;//列宽
	lv.iSubItem = 0;//列号
	//SendMessage(hListProcess,LVM_INSERTCOLUMN,0,(DWORD)&lv);
	ListView_InsertColumn(hListDllList, 0, &lv);
	
	//第二列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("OriginalFirstThunk");//列标题
	lv.cx = 140;//列宽
	lv.iSubItem = 1;//列号
	ListView_InsertColumn(hListDllList, 1, &lv);

	//第三列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("TimeDateStamp");//列标题
	lv.cx = 120;//列宽
	lv.iSubItem = 2;//列号
	ListView_InsertColumn(hListDllList, 2, &lv);

	//第四列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("ForwarderChain");//列标题
	lv.cx = 120;//列宽
	lv.iSubItem = 3;//列号
	ListView_InsertColumn(hListDllList, 3, &lv);

	//第五列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Name");//列标题
	lv.cx = 120;//列宽
	lv.iSubItem = 4;//列号
	ListView_InsertColumn(hListDllList, 4, &lv);

	//第六列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("FirstThunk");//列标题
	lv.cx = 120;//列宽
	lv.iSubItem = 5;//列号
	ListView_InsertColumn(hListDllList, 5, &lv);

	EnumDllName(hListDllList);

	return ;
}

VOID InitApiListView(HWND hwndDirectoryDetail)
{

	LV_COLUMN lv;
	HWND hListApiList;

	//初始化
	memset(&lv, 0, sizeof(LV_COLUMN));
	//获取IDC_LIST_DLL_List的句柄
	hListApiList = GetDlgItem(hwndDirectoryDetail,  IDC_LIST_API_List);
	//设置整行选中   点击某一行该行全部选中
	SendMessage(hListApiList, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	
	//第一列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("ThunkRVA");//列标题
	lv.cx = 100;//列宽
	lv.iSubItem = 0;//列号
	//SendMessage(hListProcess,LVM_INSERTCOLUMN,0,(DWORD)&lv);
	ListView_InsertColumn(hListApiList, 0, &lv);
	
	//第二列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("ThunkOffset");//列标题
	lv.cx = 100;//列宽
	lv.iSubItem = 1;//列号
	ListView_InsertColumn(hListApiList, 1, &lv);

	//第三列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("ThunkValue");//列标题
	lv.cx = 100;//列宽
	lv.iSubItem = 2;//列号
	ListView_InsertColumn(hListApiList, 2, &lv);

	//第四列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Hint");//列标题
	lv.cx = 60;//列宽
	lv.iSubItem = 3;//列号
	ListView_InsertColumn(hListApiList, 3, &lv);

	//第五列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("ApiName");//列标题
	lv.cx = 250;//列宽
	lv.iSubItem = 4;//列号
	ListView_InsertColumn(hListApiList, 4, &lv);

	return ;
}


VOID InitBlocksListView(HWND hwndDirectoryDetail)
{
	LV_COLUMN lv;
	HWND hListBlocksList;

	//初始化
	memset(&lv, 0, sizeof(LV_COLUMN));
	//获取IDC_LIST_DLL_List的句柄
	hListBlocksList = GetDlgItem(hwndDirectoryDetail,  IDC_LIST_Blocks);
	//设置整行选中   点击某一行该行全部选中
	SendMessage(hListBlocksList, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	
	//第一列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Index");//列标题
	lv.cx = 80;//列宽
	lv.iSubItem = 0;//列号
	//SendMessage(hListProcess,LVM_INSERTCOLUMN,0,(DWORD)&lv);
	ListView_InsertColumn(hListBlocksList, 0, &lv);
	
	//第二列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Section");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 1;//列号
	ListView_InsertColumn(hListBlocksList, 1, &lv);

	//第三列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("RAV");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 2;//列号
	ListView_InsertColumn(hListBlocksList, 2, &lv);

	//第四列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Items");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 3;//列号
	ListView_InsertColumn(hListBlocksList, 3, &lv);

	EnumBlocks(hListBlocksList);

	return ;
}
VOID InitBlockItemListView(HWND hwndDirectoryDetail)
{
	LV_COLUMN lv;
	HWND hListBlockItemList;

	//初始化
	memset(&lv, 0, sizeof(LV_COLUMN));
	//获取IDC_LIST_DLL_List的句柄
	hListBlockItemList = GetDlgItem(hwndDirectoryDetail,  IDC_LIST_BlockItem);
	//设置整行选中   点击某一行该行全部选中
	SendMessage(hListBlockItemList, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	
	//第一列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Index");//列标题
	lv.cx = 80;//列宽
	lv.iSubItem = 0;//列号
	//SendMessage(hListProcess,LVM_INSERTCOLUMN,0,(DWORD)&lv);
	ListView_InsertColumn(hListBlockItemList, 0, &lv);
	
	//第二列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("RAV");//列标题
	lv.cx = 120;//列宽
	lv.iSubItem = 1;//列号
	ListView_InsertColumn(hListBlockItemList, 1, &lv);

	//第三列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("FOA / Offset");//列标题
	lv.cx = 100;//列宽
	lv.iSubItem = 2;//列号
	ListView_InsertColumn(hListBlockItemList, 2, &lv);

	//第四列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Type");//列标题
	lv.cx = 100;//列宽
	lv.iSubItem = 3;//列号
	ListView_InsertColumn(hListBlockItemList, 3, &lv);

	//第五列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Far Address");//列标题
	lv.cx = 100;//列宽
	lv.iSubItem = 4;//列号
	ListView_InsertColumn(hListBlockItemList, 4, &lv);

	//第六列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Data Interpretation");//列标题
	lv.cx = 300;//列宽
	lv.iSubItem = 5;//列号
	ListView_InsertColumn(hListBlockItemList, 5, &lv);

	return ;
}

VOID InitResourceTreeView(HWND hwndDirectoryDetail)
{
	TCHAR* szResType[0x11] = { 0, "鼠标指针", "位图", "图标", "菜单",
								"对话框", "字符串列表","字体目录", "字体",
								"加速键", "非格式化资源", "消息列表", "鼠标指针组",
								"zz", "图标组","xx", "版本信息"};

	//创建树状控件对象
	TVITEM tv;
	TVINSERTSTRUCT tvs;
	memset(&tv, 0, sizeof(TVITEM));
	memset(&tvs, 0, sizeof(TVINSERTSTRUCT));

	HTREEITEM hParent;    // Tree item handle
	HTREEITEM hBefore;    // 前一级目录
	HTREEITEM hRoot;      // .......

	HWND hTreeView;
	hTreeView = GetDlgItem(hwndDirectoryDetail, IDC_TREE_Resource);

	DWORD ret = 0;
	
	//文件读取	
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if(ret != 0)
	{
		MessageBox(hwndDirectoryDetail, TEXT("File Read Error !"), TEXT("Error"), MB_OK);
		return;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的MZ标志"),TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	//判断是否是有效的PE 
	if(pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的PE标志"),TEXT("Error"),MB_OK);
		free(pFileAdd);
		return;
	}

	DWORD ResourceDirectoryAdd_RVA = pNTHeader->OptionalHeader.DataDirectory[2].VirtualAddress;
	DWORD ResourceDirectoryAdd_FOA = 0;
	ret = RVA_TO_FOA(ResourceDirectoryAdd_RVA, &ResourceDirectoryAdd_FOA, pFileAdd);
	if(ret != 0)
	{
		MessageBox(hwndDirectoryDetail, TEXT("Func RVA_TO_FOA Error!"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return ;
	}
	PIMAGE_RESOURCE_DIRECTORY pResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)((PBYTE)pFileAdd + ResourceDirectoryAdd_FOA);
	DWORD dwResourceTypeNum = 0;
	DWORD dwResourceIDNum = 0;
	DWORD dwResourcePageNum = 0;
	

	HWND hwndRootName = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_RootName);
	TCHAR szRootNameNum[10] = {0};
	wsprintf(szRootNameNum, "%04X", pResourceDirectory->NumberOfNamedEntries);
	SetWindowText(hwndRootName, szRootNameNum);

	HWND hwndRootID = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_RootID);
	TCHAR szRootIDNum[10] = {0};
	wsprintf(szRootIDNum, "%04X", pResourceDirectory->NumberOfIdEntries);
	SetWindowText(hwndRootID, szRootIDNum);


	//遍历第一层结构
	while(dwResourceTypeNum < (pResourceDirectory->NumberOfIdEntries + pResourceDirectory->NumberOfNamedEntries))
	{
		PIMAGE_RESOURCE_DIRECTORY_ENTRY pResourceTypeEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((PBYTE)pResourceDirectory + dwResourceTypeNum * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY)+sizeof(IMAGE_RESOURCE_DIRECTORY));
	
		//最高位为0 表示是类型名不是字符串
		if(!pResourceTypeEntry->NameIsString)
		{
			if(pResourceTypeEntry->Id < 0x11)
			{
				//第一层树状结构
				tvs.hParent = NULL;
				tvs.hInsertAfter=TVI_LAST; 
				tvs.item.mask=TVIF_TEXT;
				tvs.item.pszText=szResType[pResourceTypeEntry->Id];
				hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_Resource,TVM_INSERTITEM,0,(LPARAM)&tvs);
			}
			else
			{
				TCHAR szBuffer[10] = {0};
				wsprintf(szBuffer, "%d", pResourceTypeEntry->Id);
				tvs.hParent = NULL;
				tvs.hInsertAfter=TVI_LAST; 
				tvs.item.mask=TVIF_TEXT;
				tvs.item.pszText=szBuffer;
				hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_Resource,TVM_INSERTITEM,0,(LPARAM)&tvs);

			}

		}
		else//类型名是字符串
		{
			PIMAGE_RESOURCE_DIR_STRING_U pStringName = (PIMAGE_RESOURCE_DIR_STRING_U)((DWORD)pResourceDirectory + pResourceTypeEntry->NameOffset);
			TCHAR szBuffer[20] = {0};
			//复制字符串
			for(DWORD dwNum = 0; dwNum < pStringName->Length; dwNum++)
			{																	  
				szBuffer[dwNum] = pStringName->NameString[dwNum];
			}

			tvs.hParent = NULL;
			tvs.hInsertAfter=TVI_LAST; 
			tvs.item.mask=TVIF_TEXT;
			tvs.item.pszText=szBuffer;
			hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_Resource,TVM_INSERTITEM,0,(LPARAM)&tvs);

		}

		//计算第二次目录的偏移
		if(pResourceTypeEntry->DataIsDirectory)
		{
			//指向第二层目录
			PIMAGE_RESOURCE_DIRECTORY pResourceIDDirectory = (PIMAGE_RESOURCE_DIRECTORY)((DWORD)pResourceDirectory + pResourceTypeEntry->OffsetToDirectory);
			
			hRoot=hParent;
			hBefore=hParent;
			while(dwResourceIDNum < (pResourceIDDirectory->NumberOfIdEntries + pResourceIDDirectory->NumberOfNamedEntries))
			{
				PIMAGE_RESOURCE_DIRECTORY_ENTRY pResourceIDEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((PBYTE)pResourceIDDirectory + dwResourceIDNum * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY)+sizeof(IMAGE_RESOURCE_DIRECTORY));
				
				if(!pResourceIDEntry->NameIsString)
				{
					TCHAR szBuffer[10] = {0};
					wsprintf(szBuffer, "%d", pResourceIDEntry->Id);
					tvs.hParent = hBefore;
					tvs.hInsertAfter=TVI_LAST; 
					tvs.item.mask=TVIF_TEXT;
					tvs.item.pszText=szBuffer;
					hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_Resource,TVM_INSERTITEM,0,(LPARAM)&tvs);
				}
				else
				{
					PIMAGE_RESOURCE_DIR_STRING_U pStringName = (PIMAGE_RESOURCE_DIR_STRING_U)((DWORD)pResourceDirectory + pResourceIDEntry->NameOffset);
					char szBuffer[40] = {0};
					
					for(DWORD dwNum = 0; dwNum < pStringName->Length; dwNum++)
					{
						szBuffer[dwNum] = pStringName->NameString[dwNum];
					}
					tvs.hParent = hBefore;
					tvs.hInsertAfter=TVI_LAST; 
					tvs.item.mask=TVIF_TEXT;
					tvs.item.pszText=szBuffer;
					hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_Resource,TVM_INSERTITEM,0,(LPARAM)&tvs);
				}

				dwResourceIDNum++;
				dwResourcePageNum = 0;
			}
		}

		dwResourceTypeNum++;
		dwResourceIDNum = 0;
		dwResourcePageNum = 0;
	}

	free(pFileAdd);
	return ;
}

VOID InitResourceEditInfo(HWND hwndTreeView, LPNMHDR pNMHDR)
{
	DWORD ret = 0;


	/*
	//先获取被点击的Item句柄
	HTREEITEM hSelectItem = TreeView_GetSelection(hwndTreeView, IDC_TREE_Resource);
	if(hSelectItem == NULL)
	{
		return ;
	}
	TCHAR szRoot[30] = {0};//存储根节点文本信息
	TCHAR szSelect[30] = {0};//存储被选节点的文本信息

	//获取被选中的节点信息
	TV_ITEM tvSelect;
	memset(&tvSelect, 0, sizeof(tvSelect));
	tvSelect.hItem = hSelectItem;
	tvSelect.pszText = szSelect;
	tvSelect.cchTextMax = sizeof(szSelect);
	tvSelect.mask = TVIF_TEXT |TVIF_SELECTEDIMAGE;
	TreeView_GetItem(hwndTreeView, &tvSelect);

	//获取根节点句柄
	HTREEITEM hRoot = TreeView_GetNextItem(hwndTreeView, hSelectItem, TVGN_CARET);

	//将szBuffer设置为缓存
	TV_ITEM tvRoot;
	memset(&tvRoot, 0, sizeof(tvRoot));
	tvRoot.hItem = hSelectItem;
	tvRoot.pszText = szRoot;
	tvRoot.cchTextMax = sizeof(szRoot);
	tvRoot.mask = TVIF_TEXT;

	//获取根节点的信息
	TreeView_GetItem(hwndTreeView, &tvRoot);

	//遍历查找根节点
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if(ret != 0)
	{
		MessageBox(hwndTreeView, TEXT("File Read Error !"), TEXT("Error"), MB_OK);
		return;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的MZ标志"),TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	//判断是否是有效的PE 
	if(pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的PE标志"),TEXT("Error"),MB_OK);
		free(pFileAdd);
		return;
	}

	DWORD ResourceDirectoryAdd_RVA = pNTHeader->OptionalHeader.DataDirectory[2].VirtualAddress;
	DWORD ResourceDirectoryAdd_FOA = 0;
	ret = RVA_TO_FOA(ResourceDirectoryAdd_RVA, &ResourceDirectoryAdd_FOA, pFileAdd);
	if(ret != 0)
	{
		MessageBox(hwndTreeView, TEXT("Func RVA_TO_FOA Error!"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return ;
	}
	PIMAGE_RESOURCE_DIRECTORY pResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)((PBYTE)pFileAdd + ResourceDirectoryAdd_FOA);
	DWORD dwResourceTypeNum = 0;
	DWORD dwResourceIDNum = 0;
	DWORD dwResourcePageNum = 0;
	


	free(pFileAdd);
	*/
	return;
}

VOID InitBoundImportTreeView(HWND hwndDirectoryDetail)
{
	//创建树状控件对象
	TVITEM tv;
	TVINSERTSTRUCT tvs;
	memset(&tv, 0, sizeof(TVITEM));
	memset(&tvs, 0, sizeof(TVINSERTSTRUCT));

	HTREEITEM hParent;    // Tree item handle
	HTREEITEM hBefore;    // 前一级目录
	HTREEITEM hRoot;      // .......
	HTREEITEM hForword;

	HWND hTreeView;
	hTreeView = GetDlgItem(hwndDirectoryDetail, IDC_TREE_BoundImport);

	DWORD ret = 0;
	
	//文件读取	
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if(ret != 0)
	{
		MessageBox(hwndDirectoryDetail, TEXT("File Read Error !"), TEXT("Error"), MB_OK);
		return;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的MZ标志"),TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	//判断是否是有效的PE 
	if(pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的PE标志"),TEXT("Error"),MB_OK);
		free(pFileAdd);
		return;
	}
	if (pNTHeader->OptionalHeader.DataDirectory[11].VirtualAddress == 0)
	{
		MessageBox(0, TEXT("BoundImport table is not Exist!"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}

	DWORD BoundImportDirectoryAdd_RVA = pNTHeader->OptionalHeader.DataDirectory[11].VirtualAddress;
	DWORD BoundImportDirectoryAdd_FOA = 0;
	ret = RVA_TO_FOA(BoundImportDirectoryAdd_RVA, &BoundImportDirectoryAdd_FOA, pFileAdd);
	if(ret != 0)
	{
		MessageBox(hwndDirectoryDetail, TEXT("Func RVA_TO_FOA Error!"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return ;
	}
	PIMAGE_BOUND_IMPORT_DESCRIPTOR pBoundImportDirectory = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)((PBYTE)pFileAdd + BoundImportDirectoryAdd_FOA);
	
	//存储第一个绑定表地址
	DWORD dwBoundImportAddress = (DWORD)pBoundImportDirectory;
	while(pBoundImportDirectory->TimeDateStamp != 0 && pBoundImportDirectory->OffsetModuleName != 0)
	{
		TCHAR szTimeDateStamp[100] = {0};
		TCHAR szTimeData[100] = {0};
		TCHAR szOffsetModuleName[50] = {0};
		TCHAR szNumberOfModuleForwarderRefs[50] = {0}; 


		wsprintf(szTimeDateStamp, "TimeDateStamp :%08X", pBoundImportDirectory->TimeDateStamp);
		wsprintf(szOffsetModuleName, "OffsetModuleName :%04X", pBoundImportDirectory->OffsetModuleName);
		wsprintf(szNumberOfModuleForwarderRefs, "NumberOfModuleForwarderRefs :%Xh / %dd", pBoundImportDirectory->NumberOfModuleForwarderRefs, pBoundImportDirectory->NumberOfModuleForwarderRefs);

		//strftime格式化时间显示  时间戳转换成时间字符串
		struct tm p;
		p = *(struct tm *)gmtime((time_t*)&pBoundImportDirectory->TimeDateStamp);
		strftime(szTimeData, sizeof(szTimeData), "GMT :%Y-%m-%d %H:%M:%S", &p);
	
		TCHAR *pModuleName = (TCHAR *)(dwBoundImportAddress + pBoundImportDirectory->OffsetModuleName);

		//打印绑定导入表
		tvs.hParent = NULL;
		tvs.hInsertAfter=TVI_LAST; 
		tvs.item.mask=TVIF_TEXT;
		tvs.item.pszText=pModuleName;
		hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_BoundImport,TVM_INSERTITEM,0,(LPARAM)&tvs);

		hRoot=hParent;
		hBefore=hParent;
		
		tvs.hParent = hBefore;
		tvs.hInsertAfter=TVI_LAST; 
		tvs.item.mask=TVIF_TEXT;
		tvs.item.pszText=szTimeDateStamp;
		hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_BoundImport,TVM_INSERTITEM,0,(LPARAM)&tvs);
	
		tvs.hParent = hBefore;
		tvs.hInsertAfter=TVI_LAST; 
		tvs.item.mask=TVIF_TEXT;
		tvs.item.pszText=szTimeData;
		hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_BoundImport,TVM_INSERTITEM,0,(LPARAM)&tvs);
		
		tvs.hParent = hBefore;
		tvs.hInsertAfter=TVI_LAST; 
		tvs.item.mask=TVIF_TEXT;
		tvs.item.pszText=szOffsetModuleName;
		hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_BoundImport,TVM_INSERTITEM,0,(LPARAM)&tvs);
		
		tvs.hParent = hBefore;
		tvs.hInsertAfter=TVI_LAST; 
		tvs.item.mask=TVIF_TEXT;
		tvs.item.pszText=szNumberOfModuleForwarderRefs;
		hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_BoundImport,TVM_INSERTITEM,0,(LPARAM)&tvs);

		if(pBoundImportDirectory->NumberOfModuleForwarderRefs)
		{
			tvs.hParent = hBefore;
			tvs.hInsertAfter=TVI_LAST; 
			tvs.item.mask=TVIF_TEXT;
			tvs.item.pszText=TEXT("ModuleForwarders");
			hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_BoundImport,TVM_INSERTITEM,0,(LPARAM)&tvs);

			PIMAGE_BOUND_FORWARDER_REF pBoundForwardRef = (PIMAGE_BOUND_FORWARDER_REF)((PBYTE)pBoundImportDirectory + sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR));

			hBefore=hParent;

			DWORD dwNum = 0;
			//循环打印依赖模块
			while(dwNum < pBoundImportDirectory->NumberOfModuleForwarderRefs)
			{

				TCHAR szOffsetForName[50] = {0};
				TCHAR szReserved[50] = {0};
				TCHAR *szForName = (TCHAR *)(dwBoundImportAddress + pBoundForwardRef->OffsetModuleName);
				TCHAR szForTimeStamp[50] = {0};
				TCHAR szForTimeData[100] = {0};

				wsprintf(szOffsetForName, "OffsetModuleName :%04X", pBoundForwardRef->OffsetModuleName);
				wsprintf(szReserved, "Reserved :%04X", pBoundForwardRef->Reserved);
				wsprintf(szForTimeStamp, "TimeDateStamp :%08X", pBoundForwardRef->TimeDateStamp);

				//strftime格式化时间显示  时间戳转换成时间字符串
				struct tm pf;
				pf = *(struct tm *)gmtime((time_t*)&pBoundForwardRef->TimeDateStamp);
				strftime(szForTimeData, sizeof(szForTimeData), "GMT :%Y-%m-%d %H:%M:%S", &pf);

				tvs.hParent = hBefore;
				tvs.hInsertAfter=TVI_LAST; 
				tvs.item.mask=TVIF_TEXT;
				tvs.item.pszText=szForName;
				hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_BoundImport,TVM_INSERTITEM,0,(LPARAM)&tvs);

				hForword = hParent;

				tvs.hParent = hForword;
				tvs.hInsertAfter=TVI_LAST; 
				tvs.item.mask=TVIF_TEXT;
				tvs.item.pszText=szForTimeStamp;
				hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_BoundImport,TVM_INSERTITEM,0,(LPARAM)&tvs);

				tvs.hParent = hForword;
				tvs.hInsertAfter=TVI_LAST; 
				tvs.item.mask=TVIF_TEXT;
				tvs.item.pszText=szForTimeData;
				hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_BoundImport,TVM_INSERTITEM,0,(LPARAM)&tvs);

				tvs.hParent = hForword;
				tvs.hInsertAfter=TVI_LAST; 
				tvs.item.mask=TVIF_TEXT;
				tvs.item.pszText=szOffsetForName;
				hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_BoundImport,TVM_INSERTITEM,0,(LPARAM)&tvs);

				tvs.hParent = hForword;
				tvs.hInsertAfter=TVI_LAST; 
				tvs.item.mask=TVIF_TEXT;
				tvs.item.pszText=szReserved;
				hParent = (HTREEITEM)SendDlgItemMessage(hwndDirectoryDetail,IDC_TREE_BoundImport,TVM_INSERTITEM,0,(LPARAM)&tvs);

				dwNum++;
			}
		}
		pBoundImportDirectory += (1 + pBoundImportDirectory->NumberOfModuleForwarderRefs);//(PIMAGE_BOUND_IMPORT_DESCRIPTOR)((PBYTE)pBoundImportDirectory + sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR) + pBoundImportDirectory->NumberOfModuleForwarderRefs * sizeof(IMAGE_BOUND_FORWARDER_REF));
	}

	free(pFileAdd);
	return ;
}

VOID InitDebugTableList(HWND hwndDirectoryDetail)
{
	LV_COLUMN lv;
	HWND hListDebugTable;

	//初始化
	memset(&lv, 0, sizeof(LV_COLUMN));
	//获取IDC_LIST_Process的句柄
	hListDebugTable = GetDlgItem(hwndDirectoryDetail,  IDC_LIST_DebugTable);
	//设置整行选中   点击某一行该行全部选中
	SendMessage(hListDebugTable, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);


	//第一列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Characteristics");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 0;//列号
	//SendMessage(hListProcess,LVM_INSERTCOLUMN,0,(DWORD)&lv);
	ListView_InsertColumn(hListDebugTable, 0, &lv);
	
	//第二列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("TimeDateStamp");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 1;//列号
	ListView_InsertColumn(hListDebugTable, 1, &lv);

	//第三列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("MajorVersion");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 2;//列号
	ListView_InsertColumn(hListDebugTable, 2, &lv);

	//第四列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("MinorVersion");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 3;//列号
	ListView_InsertColumn(hListDebugTable, 3, &lv);

	//第五列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("Type");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 4;//列号
	ListView_InsertColumn(hListDebugTable, 4, &lv);

	//第六列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("SizeOfData");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 5;//列号
	ListView_InsertColumn(hListDebugTable, 5, &lv);

	//第七列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("AddressOfRawData");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 6;//列号
	ListView_InsertColumn(hListDebugTable, 6, &lv);

	//第八列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = TEXT("PointerToRawData");//列标题
	lv.cx = 150;//列宽
	lv.iSubItem = 7;//列号
	ListView_InsertColumn(hListDebugTable, 7, &lv);


	EnumDebugTable(hListDebugTable);

	return ;
}

VOID InitTlsList(HWND hwndDirectoryDetail)
{
	DWORD ret = 0;
	
	//文件读取	
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if(ret != 0)
	{
		MessageBox(hwndDirectoryDetail, TEXT("File Read Error !"), TEXT("Error"), MB_OK);
		return;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的MZ标志"),TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	//判断是否是有效的PE 
	if(pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的PE标志"),TEXT("Error"),MB_OK);
		free(pFileAdd);
		return;
	}

	DWORD TlsDirectoryAdd_RVA = pNTHeader->OptionalHeader.DataDirectory[9].VirtualAddress;
	DWORD TlsDirectoryAdd_FOA = 0;
	ret = RVA_TO_FOA(TlsDirectoryAdd_RVA, &TlsDirectoryAdd_FOA, pFileAdd);
	if(ret != 0)
	{
		MessageBox(hwndDirectoryDetail, TEXT("Func RVA_TO_FOA Error!"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return ;
	}
	PIMAGE_TLS_DIRECTORY32 pTlsDirectory = (PIMAGE_TLS_DIRECTORY32)((PBYTE)pFileAdd + TlsDirectoryAdd_FOA);
	

	HWND hStartAddressOfRawData = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_StartAddressOfRawData);
	TCHAR tStartAddressOfRawData[10] = { 0 };
	sprintf(tStartAddressOfRawData, "%08X", pTlsDirectory->StartAddressOfRawData);
	SendMessage(hStartAddressOfRawData, WM_SETTEXT, 0, (long)tStartAddressOfRawData);

	HWND hEndAddressOfRawData = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_EndAddressOfRawData);
	TCHAR tEndAddressOfRawData[10] = { 0 };
	sprintf(tEndAddressOfRawData, "%08X", pTlsDirectory->EndAddressOfRawData);
	SendMessage(hEndAddressOfRawData, WM_SETTEXT, 0, (long)tEndAddressOfRawData);

	HWND hAddressOfIndex = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_AddressOfIndex);
	TCHAR tAddressOfIndex[10] = { 0 };
	sprintf(tAddressOfIndex, "%08X", pTlsDirectory->AddressOfIndex);
	SendMessage(hAddressOfIndex, WM_SETTEXT, 0, (long)tAddressOfIndex);

	HWND hAddressOfCallBacks = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_AddressOfCallBacks);
	TCHAR tAddressOfCallBacks[10] = { 0 };
	sprintf(tAddressOfCallBacks, "%08X", pTlsDirectory->AddressOfCallBacks);
	SendMessage(hAddressOfCallBacks, WM_SETTEXT, 0, (long)tAddressOfCallBacks);

	HWND hSizeOfZeroFill = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_SizeOfZeroFill);
	TCHAR tSizeOfZeroFill[10] = { 0 };
	sprintf(tSizeOfZeroFill, "%08X", pTlsDirectory->SizeOfZeroFill);
	SendMessage(hSizeOfZeroFill, WM_SETTEXT, 0, (long)tSizeOfZeroFill);

	HWND hCharacteristics = GetDlgItem(hwndDirectoryDetail, IDC_EDIT_Characteristics);
	TCHAR tCharacteristics[10] = { 0 };
	sprintf(tCharacteristics, "%08X", pTlsDirectory->Characteristics);
	SendMessage(hCharacteristics, WM_SETTEXT, 0, (long)tCharacteristics);

	free(pFileAdd);
	return ;

}
//////////////////////////////////////////////////////////////////////////////////////////
//**************************************************************************************//
//////////////////////////////////////////////////////////////////////////////////////////


VOID EnumProcess(HWND hListProcess)
{

	//利用 <Tlhelp32.h>里面的API  CreateToolhelp32Snapshot() Process32First()和Process32Next()	
	LV_ITEM vitem;			

	DWORD dwIdx=0;					//计数
	TCHAR szPath[MAX_PATH];         //进程路径
	TCHAR szPID[10];                //PID
	TCHAR szImageBase[10];           //镜像基址
	TCHAR szBaseSize[10];           //镜像大小
	HANDLE hProSnapshot =NULL;		//进程快照句柄
	HANDLE hModSnapshot=NULL;		//模块快照句柄
	PROCESSENTRY32 ProcessEntry= {0};//Describes an entry from a list that enumerates the processes 
									//residing in the system address space when a snapshot was taken.

	MODULEENTRY32 MoudleEntry = {0};//This structure describes an entry from a list 
									//that enumerates the modules used by a specified process.
	
	//初始化								
	ProcessEntry.dwSize =sizeof(ProcessEntry);
	ZeroMemory(&vitem,sizeof(vitem));
	vitem.mask		= LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	vitem.state		= 0;
	vitem.stateMask	= 0;
	
	//创建进程快照
	hProSnapshot =CreateToolhelp32Snapshot(TH32CS_SNAPALL,0); 
	if(hProSnapshot==INVALID_HANDLE_VALUE)
	{
		return;
	}

	//枚举进程	获取第一个进程信息
	if(!Process32First(hProSnapshot,&ProcessEntry))
	{
		return;
	}

	do
	{
		sprintf(szPID,"%d",ProcessEntry.th32ProcessID);

		//获取相应进程的模块快照
		hModSnapshot =CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,ProcessEntry.th32ProcessID);
		if (hModSnapshot !=INVALID_HANDLE_VALUE)
		{
			//清零
			ZeroMemory(&MoudleEntry,sizeof(MoudleEntry));
			
			MoudleEntry.dwSize = sizeof(MoudleEntry);

			//此函数检索与进程相关联的第一个模块的信息         ProcessEntry.th32ProcessID进程的PID!=0
			if(Module32First(hModSnapshot, &MoudleEntry) && ProcessEntry.th32ProcessID !=0)
			{
				//将进程的ImageBase ，SizeOfImage  Path and filename of the executable file for the process 放入缓存
				sprintf(szImageBase,"%08X",MoudleEntry.modBaseAddr);
				sprintf(szBaseSize,"%08X",MoudleEntry.modBaseSize);
				sprintf(szPath,"%s",ProcessEntry.szExeFile);

			}else{
				//用0 填充
				sprintf(szImageBase,"%08X",0);
				sprintf(szBaseSize,"%08X",0);
				sprintf(szPath,"%s",ProcessEntry.szExeFile);
				
			}
		}else{
			//用0 填充
			sprintf(szImageBase,"%08X",0);
			sprintf(szBaseSize,"%08X",0);
			sprintf(szPath,"%s",ProcessEntry.szExeFile);
		}

		//将PID的数字转化成字符串
		TCHAR NumToChar[10] = {0};
		sprintf(NumToChar, "%d", dwIdx);

		//打印信息
		vitem.pszText =NumToChar;
		vitem.cchTextMax=MAX_PATH;
		vitem.iItem =dwIdx;
	    ListView_InsertItem(hListProcess,&vitem);
		ListView_SetItemText(hListProcess,dwIdx,1,szPath);
		ListView_SetItemText(hListProcess,dwIdx,2,szPID);
		ListView_SetItemText(hListProcess,dwIdx,3,szImageBase);
		ListView_SetItemText(hListProcess,dwIdx,4,szBaseSize);
		
		dwIdx++;//计数器
	//获取下一个进程
	}while(	Process32Next(hProSnapshot,&ProcessEntry));

}

VOID EnumMoudle(HWND hListProcess, HWND hListModule)
{
	DWORD dwRowId;					//行号
	TCHAR szPid[0x20];				//接收hListProcess 的 PID
	LV_ITEM lv;						//This structure that specifies or receives the attributes of a list view item.
	HWND hModule;					//Moudle的句柄
	ListView_DeleteAllItems(hListModule);//打印前清空IDC_LIST_Moudle里面的内容

	//清空缓存
	memset(&lv,0,sizeof(LV_ITEM));
	memset(szPid,0,0x20);
	
	//获取选择行
	dwRowId = SendMessage(hListProcess,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
	if(dwRowId == -1)
	{
		MessageBox(NULL,TEXT("Please select true process"),TEXT("Warning"),MB_OK);
		return;
	}

	//获取IDC_LIST_Moudle的句柄
	hModule = GetDlgItem((HWND)hAppInstance,IDC_LIST_Moudle);

	//获取PID
	lv.iSubItem = 2;      //第三列的信息
	lv.pszText = szPid;		
	lv.cchTextMax = 0x20;
	SendMessage(hListProcess,LVM_GETITEMTEXT,dwRowId,(DWORD)&lv);

	//字符串转成数字
	DWORD dwPID = atoi(szPid);

	//通过pid列出所有的Modules
	HANDLE               hModuleSnap = INVALID_HANDLE_VALUE;	//-1
    MODULEENTRY32        MoudleEnyty = {0};
 
    //给进程所引用的模块信息设定一个快照 
    hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
    if(hModuleSnap == INVALID_HANDLE_VALUE)//判断是否接收到快照
    {
		int i = GetLastError();
		return ;
    }
 
    MoudleEnyty.dwSize = sizeof(MODULEENTRY32);

	//This function retrieves information about the first module associated with a process.
    if(!Module32First(hModuleSnap, &MoudleEnyty))
    {
        CloseHandle(hModuleSnap);
		return ;
    }
	
	//将模块信息打印到下面的ListView中	
	DWORD dwIdx= 0;
    do
    {
		lv.pszText =MoudleEnyty.szModule;
		lv.iItem =dwIdx;
		lv.iSubItem = 0;
		ListView_InsertItem(hListModule,&lv);//向第一列打印信息

		ListView_SetItemText(hListModule,dwIdx,1,MoudleEnyty.szExePath);//向第二列打印信息
		
		dwIdx++;
	//This function retrieves information about the next module associated with a process or thread.
    } while(Module32Next(hModuleSnap, &MoudleEnyty));
 
	//关闭快照句柄
    CloseHandle(hModuleSnap);
}

VOID EnumSectionTable(HWND hListSectionTable)
{
	DWORD ret = 0;
	
	//文件读取	
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if(ret != 0)
	{
		MessageBox(hListSectionTable, TEXT("File Read Error !"), TEXT("Error"), MB_OK);
		return;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的MZ标志"),TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	//判断是否是有效的PE 
	if(pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的PE标志"),TEXT("Error"),MB_OK);
		free(pFileAdd);
		return;
	}

	//注意：必须设置mask
	LV_ITEM vitem;	
	memset(&vitem,0,sizeof(LV_ITEM));
	vitem.mask	= LVIF_TEXT | LVIF_STATE;


	//循环打印节表消息
	for(DWORD dwIdx = 0; dwIdx < pNTHeader->FileHeader.NumberOfSections; dwIdx++)
	{
		
		TCHAR szName[10] = {0};
		sprintf(szName, "%s", pSectionGroup->Name);
		
		TCHAR szVirtualAddress[10] = {0};
		sprintf(szVirtualAddress, "%08X", pSectionGroup->VirtualAddress);
		
		TCHAR szVirtualSize[10] = {0};
		sprintf(szVirtualSize, "%08X", pSectionGroup->Misc.VirtualSize);

		TCHAR szPointerToRawData[10] = {0};
		sprintf(szPointerToRawData, "%08X", pSectionGroup->PointerToRawData);
		
		TCHAR szSizeOfRawData[10] = {0};
		sprintf(szSizeOfRawData, "%08X", pSectionGroup->SizeOfRawData);

		TCHAR szCharacteristics[10] = {0};
		sprintf(szCharacteristics, "%08X", pSectionGroup->Characteristics);

		vitem.pszText =szName;
		vitem.cchTextMax=sizeof(szName);
		vitem.iItem =dwIdx;
		vitem.iSubItem = 0;
	    ListView_InsertItem(hListSectionTable, &vitem);
		ListView_SetItemText(hListSectionTable, dwIdx, 1, szVirtualAddress);
		ListView_SetItemText(hListSectionTable, dwIdx, 2, szVirtualSize);
		ListView_SetItemText(hListSectionTable, dwIdx, 3, szPointerToRawData);
		ListView_SetItemText(hListSectionTable, dwIdx, 4, szSizeOfRawData);
		ListView_SetItemText(hListSectionTable, dwIdx, 5, szCharacteristics);

		pSectionGroup++;
	}

	free(pFileAdd);
	return ;
}

VOID EnumFuncInformation(HWND hListTableList)
{
	DWORD ret = 0;

	//文件读取	
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if (ret != 0)
	{
		MessageBox(hListTableList, TEXT("File Read Error !"), TEXT("Error"), MB_OK);
		return;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(0, TEXT("不是有效的MZ标志"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	//判断是否是有效的PE 
	if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(0, TEXT("不是有效的PE标志"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}

	//获取导入表的RAV
	DWORD ExportDirectoryAdd_RVA = pNTHeader->OptionalHeader.DataDirectory[0].VirtualAddress;
	DWORD ExportDirectoryAdd_FOA = 0;
	ret = RVA_TO_FOA(ExportDirectoryAdd_RVA, &ExportDirectoryAdd_FOA, pFileAdd);
	if (ret != 0)
	{
		MessageBox(NULL, TEXT("func EnumFuncInformation error"), TEXT("Error"), MB_OK);
		return;
	}
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)pFileAdd + ExportDirectoryAdd_FOA);

	LV_ITEM lv;
	memset(&lv, 0, sizeof(LV_ITEM));
	lv.mask = LVIF_TEXT | LVIF_STATE;

	//通过FunctionName	 ---->>    NameOrdinal    ----->>   FunctionAddress
	//1、定位函数名表
	DWORD FuncNameTable_RVA = pExportDirectory->AddressOfNames;
	DWORD FuncNameTable_FOA = 0;
	ret = RVA_TO_FOA(FuncNameTable_RVA, &FuncNameTable_FOA, pFileAdd);
	if (ret != 0)
	{
		MessageBox(NULL, TEXT("func EnumFuncInformation error"), TEXT("Error"), MB_OK);
		return;
	}
	DWORD AddressOfName = FuncNameTable_FOA;
	
	//	定位导出序号表
	DWORD FuncOrdinalTable_RVA = pExportDirectory->AddressOfNameOrdinals;
	DWORD FuncOrdinalTable_FOA = 0;
	ret = RVA_TO_FOA(FuncOrdinalTable_RVA, &FuncOrdinalTable_FOA, pFileAdd);
	if (ret != 0)
	{
		MessageBox(NULL, TEXT("func EnumFuncInformation error"), TEXT("Error"), MB_OK);
		return;
	}
	DWORD AddressOfNameOrdinals = FuncOrdinalTable_FOA;

	//	定位函数地址表
	DWORD FuncAddressTable_RVA = pExportDirectory->AddressOfFunctions;
	DWORD FuncAddressTable_FOA = 0;
	ret = RVA_TO_FOA(FuncAddressTable_RVA, &FuncAddressTable_FOA, pFileAdd);
	if (ret != 0)
	{
		MessageBox(NULL, TEXT("func EnumFuncInformation error"), TEXT("Error"), MB_OK);
		return;
	}
	DWORD AddressOFunctions = (DWORD)((PBYTE)pFileAdd + FuncAddressTable_FOA);
	
	DWORD dwNum = 0;								//循环次数
	TCHAR *szFuncName = NULL;						//函数名
	TCHAR szOrdinals[10] = { 0 };					//导出序号
	TCHAR szFuncAddRVA[10] = { 0 };				//函数相对虚拟地址
	TCHAR szFuncAddFOA[10] = { 0 };				//函数文件偏移地址

	//2、遍历函数名
	while (dwNum < pExportDirectory->NumberOfNames)
	{
		DWORD FuncName_RVA = *(PDWORD)((PBYTE)pFileAdd + AddressOfName + sizeof(DWORD) * dwNum);
		DWORD FuncName_FOA = 0;
		//3、定位函数名
		ret = RVA_TO_FOA(FuncName_RVA, &FuncName_FOA, pFileAdd);
		if (ret != 0)
		{
			MessageBox(NULL, TEXT("func EnumFuncInformation error"), TEXT("Error"), MB_OK);
			return;
		}
		szFuncName = (TCHAR *)((PBYTE)pFileAdd + FuncName_FOA);

		//获取函数的导出序号
		WORD FuncOrdinal = *(WORD*)((PBYTE)pFileAdd + AddressOfNameOrdinals + sizeof(WORD) * dwNum);
		
		//获取函数的相对虚拟地址
		DWORD FuncAddress_RVA = *(PDWORD)(AddressOFunctions + sizeof(DWORD) * FuncOrdinal);
		DWORD FuncAddress_FOA = 0;
		ret = RVA_TO_FOA(FuncAddress_RVA, &FuncAddress_FOA, pFileAdd);
		if (ret != 0)
		{
			MessageBox(NULL, TEXT("func EnumFuncInformation error"), TEXT("Error"), MB_OK);
			return;
		}

		//将 数字 转换成 字符
		sprintf(szOrdinals, "%04X", FuncOrdinal + pExportDirectory->Base);
		sprintf(szFuncAddRVA, "%08X", FuncAddress_RVA);
		sprintf(szFuncAddFOA, "%08X", FuncAddress_FOA);

		//打印
		lv.pszText = szOrdinals;
		lv.cchTextMax = sizeof(szOrdinals);
		lv.iItem = dwNum;
		lv.iSubItem = 0;
		ListView_InsertItem(hListTableList, &lv);
		ListView_SetItemText(hListTableList, dwNum, 1, szFuncAddRVA);
		ListView_SetItemText(hListTableList, dwNum, 2, szFuncAddFOA);
		ListView_SetItemText(hListTableList, dwNum, 3, szFuncName);

		dwNum++;
	}

	free(pFileAdd);
	return;
}



VOID EnumDllName(HWND hListDllList)
{
	DWORD ret = 0;
	
	//文件读取	
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if(ret != 0)
	{
		MessageBox(hListDllList, TEXT("File Read Error !"), TEXT("Error"), MB_OK);
		return;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的MZ标志"),TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	//判断是否是有效的PE 
	if(pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的PE标志"),TEXT("Error"),MB_OK);
		free(pFileAdd);
		return;
	}

	//获取导入表的RAV
	DWORD ImportDirectoryAdd_RVA = pNTHeader->OptionalHeader.DataDirectory[1].VirtualAddress;
	DWORD ImportDirectoryAdd_FOA = 0;
	ret = RVA_TO_FOA(ImportDirectoryAdd_RVA, &ImportDirectoryAdd_FOA, pFileAdd);
	if(ret != 0)
	{
		MessageBox(NULL, TEXT("func EnumDllName error"), TEXT("Error"), MB_OK);
		return ;
	}
	PIMAGE_IMPORT_DESCRIPTOR pImportDirectoory = (PIMAGE_IMPORT_DESCRIPTOR)((PBYTE)pFileAdd + ImportDirectoryAdd_FOA);

	//注意：必须设置mask
	LV_ITEM vitem;	
	memset(&vitem,0,sizeof(LV_ITEM));
	vitem.mask = LVIF_TEXT | LVIF_STATE;

	DWORD dwIdx = 0;
	//判断导入表是否结束
	while(pImportDirectoory->FirstThunk != 0 && pImportDirectoory->Name != 0)
	{
		DWORD DllName_RVA = pImportDirectoory->Name;
		DWORD DllName_FOA = 0;
		ret = RVA_TO_FOA(DllName_RVA, &DllName_FOA, pFileAdd);
		if(ret != 0)
		{
			MessageBox(NULL, TEXT("func EnumDllName error"), TEXT("Error"), MB_OK);
			return ;
		}

		//存储DLL名
		TCHAR *szDllName = (TCHAR *)((PBYTE)pFileAdd + DllName_FOA);

		TCHAR szOriginalFirstThunk[10] = {0};
		TCHAR szTimeDateStamp[10] = {0};
		TCHAR szForwarderChain[10] = {0};
		TCHAR szName[10] = {0};
		TCHAR szFirstThunk[10] = {0};

		//转换
		sprintf(szOriginalFirstThunk, "%08X", pImportDirectoory->OriginalFirstThunk);
		sprintf(szTimeDateStamp, "%08X", pImportDirectoory->TimeDateStamp);
		sprintf(szForwarderChain, "%08X", pImportDirectoory->ForwarderChain);
		sprintf(szName, "%08X", pImportDirectoory->Name);
		sprintf(szFirstThunk, "%08X", pImportDirectoory->FirstThunk);
		

		//打印
		vitem.pszText =szDllName;
		vitem.cchTextMax=sizeof(szDllName);
		vitem.iItem =dwIdx;
		vitem.iSubItem = 0;
	    ListView_InsertItem(hListDllList, &vitem);
		ListView_SetItemText(hListDllList, dwIdx, 1, szOriginalFirstThunk);
		ListView_SetItemText(hListDllList, dwIdx, 2, szTimeDateStamp);
		ListView_SetItemText(hListDllList, dwIdx, 3, szForwarderChain);
		ListView_SetItemText(hListDllList, dwIdx, 4, szName);
		ListView_SetItemText(hListDllList, dwIdx, 5, szFirstThunk);

		dwIdx++;
		pImportDirectoory++;
		
	}

	free(pFileAdd);
	return ;
}

VOID EnumApiName(HWND hListDllList, HWND hListApiList)
{
	//遍历导入表的Api名字时注意判断PIMAGE_THUNK_DATA的最高位是否是1
	//如果是1则是按序号导入
	//如果是0则是按名称导入，PIMAGE_THUNK_DATA是PIMAGE_IMPORT_BY_NAME的相对虚拟地址

	DWORD ret = 0;
	DWORD dwRowId;					//行号
	LV_ITEM lv;						//This structure that specifies or receives the attributes of a list view item.
	HWND hApiList;					//IDC_LIST_API_List的句柄
	ListView_DeleteAllItems(hListApiList);//打印前清空IDC_LIST_Api里面的内容

	//清空缓存
	memset(&lv,0,sizeof(LV_ITEM));
	lv.mask	= LVIF_TEXT | LVIF_STATE;

	//获取选择行
	dwRowId = SendMessage(hListDllList,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
	if(dwRowId == -1)
	{
		MessageBox(NULL,TEXT("Please select true DLL"),TEXT("Warning"),MB_OK);
		return;
	}

	//获取IDC_LIST_Moudle的句柄
	hApiList = GetDlgItem((HWND)hAppInstance,IDC_LIST_API_List);

	//初始化PE头，并进行导入模块定位
	
	//文件读取	
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if(ret != 0)
	{
		MessageBox(hListDllList, TEXT("File Read Error !"), TEXT("Error"), MB_OK);
		return;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的MZ标志"),TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	//判断是否是有效的PE 
	if(pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的PE标志"),TEXT("Error"),MB_OK);
		free(pFileAdd);
		return;
	}

	//获取导入表的RAV
	DWORD ImportDirectoryAdd_RVA = pNTHeader->OptionalHeader.DataDirectory[1].VirtualAddress;
	DWORD ImportDirectoryAdd_FOA = 0;
	ret = RVA_TO_FOA(ImportDirectoryAdd_RVA, &ImportDirectoryAdd_FOA, pFileAdd);
	if(ret != 0)
	{
		MessageBox(NULL, TEXT("func EnumDllName error"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return ;
	}
	PIMAGE_IMPORT_DESCRIPTOR pImportDirectoory = (PIMAGE_IMPORT_DESCRIPTOR)((PBYTE)pFileAdd + ImportDirectoryAdd_FOA + dwRowId * sizeof(IMAGE_IMPORT_DESCRIPTOR));
	

	//定位PIMAGE_THUNK_DATA
	DWORD ThunkINT_RVA = pImportDirectoory->OriginalFirstThunk;
	DWORD ThunkINT_FOA = 0;
	ret = RVA_TO_FOA(ThunkINT_RVA, &ThunkINT_FOA, pFileAdd);
	if(ret != 0)
	{
		MessageBox(NULL, TEXT("func EnumApiName error"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return ;
	}

	//定位打印行
	DWORD dwNum = 0;
	PIMAGE_THUNK_DATA pThunkINT = (PIMAGE_THUNK_DATA)((PBYTE)pFileAdd + ThunkINT_FOA);
	PIMAGE_IMPORT_BY_NAME pApiName = NULL;
	
	do
	{
		ThunkINT_FOA = (DWORD)pThunkINT - (DWORD)pFileAdd;
		ret = FOA_TO_RVA(ThunkINT_FOA, &ThunkINT_RVA, pFileAdd);
		if(ret != 0)
		{
			MessageBox(NULL, TEXT("func EnumApiName error"), TEXT("Error"), MB_OK);
			free(pFileAdd);
			return ;
		}

		TCHAR szThunkRAV[10] = {0};
		TCHAR szThunkOffset[10] = {0};
		TCHAR szThunkValue[10] = {0};
		TCHAR szHint[10] = {0};
		TCHAR *pName = NULL;


		if(pThunkINT->u1.Ordinal & 0x80000000)
		{
			//定位导入序号
			TCHAR szOrdinal[20] = {0};

			sprintf(szHint, "---");

			//去掉最高位获取序号偏移

			DWORD Ordinal = pThunkINT->u1.Ordinal & 0x7FFFFFFF;
			
			sprintf(szOrdinal, "Ordinal %Xh, %dd", Ordinal, Ordinal);
			pName = (TCHAR *)&szOrdinal;

		}
		else
		{

			//定位PIMAGE_IMPORT_BY_NAME
			DWORD ApiName_RAV = pThunkINT->u1.Ordinal;
			DWORD ApiName_FOA = 0;
			ret = RVA_TO_FOA(ApiName_RAV, &ApiName_FOA, pFileAdd);
			if(ret != 0)
			{
				MessageBox(NULL, TEXT("func EnumApiName error"), TEXT("Error"), MB_OK);
				return ;
			}
			pApiName = (PIMAGE_IMPORT_BY_NAME)((PBYTE)pFileAdd + ApiName_FOA);

			pName = (TCHAR *)pApiName->Name;
			sprintf(szHint, "%04X", pApiName->Hint);
			
		}

		sprintf(szThunkRAV, "%08X", ThunkINT_RVA);
		sprintf(szThunkOffset, "%08X", ThunkINT_FOA);
		sprintf(szThunkValue, "%08X", pThunkINT->u1.Ordinal);

		//打印
		lv.pszText =szThunkRAV;
		lv.cchTextMax=sizeof(szThunkRAV);
		lv.iItem =dwNum;
		lv.iSubItem = 0;
	    ListView_InsertItem(hListApiList, &lv);
		ListView_SetItemText(hListApiList, dwNum, 1, szThunkOffset);
		ListView_SetItemText(hListApiList, dwNum, 2, szThunkValue);
		ListView_SetItemText(hListApiList, dwNum, 3, szHint);
		ListView_SetItemText(hListApiList, dwNum, 4, pName);
		
		dwNum++;
		pThunkINT++;
	}while(pThunkINT->u1.AddressOfData != 0);

	free(pFileAdd);
	return ;
}


//遍历Blocks
VOID EnumBlocks(HWND hListBlocksList)
{
	DWORD ret = 0;
	DWORD dwRowId;					//行号
	LV_ITEM lv;						//This structure that specifies or receives the attributes of a list view item.
	HWND hBlocksList = GetDlgItem(hListBlocksList, IDC_LIST_Blocks);				//IDC_LIST_API_List的句柄
	ListView_DeleteAllItems(hBlocksList);//打印前清空IDC_LIST_Api里面的内容

	//清空缓存
	memset(&lv,0,sizeof(LV_ITEM));
	lv.mask	= LVIF_TEXT | LVIF_STATE;

	//获取选择行
	dwRowId = SendMessage(hBlocksList,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
	if(dwRowId == -1)
	{
		return ;
	}

	//初始化PE头，并进行导入模块定位
	
	//文件读取	
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if(ret != 0)
	{
		return;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的MZ标志"),TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	//判断是否是有效的PE 
	if(pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的PE标志"),TEXT("Error"),MB_OK);
		free(pFileAdd);
		return;
	}

	//获取重定位表的RAV
	DWORD RelocationDirectoryAdd_RVA = pNTHeader->OptionalHeader.DataDirectory[5].VirtualAddress;
	DWORD RelocationDirectoryAdd_FOA = 0;
	ret = RVA_TO_FOA(RelocationDirectoryAdd_RVA, &RelocationDirectoryAdd_FOA, pFileAdd);
	if(ret != 0)
	{
		MessageBox(NULL, TEXT("func EnumDllName error"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return ;
	}
	PIMAGE_BASE_RELOCATION pRelocationTable = (PIMAGE_BASE_RELOCATION)((PBYTE)pFileAdd + RelocationDirectoryAdd_FOA);

	DWORD dwNum = 0;
	while(pRelocationTable->SizeOfBlock != 0 && pRelocationTable->VirtualAddress != 0)
	{
		//定义
		TCHAR szIndex[20] = {0};
		TCHAR szSection[20] = {0};
		TCHAR szRVA[10] = {0};
		TCHAR szItems[20] = {0};

		ret = RAVLocaion(pRelocationTable->VirtualAddress, szSection, pFileAdd);
		if(ret != 0)
		{
			MessageBox(NULL, TEXT("Section is not Fond"), TEXT("Warning"), MB_OK);
			free(pFileAdd);
			return ;
		}
		//Item的数量
		DWORD dwItemsNumber = (pRelocationTable->SizeOfBlock - sizeof(pRelocationTable->VirtualAddress) - sizeof(pRelocationTable->SizeOfBlock) ) / sizeof(WORD);

		//转换
		sprintf(szIndex, "%d", dwNum);
		sprintf(szRVA, "%08X", pRelocationTable->VirtualAddress);
		sprintf(szItems, "%Xh / %dd", dwItemsNumber, dwItemsNumber);

		//打印
		lv.pszText =szIndex;
		lv.cchTextMax=sizeof(szIndex);
		lv.iItem =dwNum;
		lv.iSubItem = 0;
	    ListView_InsertItem(hListBlocksList, &lv);
		ListView_SetItemText(hListBlocksList, dwNum, 1, szSection);
		ListView_SetItemText(hListBlocksList, dwNum, 2, szRVA);
		ListView_SetItemText(hListBlocksList, dwNum, 3, szItems);

		//后移
		pRelocationTable = (PIMAGE_BASE_RELOCATION)((PBYTE)pRelocationTable + pRelocationTable->SizeOfBlock);
		dwNum++;
	}

	free(pFileAdd);
	return ;
}
//遍历BlockItem
VOID EnumBlockItem(HWND hListBlocksList, HWND hListBlockItemList)
{
	DWORD ret = 0;
	DWORD dwRowId = 0;					//行号
	LV_ITEM lv;						//This structure that specifies or receives the attributes of a list view item.
	HWND hItemList; 					//IDC_LIST_API_List的句柄
	ListView_DeleteAllItems(hListBlockItemList);//打印前清空IDC_LIST_Api里面的内容

	//清空缓存
	memset(&lv,0,sizeof(LV_ITEM));
	lv.mask	= LVIF_TEXT | LVIF_STATE;

	//获取选择行
	dwRowId = SendMessage(hListBlocksList,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
	if(dwRowId == -1)
	{
		return;
	}

	//获取IDC_LIST_BlockItem的句柄
	hItemList = GetDlgItem((HWND)hAppInstance,IDC_LIST_BlockItem);

	//初始化PE头，并进行导入模块定位
	
	//文件读取	
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if(ret != 0)
	{
		MessageBox(hListBlocksList, TEXT("File Read Error !"), TEXT("Error"), MB_OK);
		return;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的MZ标志"),TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	//判断是否是有效的PE 
	if(pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的PE标志"),TEXT("Error"),MB_OK);
		free(pFileAdd);
		return;
	}
	//获取重定位表的RAV
	DWORD RelocationDirectoryAdd_RVA = pNTHeader->OptionalHeader.DataDirectory[5].VirtualAddress;
	DWORD RelocationDirectoryAdd_FOA = 0;
	ret = RVA_TO_FOA(RelocationDirectoryAdd_RVA, &RelocationDirectoryAdd_FOA, pFileAdd);
	if(ret != 0)
	{
		MessageBox(NULL, TEXT("func EnumBlockItem error"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return ;
	}
	PIMAGE_BASE_RELOCATION pRelocationTable = (PIMAGE_BASE_RELOCATION)((PBYTE)pFileAdd + RelocationDirectoryAdd_FOA);

	//获取相应的Items
	DWORD dwNum = 0;
	while(dwNum < dwRowId)
	{
		pRelocationTable = (PIMAGE_BASE_RELOCATION)((PBYTE)pRelocationTable + pRelocationTable->SizeOfBlock);
		dwNum++;
	}
	
	//计算Item的数量
	DWORD dwItemsNumber = (pRelocationTable->SizeOfBlock - 2 * sizeof(DWORD)) / sizeof(WORD);
	WORD wItemSub_RAV = 0;
	dwNum = 0;

	while(dwNum < dwItemsNumber)
	{
		TCHAR szIndex[10] = {0};
		TCHAR szRVA[10] = {0};
		TCHAR szFOA[10] = {0};
		TCHAR szType[20] = {0};
		TCHAR szFar_Address[10] = {0};
		TCHAR szDataInformation[60] = {0};

		wItemSub_RAV = *(WORD*)((PBYTE)pRelocationTable + 2 * sizeof(DWORD) + dwNum * sizeof(WORD));

		//确定Item的类型
		if((wItemSub_RAV & 0xF000) == 0x3000)
		{	
			DWORD dwItemAdd_RVA = (DWORD)(pRelocationTable->VirtualAddress + (DWORD)(wItemSub_RAV & 0x0FFF));
			DWORD dwItemAdd_FOA = 0;
			ret = RVA_TO_FOA(dwItemAdd_RVA, &dwItemAdd_FOA, pFileAdd);
			if(ret != 0)
			{
				MessageBox(NULL, TEXT("func EnumBlockItem error"), TEXT("Error"), MB_OK);
				free(pFileAdd);
				return ;
			}

			//需要重定位的地址
			DWORD dwRelocationAddress = *(DWORD*)((PBYTE)pFileAdd + dwItemAdd_FOA);

			//根据重定位的地址获取重定位地址的内容的地址
			DWORD dwDataInformation_RVA = dwRelocationAddress - pNTHeader->OptionalHeader.ImageBase;
			DWORD dwDataInformation_FOA = 0;
			ret = RVA_TO_FOA(dwDataInformation_RVA, &dwDataInformation_FOA, pFileAdd);
			if(ret != 0)
			{
				MessageBox(NULL, TEXT("func EnumBlockItem error"), TEXT("Error"), MB_OK);
				free(pFileAdd);
				return ;
			}	
			
			//转换
			sprintf(szIndex, "%d", dwNum);
			sprintf(szRVA, "%08X", dwItemAdd_RVA);
			sprintf(szFOA, "%08X", dwItemAdd_FOA);
			sprintf(szType, "HIGHLOW [3]");
			sprintf(szFar_Address, "%08X", dwRelocationAddress);


			sprintf(szDataInformation, "%02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X",*(PBYTE)((PBYTE)pFileAdd + dwDataInformation_FOA + 0), *(PBYTE)((PBYTE)pFileAdd + dwDataInformation_FOA + 1),\
																			*(PBYTE)((PBYTE)pFileAdd + dwDataInformation_FOA + 2), *(PBYTE)((PBYTE)pFileAdd + dwDataInformation_FOA + 3),
																			*(PBYTE)((PBYTE)pFileAdd + dwDataInformation_FOA + 4), *(PBYTE)((PBYTE)pFileAdd + dwDataInformation_FOA + 5),
																			*(PBYTE)((PBYTE)pFileAdd + dwDataInformation_FOA + 6), *(PBYTE)((PBYTE)pFileAdd + dwDataInformation_FOA + 7),
																			*(PBYTE)((PBYTE)pFileAdd + dwDataInformation_FOA + 8), *(PBYTE)((PBYTE)pFileAdd + dwDataInformation_FOA + 9));

		}
		else if((wItemSub_RAV & 0xF000) == 0x0000)
		{
			//转换
			sprintf(szIndex, "%d", dwNum);
			sprintf(szRVA, "---");
			sprintf(szFOA, "---");
			sprintf(szType, "ABSOLUTE [0]");
			sprintf(szFar_Address, "---");
			sprintf(szDataInformation, "---");
		}

		//打印
		lv.pszText =szIndex;
		lv.cchTextMax=sizeof(szIndex);
		lv.iItem =dwNum;
		lv.iSubItem = 0;
	    ListView_InsertItem(hListBlockItemList, &lv);
		ListView_SetItemText(hListBlockItemList, dwNum, 1, szRVA);
		ListView_SetItemText(hListBlockItemList, dwNum, 2, szFOA);
		ListView_SetItemText(hListBlockItemList, dwNum, 3, szType);
		ListView_SetItemText(hListBlockItemList, dwNum, 4, szFar_Address);
		ListView_SetItemText(hListBlockItemList, dwNum, 5, szDataInformation);

		dwNum++;
	}

	free(pFileAdd);
	return ;
}

VOID EnumDebugTable(HWND hListDebugTable)
{
	TCHAR *szType[11] = {	"UNKNOWN [0]", "COFF [1]", "CODEVIEW [2]", "FPO [3]",
							"MISC [4]", "EXCEPTION [5]", "FIXUP [6]", "OMAP_TO_SRC [7]",
							"OMAP_FROM_SRC [8]", "BORLAND [9]", "RESERVED10 [10]"};

	DWORD ret = 0;
	//文件读取	
	LPVOID pFileAdd = NULL;	//文件的内存地址
	ret = ReadFile(&pFileAdd, szFileName);
	if(ret != 0)
	{
		MessageBox(hListDebugTable, TEXT("File Read Error !"), TEXT("Error"), MB_OK);
		return;
	}

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的MZ标志"),TEXT("Error"), MB_OK);
		free(pFileAdd);
		return;
	}
	//判断是否是有效的PE 
	if(pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(0,TEXT("不是有效的PE标志"),TEXT("Error"),MB_OK);
		free(pFileAdd);
		return;
	}

	//注意：必须设置mask
	LV_ITEM lv;	
	memset(&lv,0,sizeof(LV_ITEM));
	lv.mask	= LVIF_TEXT | LVIF_STATE;
	
	DWORD DebugTableAddress_RVA = pNTHeader->OptionalHeader.DataDirectory[6].VirtualAddress;
	DWORD DebugTableAddress_FOA = 0;
	ret = RVA_TO_FOA(DebugTableAddress_RVA, &DebugTableAddress_FOA, pFileAdd);
	if(ret != 0)
	{
		MessageBox(NULL, TEXT("func EnumDebugTable error"), TEXT("Error"), MB_OK);
		free(pFileAdd);
		return ;
	}
	PIMAGE_DEBUG_DIRECTORY pDebugTable = (PIMAGE_DEBUG_DIRECTORY)((PBYTE)pFileAdd + DebugTableAddress_FOA);

	DWORD dwNum = 0;
	while(dwNum < pNTHeader->OptionalHeader.DataDirectory[6].Size / sizeof(IMAGE_DEBUG_DIRECTORY))
	{
		TCHAR szCharacteristics[20] = {0};
		TCHAR szTimeDateStamp[20] = {0};
		TCHAR szMajorVersion[20] = {0};
		TCHAR szMinorVersion[20] = {0};
		TCHAR szSizeOfData[20] = {0};
		TCHAR szAddressOfRawData[20] = {0};
		TCHAR szPointerToRawData[20] = {0};

		wsprintf(szCharacteristics, "%08X", pDebugTable->Characteristics);
		wsprintf(szTimeDateStamp, "%08X", pDebugTable->TimeDateStamp);
		wsprintf(szMajorVersion, "%04X", pDebugTable->MajorVersion);
		wsprintf(szMinorVersion, "%08X", pDebugTable->MinorVersion);
		wsprintf(szSizeOfData, "%08X", pDebugTable->SizeOfData);
		wsprintf(szAddressOfRawData, "%08X", pDebugTable->AddressOfRawData);
		wsprintf(szPointerToRawData, "%08X", pDebugTable->PointerToRawData);

		//打印
		lv.pszText =szCharacteristics;
		lv.cchTextMax=sizeof(szCharacteristics);
		lv.iItem =dwNum;
		lv.iSubItem = 0;
	    ListView_InsertItem(hListDebugTable, &lv);
		ListView_SetItemText(hListDebugTable, dwNum, 1, szTimeDateStamp);
		ListView_SetItemText(hListDebugTable, dwNum, 2, szMajorVersion);
		ListView_SetItemText(hListDebugTable, dwNum, 3, szMinorVersion);
		if (pDebugTable->Type < 11)
		{
			ListView_SetItemText(hListDebugTable, dwNum, 4, szType[pDebugTable->Type]);
		}
		else
		{
			TCHAR szTypeNum[8] = { 0 };
			wsprintf(szTypeNum, "%04X", pDebugTable->Type);
			ListView_SetItemText(hListDebugTable, dwNum, 4, szTypeNum);
		}
		ListView_SetItemText(hListDebugTable, dwNum, 5, szSizeOfData);
		ListView_SetItemText(hListDebugTable, dwNum, 6, szAddressOfRawData);
		ListView_SetItemText(hListDebugTable, dwNum, 7, szPointerToRawData);

		pDebugTable++;
		dwNum++;
	}

	free(pFileAdd);
	return ;
}


//////////////////////////////////////////////////////////////////////////////////////////
////////&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&/////*****////
//////////////////////////////////////////////////////////////////////////////////////////

