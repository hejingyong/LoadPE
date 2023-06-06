# include "Shell.h"

BOOL CALLBACK ShellProc(
  HWND hwndShell,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
 )
{
	switch(uMsg)
	{
		
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_Exit:
			EndDialog(hwndShell, 0);
			return true;
		case IDC_BUTTON_CreateShell:
			//加壳后的文件名
			wsprintf(szNewExeName, "%s-out.exe", szExeName);
			if(!CreateShell())
			{
				MessageBox(hwndShell, TEXT("加壳完成！"), TEXT("Seccess!"), MB_OK);
			}

			return true;

		case IDC_BUTTON_ShellPath:
			{
				//设置打开文件
				OPENFILENAME stOpenFile;
				//文件过滤器
				TCHAR szPeFileExt[200] =	"(*.exe)\0*.exe\0";
											//设定过滤器要用空字符结尾\0   格式：(*.exe)\0*.exe\0
				
				memset(szShellName, 0, MAX_PATH);
				memset(&stOpenFile, 0, sizeof(OPENFILENAME));

				stOpenFile.lStructSize = sizeof(OPENFILENAME);
				stOpenFile.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
				stOpenFile.hwndOwner = hwndShell;
				stOpenFile.lpstrFilter = szPeFileExt;
				stOpenFile.lpstrFile = szShellName;
				stOpenFile.nMaxFile = MAX_PATH;//最少256字节/字符
				
				//获取文件路径
				//The GetOpenFileName function creates an Open dialog box that lets the 
				//user specify the drive, directory, and the name of a file or set of files to open.
				if(GetOpenFileName(&stOpenFile))
				{ 
					HWND hwndShellPath = GetDlgItem(hwndShell, IDC_EDIT_ShellPath);
					SetWindowText(hwndShellPath, szShellName);
				}
				else
				{
					//没有选择文件
					MessageBox(NULL, TEXT("Open File Failed"), TEXT("Error"), MB_OK);
				}
				return true;
			}
		case IDC_BUTTON_FilePath:
			{
				//设置打开文件
				OPENFILENAME stOpenFile;
				//文件过滤器
				TCHAR szPeFileExt[200] =	"(*.exe)\0*.exe\0(*.txt)\0*.txt\0";
											//设定过滤器要用空字符结尾\0   格式：(*.exe)\0*.exe\0
				
				memset(szExeName, 0, MAX_PATH);
				memset(&stOpenFile, 0, sizeof(OPENFILENAME));

				stOpenFile.lStructSize = sizeof(OPENFILENAME);
				stOpenFile.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
				stOpenFile.hwndOwner = hwndShell;
				stOpenFile.lpstrFilter = szPeFileExt;
				stOpenFile.lpstrFile = szExeName;
				stOpenFile.nMaxFile = MAX_PATH;//最少256字节/字符
				
				//获取文件路径
				//The GetOpenFileName function creates an Open dialog box that lets the 
				//user specify the drive, directory, and the name of a file or set of files to open.
				if(GetOpenFileName(&stOpenFile))
				{ 
					HWND hwndExePath = GetDlgItem(hwndShell, IDC_EDIT_FilePath);
					SetWindowText(hwndExePath, szExeName);
				}
				else
				{
					//没有选择文件
					MessageBox(NULL, TEXT("Open File Failed"), TEXT("Error"), MB_OK);
				}
				return true;
			}
		}
		return true;
		
	case WM_CLOSE:
		EndDialog(hwndShell, 0);
		return true;
	}
	return false;
}


DWORD CreateShell()
{
	DWORD ret = 0;
	DWORD dwExeLength = 0;
	DWORD dwShellLength = 0;
	VOID *pFileAdd = NULL;
	VOID *pDataFileAdd = NULL;
	
	ret = FileLength(&dwExeLength, szExeName);
	if(ret != 0)
	{
		return ret;
	}

	ret = FileLength(&dwShellLength, szShellName);
	if(ret != 0)
	{
		return ret;
	}

	ret = ReadFile2(&pFileAdd, szShellName, dwExeLength);
	if(ret != 0)
	{
		return ret;
	}
	ret = ReadFile(&pDataFileAdd, szExeName);
	if(ret != 0)
	{
		return ret;
	}

	//对EXE文件进行加密!
	ret = EncryptExeFile(pDataFileAdd, dwExeLength);
	if(ret != 0)
	{
		return ret;
	}
	ret = AddNewSection(dwExeLength, pFileAdd, pDataFileAdd);
	if(ret != 0)
	{
		return ret;
	}
	ret = CreateNewFile(pFileAdd, szNewExeName, (dwShellLength + dwExeLength));
	if(ret != 0)
	{
		return ret;
	}
	free(pFileAdd);
	free(pDataFileAdd);
	return ret;
}
