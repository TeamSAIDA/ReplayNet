#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <tchar.h>
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
#include <locale>
#include <codecvt>
#include <time.h>
#include <fcntl.h>
#include <sys/timeb.h>

using namespace std;

#define _UNICODE

#define BUFSIZE 1024
#define MD5LEN  16

int getMD5(std::wstring lpcTargetFilename, std::wstring &fileMD5hash);

//std::wstring widen(std::string &s)
//{
//	std::wstring wsTmp(s.begin(), s.end());
//	return wsTmp;
//}

int wmain(int argc, wchar_t** argv) {

	_setmode(_fileno(stdout), _O_U16TEXT);

	// ���÷��� ���ϵ��� �˻��� ���� ���
	std::wstring startingFolder;

	// ���÷��� ���ϵ� �� ��� ������ �����س��� ���� ����
	std::wstring destinationFolder;

	// ���� ����
	std::wstring nextFolder;

	//for (int i = 0; i < argc; i++) {
	//	wprintf("%s\n", argv[i]);
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
	std::wofstream listFileStream;
	std::wstring listFileFullPath = destinationFolder + L"\\list.csv";
	listFileStream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	listFileStream.open(listFileFullPath.c_str(), std::ofstream::out);

	// �湮��� ���� stack -> ������ġ�κ��� �����ؼ� Depth First Search ������� �湮�Ѵ�
	std::stack<std::wstring> folders;

	// ���� ã�� �ڵ�
	long handle;
	
	// ���� ���� ����ü
	struct _wfinddata_t fileInfo;   

	std::wstring fileMD5hash;

	// 260�� �Ѵ� �н� �ٷ��
	wchar_t* _startingFolder = _wgetcwd(NULL, 0);
	startingFolder = _startingFolder;
	free(_startingFolder);

	// �湮 ��� ���� stack �� ���� ������ �߰��Ѵ�
	folders.push(startingFolder);
	wprintf(L"Copy all replay files from %s\n", startingFolder.c_str());
	
	wchar_t fromFileFullPath[_MAX_PATH];

	// stack �� ���� ���� �ִ� ���� ��η� �̵��Ѵ�
	while (folders.empty() == false) {
		std::wstring nextFolder = folders.top();
		folders.pop();

		_wchdir(nextFolder.c_str());
		
		wprintf(L"\nCurrent Visiting Folder : %s\n", nextFolder);
		
		// ���� ������ ���� �� �������� ����� ��� ���´�
		handle = _wfindfirst(wstring(nextFolder + L"\\*").c_str(), &fileInfo);
		if (-1 == handle) {
			// No file in directory 
		}
		else {
			do {
				// ���� �����̸� �湮 ��� ���� stack �� �߰��Ѵ�
				if (_A_SUBDIR == fileInfo.attrib)
				{
					if (wcscmp(fileInfo.name, L".") != 0 && wcscmp(fileInfo.name, L"..") != 0) {

						if (_wfullpath(fromFileFullPath, fileInfo.name, _MAX_PATH) != NULL) {
							wprintf(L"Add folder to visit : %s\n", fromFileFullPath);
							folders.push(fromFileFullPath);
						}
					}
				}
				// ���� ������ �ƴϸ� ���� ���縦 �ϰ�, ���� ��Ͽ� �߰��Ѵ�
				else
				{
					if (_wfullpath(fromFileFullPath, fileInfo.name, _MAX_PATH) != NULL) {

						if (NULL == wcsstr(fileInfo.name, L".rep")) {
							continue;
						}

						// �ű� ���� �̸� : Ÿ�ӽ�����
						time_t ltime;
						time(&ltime);
						struct _timeb timebuffer;
						_ftime64_s(&timebuffer);
						std::wstring newFileName = to_wstring(ltime) + to_wstring(timebuffer.millitm);

						// ������ md5 �ؽð� ����ϱ� : https://msdn.microsoft.com/en-us/library/windows/desktop/aa382380(v=vs.85).aspx
						getMD5(fileInfo.name, fileMD5hash);
						
						wprintf(L"\n");
						wprintf(L"File Name : %s\n", fromFileFullPath);
						wprintf(L"MD5 Hash : %s\n", fileMD5hash.c_str());
						
						if (fileMD5hash.size() > 0) {
							
							// ������ ���� �Ѵ�
							std::wstring destinationFileName = newFileName + L".rep";
							std::wstring destinationFileFullPath = destinationFolder + L"\\" + destinationFileName;
							wprintf(L"Copy to : %s\n", destinationFileFullPath.c_str());
							
							CopyFile(wstring(fromFileFullPath).c_str(), destinationFileFullPath.c_str(), true);


							// ������ ������ ��¥ �� �ð� ���ڿ��� �����
							struct tm fileDateTime;
							_localtime64_s(&fileDateTime, &fileInfo.time_write);
							wchar_t fileDateString[11];
							wchar_t fileTimeString[9];
							swprintf_s(fileDateString, 11, L"%04d-%02d-%02d\0", (fileDateTime.tm_year + 1900), (fileDateTime.tm_mon + 1), fileDateTime.tm_mday);
							swprintf_s(fileTimeString, 9, L"%02d:%02d:%02d\0", fileDateTime.tm_hour, fileDateTime.tm_min, fileDateTime.tm_sec);

							// ���� ���.csv �� append �Ѵ�
							// . �׸� : startingFolder ��, ���� ���� ���, ���� ���ϸ�, ���� ���� �����Ͻ�, destinationFolder ��, �ű� ���� �̸�, ���� ũ��, ���� �ؽ�
							// . �߰���� : cout ���� �ٷιٷ�. flush
							listFileStream << startingFolder.c_str()
								<< "," << nextFolder.c_str()
								<< "," << fileInfo.name
								<< "," << fileDateString
								<< "," << fileTimeString
								<< "," << destinationFolder.c_str()
								<< "," << destinationFileName.c_str()
								<< "," << fileInfo.size
								<< "," << fileMD5hash
								<< L"\n";
							listFileStream.flush();
						}

					}
				}
			} while (_wfindnext(handle, &fileInfo) == 0);
		}
	}

	_findclose(handle);  
	listFileStream.flush();
	listFileStream.close();

	return 0;
}

int getMD5(std::wstring targetFileName, std::wstring &fileMD5hash) {
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

	//wprintf(L"Get MD5 hash of file %s \n", targetFileName.c_str());

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
		wprintf(L"Error opening file %s\nError: %d\n", lpcTargetFilename, dwStatus);
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
		wprintf(L"CryptAcquireContext failed: %d\n", dwStatus);
		CloseHandle(hFile);
		return dwStatus;
	}

	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
	{
		dwStatus = GetLastError();
		wprintf(L"CryptAcquireContext failed: %d\n", dwStatus);
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
			wprintf(L"CryptHashData failed: %d\n", dwStatus);
			CryptReleaseContext(hProv, 0);
			CryptDestroyHash(hHash);
			CloseHandle(hFile);
			return dwStatus;
		}
	}

	if (!bResult)
	{
		dwStatus = GetLastError();
		wprintf(L"ReadFile failed: %d\n", dwStatus);
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		CloseHandle(hFile);
		return dwStatus;
	}

	cbHash = MD5LEN;
	if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
	{
		//wprintf("MD5 hash of file %s is : ", targetFileName.c_str());

		fileMD5hash = L"";
		for (DWORD i = 0; i < cbHash; i++)
		{
			//wprintf("%c%c", rgbDigits[rgbHash[i] >> 4], rgbDigits[rgbHash[i] & 0xf]);

			fileMD5hash += rgbDigits[rgbHash[i] >> 4];
			fileMD5hash += rgbDigits[rgbHash[i] & 0xf];
		}
		//wprintf("\n");
	}
	else
	{
		dwStatus = GetLastError();
		wprintf(L"CryptGetHashParam failed: %d\n", dwStatus);
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	CloseHandle(hFile);

	return dwStatus;
}

