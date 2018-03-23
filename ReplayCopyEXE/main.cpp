#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <string>
#include <direct.h>
#include <queue>
#include <stack>
#include <windows.h>
#include <Wincrypt.h>
#include <locale>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <time.h>

using namespace std;

#define BUFSIZE 1024
#define MD5LEN  16

int getMD5(std::string lpcTargetFilename, std::string &fileMD5hash);

std::wstring widen(std::string &s)
{
	std::wstring wsTmp(s.begin(), s.end());
	return wsTmp;
}

int main(int argc, char** argv) {

	// TODO : ��θ� �� ���ϸ� �ѱ� ���� ���

	// TODO : .rep ������ �ƴϸ� ������


	// ���÷��� ���ϵ��� �˻��� ���� ���
	std::string startingFolder;

	// ���÷��� ���ϵ� �� ��� ������ �����س��� ���� ����
	std::string destinationFolder;

	// ���� ����
	std::string nextFolder;

	//for (int i = 0; i < argc; i++) {
	//	printf("%s\n", argv[i]);
	//}

	if (argc == 3) {
		// TODO : argv[1], argv[2] �� �������� ���� ������� �ƴ��� üũ�Ѵ�
		startingFolder = argv[1];
		destinationFolder = argv[2];
	}
	else {
		return 0;
	}

	// ���� ���� ����� ���� ��� ����
	std::ofstream listFileStream;
	std::string listFileFullPath = destinationFolder + "\\list.csv";
	listFileStream.open(listFileFullPath.c_str(), std::ofstream::out);

	// �湮��� ���� stack -> ������ġ�κ��� �����ؼ� Depth First Search ������� �湮�Ѵ�
	std::stack<std::string> folders;

	// ���� ã�� �ڵ�
	long handle;
	
	// ���� ���� ����ü
	struct _finddata_t fileInfo;   

	std::string fileMD5hash;

	// 260�� �Ѵ� �н� �ٷ��
	char* _startingFolder = _getcwd(NULL, 0);
	startingFolder = _startingFolder;
	free(_startingFolder);

	// �湮 ��� ���� stack �� ���� ������ �߰��Ѵ�
	folders.push(startingFolder);
	printf("Copy all replay files from %s\n", startingFolder.c_str());

	char fromFileFullPath[_MAX_PATH];

	// stack �� ���� ���� �ִ� ���� ��η� �̵��Ѵ�
	while (folders.empty() == false) {
		std::string nextFolder = folders.top();
		folders.pop();

		_chdir(nextFolder.c_str());
		
		//printf("current visiting folder %s\n", nextFolder);

		// ���� ������ ���� �� �������� ����� ��� ���´�
		handle = _findfirst(string(nextFolder + "\\*.*").c_str(), &fileInfo);
		if (-1 == handle) {
			// No file in directory 
		}
		else {
			do {
				// ���� �����̸� �湮 ��� ���� stack �� �߰��Ѵ�
				if (_A_SUBDIR == fileInfo.attrib)
				{
					if (strcmp(fileInfo.name, ".") != 0 && strcmp(fileInfo.name, "..") != 0) {

						if (_fullpath(fromFileFullPath, fileInfo.name, _MAX_PATH) != NULL) {
							//printf("Add folder to visit : %s\n", fromFileFullPath);
							folders.push(fromFileFullPath);
						}
					}
				}
				// ���� ������ �ƴϸ� ���� ���縦 �ϰ�, ���� ��Ͽ� �߰��Ѵ�
				else
				{
					if (_fullpath(fromFileFullPath, fileInfo.name, _MAX_PATH) != NULL) {

						// �ű� ���� �̸� : ������ md5 �ؽð����� ���� https://msdn.microsoft.com/en-us/library/windows/desktop/aa382380(v=vs.85).aspx
						getMD5(fileInfo.name, fileMD5hash);
						
						printf("\nReplay File : %s \n", fromFileFullPath);
						printf("   MD5 hash : %s \n", fileMD5hash.c_str());

						if (fileMD5hash.size() > 0) {
							
							// ������ ���� �Ѵ�
							std::string destinationFileFullPath = destinationFolder + "\\" + fileMD5hash + ".rep";
							printf("Copy to %s \n", destinationFileFullPath.c_str());

							CopyFile(widen(string(fromFileFullPath)).c_str(), widen(destinationFileFullPath).c_str(), true);


							// ������ ������ ��¥ �� �ð� ���ڿ��� �����
							struct tm fileDateTime;
							_localtime64_s(&fileDateTime, &fileInfo.time_write);
							char fileDateString[11];
							char fileTimeString[9];
							sprintf_s(fileDateString, "%04d-%02d-%02d\0", (fileDateTime.tm_year + 1900), (fileDateTime.tm_mon + 1), fileDateTime.tm_mday);
							sprintf_s(fileTimeString, "%02d:%02d:%02d\0", fileDateTime.tm_hour, fileDateTime.tm_min, fileDateTime.tm_sec);

							// ���� ���.csv �� append �Ѵ�
							// . �׸� : startingFolder ��, ���� ���� ���, ���� ���ϸ�, ���� ���� �����Ͻ�, destinationFolder ��, �ű� ���� �̸�, ���� ũ��
							// . �߰���� : cout ���� �ٷιٷ�. flush
							listFileStream << startingFolder.c_str()
								<< "," << nextFolder.c_str()
								<< "," << fileInfo.name
								<< "," << fileDateString
								<< "," << fileTimeString
								<< "," << destinationFolder.c_str()
								<< "," << fileMD5hash + ".rep"
								<< "," << fileInfo.size
								<< "\n";
							listFileStream.flush();
						}

					}
				}
			} while (_findnext(handle, &fileInfo) == 0);
		}
	}

	_findclose(handle);  
	listFileStream.close();

	return 0;
}

int getMD5(std::string targetFileName, std::string &fileMD5hash) {
	DWORD dwStatus = 0;
	BOOL bResult = FALSE;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	HANDLE hFile = NULL;
	BYTE rgbFile[BUFSIZE];
	DWORD cbRead = 0;
	BYTE rgbHash[MD5LEN];
	DWORD cbHash = 0;
	CHAR rgbDigits[] = "0123456789abcdef";
	
	wstring wideTargetFileName(targetFileName.begin(), targetFileName.end());	
	LPCWSTR lpcTargetFilename = wideTargetFileName.c_str();	

	// Logic to check usage goes here.

	//printf("Get MD5 hash of file %s \n", targetFileName.c_str());

	hFile = CreateFile(lpcTargetFilename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		dwStatus = GetLastError();
		printf("Error opening file %s\nError: %d\n", lpcTargetFilename, dwStatus);
		return dwStatus;
	}

	// Get handle to the crypto provider
	if (!CryptAcquireContext(&hProv,
		NULL,
		NULL,
		PROV_RSA_FULL,
		CRYPT_VERIFYCONTEXT))
	{
		dwStatus = GetLastError();
		printf("CryptAcquireContext failed: %d\n", dwStatus);
		CloseHandle(hFile);
		return dwStatus;
	}

	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
	{
		dwStatus = GetLastError();
		printf("CryptAcquireContext failed: %d\n", dwStatus);
		CloseHandle(hFile);
		CryptReleaseContext(hProv, 0);
		return dwStatus;
	}

	while (bResult = ReadFile(hFile, rgbFile, BUFSIZE,
		&cbRead, NULL))
	{
		if (0 == cbRead)
		{
			break;
		}

		if (!CryptHashData(hHash, rgbFile, cbRead, 0))
		{
			dwStatus = GetLastError();
			printf("CryptHashData failed: %d\n", dwStatus);
			CryptReleaseContext(hProv, 0);
			CryptDestroyHash(hHash);
			CloseHandle(hFile);
			return dwStatus;
		}
	}

	if (!bResult)
	{
		dwStatus = GetLastError();
		printf("ReadFile failed: %d\n", dwStatus);
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		CloseHandle(hFile);
		return dwStatus;
	}

	cbHash = MD5LEN;
	if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
	{
		//printf("MD5 hash of file %s is : ", targetFileName.c_str());

		fileMD5hash = "";
		for (DWORD i = 0; i < cbHash; i++)
		{
			//printf("%c%c", rgbDigits[rgbHash[i] >> 4], rgbDigits[rgbHash[i] & 0xf]);

			fileMD5hash += rgbDigits[rgbHash[i] >> 4];
			fileMD5hash += rgbDigits[rgbHash[i] & 0xf];
		}
		//printf("\n");
	}
	else
	{
		dwStatus = GetLastError();
		printf("CryptGetHashParam failed: %d\n", dwStatus);
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	CloseHandle(hFile);

	return dwStatus;
}

