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
#include <strsafe.h>
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

int getMD5(std::wstring targetFileName, std::wstring &sourceFileMD5HashString) {
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

		sourceFileMD5HashString = L"";
		for (DWORD i = 0; i < cbHash; i++)
		{
			//wprintf("%c%c", rgbDigits[rgbHash[i] >> 4], rgbDigits[rgbHash[i] & 0xf]);

			sourceFileMD5HashString += rgbDigits[rgbHash[i] >> 4];
			sourceFileMD5HashString += rgbDigits[rgbHash[i] & 0xf];
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


//std::wstring widen(std::string &s)
//{
//	std::wstring wsTmp(s.begin(), s.end());
//	return wsTmp;
//}


void getLastErrorString(std::wstring &errorReason)
{
	// Retrieve the system error message for the last-error code
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms680582(v=vs.85).aspx
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	errorReason = L"";
	errorReason.append((LPTSTR)lpMsgBuf);

	LocalFree(lpMsgBuf);
	ExitProcess(dw);
}