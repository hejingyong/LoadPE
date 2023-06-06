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
			//�ӿǺ���ļ���
			wsprintf(szNewExeName, "%s-out.exe", szExeName);
			if(!CreateShell())
			{
				MessageBox(hwndShell, TEXT("�ӿ���ɣ�"), TEXT("Seccess!"), MB_OK);
			}

			return true;

		case IDC_BUTTON_ShellPath:
			{
				//���ô��ļ�
				OPENFILENAME stOpenFile;
				//�ļ�������
				TCHAR szPeFileExt[200] =	"(*.exe)\0*.exe\0";
											//�趨������Ҫ�ÿ��ַ���β\0   ��ʽ��(*.exe)\0*.exe\0
				
				memset(szShellName, 0, MAX_PATH);
				memset(&stOpenFile, 0, sizeof(OPENFILENAME));

				stOpenFile.lStructSize = sizeof(OPENFILENAME);
				stOpenFile.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
				stOpenFile.hwndOwner = hwndShell;
				stOpenFile.lpstrFilter = szPeFileExt;
				stOpenFile.lpstrFile = szShellName;
				stOpenFile.nMaxFile = MAX_PATH;//����256�ֽ�/�ַ�
				
				//��ȡ�ļ�·��
				//The GetOpenFileName function creates an Open dialog box that lets the 
				//user specify the drive, directory, and the name of a file or set of files to open.
				if(GetOpenFileName(&stOpenFile))
				{ 
					HWND hwndShellPath = GetDlgItem(hwndShell, IDC_EDIT_ShellPath);
					SetWindowText(hwndShellPath, szShellName);
				}
				else
				{
					//û��ѡ���ļ�
					MessageBox(NULL, TEXT("Open File Failed"), TEXT("Error"), MB_OK);
				}
				return true;
			}
		case IDC_BUTTON_FilePath:
			{
				//���ô��ļ�
				OPENFILENAME stOpenFile;
				//�ļ�������
				TCHAR szPeFileExt[200] =	"(*.exe)\0*.exe\0(*.txt)\0*.txt\0";
											//�趨������Ҫ�ÿ��ַ���β\0   ��ʽ��(*.exe)\0*.exe\0
				
				memset(szExeName, 0, MAX_PATH);
				memset(&stOpenFile, 0, sizeof(OPENFILENAME));

				stOpenFile.lStructSize = sizeof(OPENFILENAME);
				stOpenFile.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
				stOpenFile.hwndOwner = hwndShell;
				stOpenFile.lpstrFilter = szPeFileExt;
				stOpenFile.lpstrFile = szExeName;
				stOpenFile.nMaxFile = MAX_PATH;//����256�ֽ�/�ַ�
				
				//��ȡ�ļ�·��
				//The GetOpenFileName function creates an Open dialog box that lets the 
				//user specify the drive, directory, and the name of a file or set of files to open.
				if(GetOpenFileName(&stOpenFile))
				{ 
					HWND hwndExePath = GetDlgItem(hwndShell, IDC_EDIT_FilePath);
					SetWindowText(hwndExePath, szExeName);
				}
				else
				{
					//û��ѡ���ļ�
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

	//��EXE�ļ����м���!
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
