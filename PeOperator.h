#define _CRT_SECURE_NO_WARNINGS
#include "stdio.h"
#include "windows.h"

//�ļ��Ĵ�С
DWORD FileLength(PDWORD dwLength, LPSTR szFileName);
//��RVAװ����FOA
DWORD RVA_TO_FOA(DWORD RVA, DWORD *FOA, LPVOID pFileAdd);
//��FOAװ����RVA
DWORD FOA_TO_RVA(DWORD FOA, DWORD *RVA, LPVOID pFileAdd);
//���ļ����뵽�ڴ�
DWORD ReadFile(LPVOID *pFileAdd,LPSTR szFileName);
DWORD ReadFile2(LPVOID *pFileAdd,LPSTR szFileName, DWORD dwDataLength/*�����ڵĴ�С*/);
//�ж�RAV������һ������
DWORD RAVLocaion(DWORD VirtualAddress, TCHAR *szSection, VOID *pFileAdd);
//����һ���½���Shell��
DWORD AddNewSection(DWORD dwLength, VOID *pFileAdd, VOID *pDataFileAdd/*���ݵ�ַ*/);
//�ļ�����
DWORD FileAlignment(DWORD dwFileAlignment, VOID *pFileAdd);
//�ڴ���� ����ֵΪ����ֵ
DWORD SectionAlignment(DWORD dwSectionAlignment, VOID *pFileAdd);
//���޸ĺ���ļ�����
DWORD CreateNewFile(VOID *pFileAdd, LPSTR szNewFileName, DWORD dwLength);
//����EXE�ļ�����
DWORD EncryptExeFile(VOID *pDataFileAdd, DWORD dwExeLength);//�������滻���ܣ�
//�޸��ض�λ��
DWORD RecoveryRelocation(LPVOID pFileDataAdd, DWORD dwOldBase, DWORD dwNewBase);

//��������ض�λ
DWORD RecoveryRelocation2(LPVOID pFileDataAdd, DWORD dwOldBase, DWORD dwNewBase);