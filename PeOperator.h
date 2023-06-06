#define _CRT_SECURE_NO_WARNINGS
#include "stdio.h"
#include "windows.h"

//文件的大小
DWORD FileLength(PDWORD dwLength, LPSTR szFileName);
//将RVA装换成FOA
DWORD RVA_TO_FOA(DWORD RVA, DWORD *FOA, LPVOID pFileAdd);
//将FOA装换成RVA
DWORD FOA_TO_RVA(DWORD FOA, DWORD *RVA, LPVOID pFileAdd);
//将文件读入到内存
DWORD ReadFile(LPVOID *pFileAdd,LPSTR szFileName);
DWORD ReadFile2(LPVOID *pFileAdd,LPSTR szFileName, DWORD dwDataLength/*新增节的大小*/);
//判断RAV属于哪一个节区
DWORD RAVLocaion(DWORD VirtualAddress, TCHAR *szSection, VOID *pFileAdd);
//增加一个新节在Shell上
DWORD AddNewSection(DWORD dwLength, VOID *pFileAdd, VOID *pDataFileAdd/*数据地址*/);
//文件对其
DWORD FileAlignment(DWORD dwFileAlignment, VOID *pFileAdd);
//内存对齐 返回值为对齐值
DWORD SectionAlignment(DWORD dwSectionAlignment, VOID *pFileAdd);
//将修改后的文件存盘
DWORD CreateNewFile(VOID *pFileAdd, LPSTR szNewFileName, DWORD dwLength);
//加密EXE文件缓存
DWORD EncryptExeFile(VOID *pDataFileAdd, DWORD dwExeLength);//二进制替换加密！
//修复重定位表
DWORD RecoveryRelocation(LPVOID pFileDataAdd, DWORD dwOldBase, DWORD dwNewBase);

//已拉伸的重定位
DWORD RecoveryRelocation2(LPVOID pFileDataAdd, DWORD dwOldBase, DWORD dwNewBase);