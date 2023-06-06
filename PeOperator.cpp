#include "PeOperator.h"

DWORD FileLength(PDWORD dwLength, LPSTR szFileName)
{
	DWORD ret = 0;
	FILE* pf = fopen(szFileName, "rb");
	if(pf == NULL)
	{
		ret = -2;
		MessageBox(NULL, TEXT("Filelength fopen error!"), TEXT("Error"), MB_OK);
		return ret;
	}

	fseek(pf, 0, SEEK_END);
	(*dwLength) = ftell(pf);
	fseek(pf, 0, SEEK_SET);

	fclose(pf);
	return ret;
}


DWORD RVA_TO_FOA(DWORD RVA, DWORD *FOA, LPVOID pFileAdd)
{
	DWORD ret = 0;

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		ret = -3;
		MessageBox(0,TEXT("不是有效的MZ标志"),TEXT("Error"), MB_OK);
		free(pFileAdd);
		return ret;
	}
	//判断是否是有效的PE 
	if(pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		ret = -3;
		MessageBox(0,TEXT("不是有效的PE标志"),TEXT("Error"),MB_OK);
		free(pFileAdd);
		return ret;
	}

	//RVA的地址在PE头内
	if(RVA < pSectionGroup->VirtualAddress)
	{
		*FOA = RVA;
	}
	else
	{	//RAV的地址在节区内
		for(DWORD dwNum = 0; dwNum < pNTHeader->FileHeader.NumberOfSections; dwNum++)
		{
			if(RVA >= pSectionGroup->VirtualAddress && RVA < pSectionGroup->VirtualAddress + pSectionGroup->Misc.VirtualSize)
			{
				*FOA = RVA - pSectionGroup->VirtualAddress + pSectionGroup->PointerToRawData;
			}
			pSectionGroup++;
		}
	}

	return ret;
}


DWORD FOA_TO_RVA(DWORD FOA, DWORD *RVA, LPVOID pFileAdd)
{
	DWORD ret = 0;

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断MZ标志
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		ret = -3;
		MessageBox(0,TEXT("不是有效的MZ标志"),TEXT("Error"), MB_OK);
		free(pFileAdd);
		return ret;
	}
	//判断是否是有效的PE 
	if(pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		ret = -3;
		MessageBox(0,TEXT("不是有效的PE标志"),TEXT("Error"),MB_OK);
		free(pFileAdd);
		return ret;
	}

	if(FOA < pSectionGroup->PointerToRawData)
	{
		*RVA = FOA;
	}
	else
	{
		for(DWORD dwNum = 0; dwNum < pNTHeader->FileHeader.NumberOfSections; dwNum++)
		{
			if(FOA >= pSectionGroup->PointerToRawData && FOA < pSectionGroup->PointerToRawData + pSectionGroup->SizeOfRawData)
			{
				*RVA = FOA - pSectionGroup->PointerToRawData + pSectionGroup->VirtualAddress;
			}
			pSectionGroup++;
		}
	}

	return ret;
}


DWORD ReadFile(LPVOID *pFileAdd,LPSTR szFileName)
{
	DWORD ret = 0;
	DWORD dwFileSize = 0;
	LPVOID pAdd = NULL;

	FILE* pFile = fopen(szFileName, "rb");
	if(pFile == NULL)
	{
		ret = -2;
		MessageBox(NULL, TEXT("ReadFile fopen error!"), TEXT("Error"), MB_OK);
		return ret;
	}

	ret = FileLength(&dwFileSize, szFileName);
	if(ret != 0)
	{
		MessageBox(NULL, TEXT("ReadFile FileLength error"), TEXT("Error"), MB_OK);
		return ret;
	}

	//分配缓冲区
	pAdd = malloc(dwFileSize);
	if(pFile == NULL)
	{
		ret = -2;
		MessageBox(NULL, TEXT("ReadFile malloc error"), TEXT("Error"), MB_OK);
		return ret;
	}
	memset(pAdd, 0, dwFileSize);

	//读取文件
	fread(pAdd, dwFileSize, 1, pFile);

	fclose(pFile);
	*pFileAdd = pAdd;
	return ret;
}

DWORD ReadFile2(LPVOID *pFileAdd,LPSTR szFileName, DWORD dwDataLength)
{
	DWORD ret = 0;
	DWORD dwFileSize = 0;
	LPVOID pAdd = NULL;

	FILE* pFile = fopen(szFileName, "rb");
	if(pFile == NULL)
	{
		ret = -2;
		MessageBox(NULL, TEXT("ReadFile fopen error!"), TEXT("Error"), MB_OK);
		return ret;
	}

	ret = FileLength(&dwFileSize, szFileName);
	if(ret != 0)
	{
		MessageBox(NULL, TEXT("ReadFile FileLength error"), TEXT("Error"), MB_OK);
		return ret;
	}

	//分配缓冲区
	pAdd = malloc(dwFileSize+dwDataLength);
	if(pFile == NULL)
	{
		ret = -2;
		MessageBox(NULL, TEXT("ReadFile malloc error"), TEXT("Error"), MB_OK);
		return ret;
	}
	memset(pAdd, 0, dwFileSize+dwDataLength);

	//读取文件
	fread(pAdd, dwFileSize, 1, pFile);

	fclose(pFile);
	*pFileAdd = pAdd;
	return ret;
}

DWORD RAVLocaion(DWORD VirtualAddress, TCHAR *szSection, VOID *pFileAdd)
{
	DWORD ret = 0;

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);


	for(DWORD dwNum = 0; dwNum < pNTHeader->FileHeader.NumberOfSections; dwNum++)
	{
		if(VirtualAddress >= pSectionGroup->VirtualAddress && VirtualAddress < pSectionGroup->VirtualAddress + pSectionGroup->Misc.VirtualSize)
		{
			sprintf(szSection, "%d[“%s”]", dwNum, pSectionGroup->Name);
			return ret;
		}
	pSectionGroup++;
	}
	ret = -1;
	return ret;
}

DWORD FileAlignment(DWORD dwFileAlignment, VOID *pFileAdd)
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);

	DWORD retAlignment = 0;
	if(dwFileAlignment % pNTHeader->OptionalHeader.FileAlignment)
	{
		retAlignment = (dwFileAlignment / pNTHeader->OptionalHeader.FileAlignment) * pNTHeader->OptionalHeader.FileAlignment + pNTHeader->OptionalHeader.FileAlignment;
	}
	else
	{
		retAlignment = dwFileAlignment;
	}

	return retAlignment;
}

DWORD SectionAlignment(DWORD dwSectionAlignment, VOID *pFileAdd)
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);

	DWORD retAlignment = 0;
	if(dwSectionAlignment % pNTHeader->OptionalHeader.SectionAlignment)
	{
		retAlignment = (dwSectionAlignment / pNTHeader->OptionalHeader.SectionAlignment) * pNTHeader->OptionalHeader.SectionAlignment + pNTHeader->OptionalHeader.SectionAlignment;
	}
	else
	{
		retAlignment = dwSectionAlignment;
	}

	return retAlignment;
}


DWORD AddNewSection(DWORD dwLength, VOID *pFileAdd, VOID *pDataFileAdd)
{
	DWORD ret = 0;

	//初始化PE头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileAdd + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PBYTE)pNTHeader + sizeof(IMAGE_FILE_HEADER) + pNTHeader->FileHeader.SizeOfOptionalHeader + 0x04);

	//判断是否有足够空间添加新节表
	DWORD dwSpace = pNTHeader->OptionalHeader.SizeOfHeaders - pDosHeader->e_lfanew - sizeof(IMAGE_NT_HEADERS) - sizeof(IMAGE_SECTION_HEADER) * pNTHeader->FileHeader.NumberOfSections;
	if(dwSpace <= 2*sizeof(IMAGE_SECTION_HEADER))
	{
		MessageBox(NULL, TEXT("AddNewSection Space Not Enough！"), TEXT("Wrong"), MB_OK);
		ret = -1;
		return ret;
	}

	//定位节表指针  设定新节表的值
	PIMAGE_SECTION_HEADER pLastSection = (PIMAGE_SECTION_HEADER)((PBYTE)pSectionGroup + sizeof(IMAGE_SECTION_HEADER) * (pNTHeader->FileHeader.NumberOfSections-1));
	PIMAGE_SECTION_HEADER pNewSection = (PIMAGE_SECTION_HEADER)((PBYTE)pSectionGroup + sizeof(IMAGE_SECTION_HEADER) * pNTHeader->FileHeader.NumberOfSections);


	PDWORD pVirtualAddress	 = &pNewSection->VirtualAddress;
	PDWORD pVirtualSize		 = &pNewSection->Misc.VirtualSize;
	PDWORD pPointerToRawData = &pNewSection->PointerToRawData;
	PDWORD pSizeOfRawData	 = &pNewSection->SizeOfRawData;
	PDWORD pCharacteristics	 = &pNewSection->Characteristics;

	memcpy(&pNewSection->Name, TEXT("NewSec"), 8);
	*pVirtualAddress = SectionAlignment(pLastSection->VirtualAddress + pLastSection->Misc.VirtualSize, pFileAdd);
	*pVirtualSize = dwLength; 
	*pSizeOfRawData = dwLength;
	*pPointerToRawData = FileAlignment(pLastSection->PointerToRawData + pLastSection->SizeOfRawData, pFileAdd);
	*pCharacteristics = 0xE00000E0;


	PDWORD pSizeOfImage = &pNTHeader->OptionalHeader.SizeOfImage;
	PWORD pNumberOfSection = &pNTHeader->FileHeader.NumberOfSections;
	
	*pNumberOfSection = *pNumberOfSection+1;
	*pSizeOfImage = SectionAlignment(pNTHeader->OptionalHeader.SizeOfImage + dwLength, pFileAdd);
	
	memcpy(((PBYTE)pFileAdd + pNewSection->PointerToRawData), pDataFileAdd, dwLength);

	return ret;
}

DWORD CreateNewFile(VOID *pFileAdd, LPSTR szNewFileName, DWORD dwLength)
{
	DWORD ret = 0;

	FILE* pf = fopen(szNewFileName, "wb");
	if(pf == NULL)
	{
		ret = -2;
		MessageBox(NULL, TEXT("CreateNewFile fopen error!"), TEXT("Error"), MB_OK);
		return ret;
	}

	fwrite(pFileAdd, dwLength, 1, pf);
	fclose(pf);
	return ret;
}

DWORD EncryptExeFile(VOID *pDataFileAdd, DWORD dwExeLength)
{
	DWORD ret = 0;

	BYTE PassWord[0x100] = {0x08, 0x01, 0x0B, 0x03, 0x0D, 0x05, 0x0F, 0x09,
							0x00, 0x07, 0x0A, 0x02, 0x0C, 0x04, 0x0E, 0x06,

							0x18, 0x11, 0x1B, 0x13, 0x1D, 0x15, 0x1F, 0x19,
							0x10, 0x17, 0x1A, 0x12, 0x1C, 0x14, 0x1E, 0x16,

							0x28, 0x21, 0x2B, 0x23, 0x2D, 0x25, 0x2F, 0x29,
							0x20, 0x27, 0x2A, 0x22, 0x2C, 0x24, 0x2E, 0x26,

							0x38, 0x31, 0x3B, 0x33, 0x3D, 0x35, 0x3F, 0x39,
							0x30, 0x37, 0x3A, 0x32, 0x3C, 0x34, 0x3E, 0x36,

							0x48, 0x41, 0x4B, 0x43, 0x4D, 0x45, 0x4F, 0x49,
							0x40, 0x47, 0x4A, 0x42, 0x4C, 0x44, 0x4E, 0x46,

							0x58, 0x51, 0x5B, 0x53, 0x5D, 0x55, 0x5F, 0x59,
							0x50, 0x57, 0x5A, 0x52, 0x5C, 0x54, 0x5E, 0x56,

							0x68, 0x61, 0x6B, 0x63, 0x6D, 0x65, 0x6F, 0x69,
							0x60, 0x67, 0x6A, 0x62, 0x6C, 0x64, 0x6E, 0x66,

							0x78, 0x71, 0x7B, 0x73, 0x7D, 0x75, 0x7F, 0x79,
							0x70, 0x77, 0x7A, 0x72, 0x7C, 0x74, 0x7E, 0x76,

							0x88, 0x81, 0x8B, 0x83, 0x8D, 0x85, 0x8F, 0x89,
							0x80, 0x87, 0x8A, 0x82, 0x8C, 0x84, 0x8E, 0x86,

							0x98, 0x91, 0x9B, 0x93, 0x9D, 0x95, 0x9F, 0x99,
							0x90, 0x97, 0x9A, 0x92, 0x9C, 0x94, 0x9E, 0x96,

							0xA8, 0xA1, 0xAB, 0xA3, 0xAD, 0xA5, 0xAF, 0xA9,
							0xA0, 0xA7, 0xAA, 0xA2, 0xAC, 0xA4, 0xAE, 0xA6,

							0xB8, 0xB1, 0xBB, 0xB3, 0xBD, 0xB5, 0xBF, 0xB9,
							0xB0, 0xB7, 0xBA, 0xB2, 0xBC, 0xB4, 0xBE, 0xB6,

							0xC8, 0xC1, 0xCB, 0xC3, 0xCD, 0xC5, 0xCF, 0xC9,
							0xC0, 0xC7, 0xCA, 0xC2, 0xCC, 0xC4, 0xCE, 0xC6,

							0xD8, 0xD1, 0xDB, 0xD3, 0xDD, 0xD5, 0xDF, 0xD9,
							0xD0, 0xD7, 0xDA, 0xD2, 0xDC, 0xD4, 0xDE, 0xD6,

							0xE8, 0xE1, 0xEB, 0xE3, 0xED, 0xE5, 0xEF, 0xE9,
							0xE0, 0xE7, 0xEA, 0xE2, 0xEC, 0xE4, 0xEE, 0xE6,

							0xF8, 0xF1, 0xFB, 0xF3, 0xFD, 0xF5, 0xFF, 0xF9,
							0xF0, 0xF7, 0xFA, 0xF2, 0xFC, 0xF4, 0xFE, 0xF6};

	for(DWORD dwNum = 0; dwNum < dwExeLength; dwNum++)
	{
		PBYTE pbData = (PBYTE)((PBYTE)pDataFileAdd + dwNum);
		*pbData = PassWord[*((PBYTE)pDataFileAdd + dwNum)];
	}

	return ret;
}

//没有拉伸的重定位
DWORD RecoveryRelocation(LPVOID pFileDataAdd, DWORD dwOldBase, DWORD dwNewBase)
{
	DWORD ret = 0;

	//初始化指针
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileDataAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileDataAdd + pDosHeader->e_lfanew);
	
	//指向重定位表
	DWORD dwRelocationAdd_RVA = pNTHeader->OptionalHeader.DataDirectory[5].VirtualAddress;
	DWORD dwRelocationAdd_FOA = 0;
	ret = RVA_TO_FOA(dwRelocationAdd_RVA, &dwRelocationAdd_FOA, pFileDataAdd);
	if(ret != 0)
	{
		MessageBox(NULL, TEXT("RecoveryRelocation RVA_TO_FOA error!"), TEXT("Error"), MB_OK);
		return ret;
	}
	PIMAGE_BASE_RELOCATION pRelocationTable = (PIMAGE_BASE_RELOCATION)((PBYTE)pFileDataAdd + dwRelocationAdd_FOA);


	while (pRelocationTable->VirtualAddress != 0 && pRelocationTable->SizeOfBlock != 0)
	{
		//Item的数量
		DWORD dwNumItem = (pRelocationTable->SizeOfBlock - 2 * sizeof(DWORD)) / sizeof(WORD);
		for(DWORD i = 0; i < dwNumItem; i++)
		{
			WORD wDataAdd_RVA = *(WORD *)((BYTE *)pRelocationTable + 0x8 + sizeof(WORD) * i);
			
			//判断数据是否有效
			if((wDataAdd_RVA & 0xF000) == 0x3000)
			{
				DWORD dwDataAdd_RVA = (DWORD)(pRelocationTable->VirtualAddress + ((DWORD)wDataAdd_RVA & 0x0FFF));
				DWORD dwDataAdd_FOA = 0;
				ret = RVA_TO_FOA(dwDataAdd_RVA, &dwDataAdd_FOA, pFileDataAdd);
				if (ret != 0)
				{
					MessageBox(NULL, TEXT("RecoveryRelocation DataAdd_RVA RVA_TO_FOA error!"), TEXT("Error"), MB_OK);
					return ret;
				}
				//需要重定位的地址
				PDWORD dwRelocationAddress = (PDWORD)((PBYTE)pFileDataAdd + dwDataAdd_FOA);
				//修复
				*dwRelocationAddress = ((*dwRelocationAddress) - dwOldBase + dwNewBase);
			}
		}
		pRelocationTable = (PIMAGE_BASE_RELOCATION)((PBYTE)pRelocationTable + pRelocationTable->SizeOfBlock);
	}
	return ret;
}


//已拉伸的重定位
DWORD RecoveryRelocation2(LPVOID pFileDataAdd, DWORD dwOldBase, DWORD dwNewBase)
{
	DWORD ret = 0;

	//初始化指针
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileDataAdd;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pFileDataAdd + pDosHeader->e_lfanew);

	//指向重定位表
	DWORD dwRelocationAdd_RVA = pNTHeader->OptionalHeader.DataDirectory[5].VirtualAddress;
	
	PIMAGE_BASE_RELOCATION pRelocationTable = (PIMAGE_BASE_RELOCATION)((PBYTE)pFileDataAdd + dwRelocationAdd_RVA);


	while (pRelocationTable->VirtualAddress != 0 && pRelocationTable->SizeOfBlock != 0)
	{
		//Item的数量
		DWORD dwNumItem = (pRelocationTable->SizeOfBlock - 2 * sizeof(DWORD)) / sizeof(WORD);
		for (DWORD i = 0; i < dwNumItem; i++)
		{
			WORD wDataAdd_RVA = *(WORD *)((BYTE *)pRelocationTable + 0x8 + sizeof(WORD) * i);

			//判断数据是否有效
			if ((wDataAdd_RVA & 0xF000) == 0x3000)
			{
				DWORD dwDataAdd_RVA = (DWORD)(pRelocationTable->VirtualAddress + ((DWORD)wDataAdd_RVA & 0x0FFF));
				
				//需要重定位的地址
				PDWORD dwRelocationAddress = (PDWORD)((PBYTE)pFileDataAdd + dwDataAdd_RVA);
				//修复
				*dwRelocationAddress = ((*dwRelocationAddress) - dwOldBase + dwNewBase);
			}
		}
		pRelocationTable = (PIMAGE_BASE_RELOCATION)((PBYTE)pRelocationTable + pRelocationTable->SizeOfBlock);
	}
	return ret;
}