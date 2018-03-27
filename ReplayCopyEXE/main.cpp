#include "main.h"

int wmain(int argc, wchar_t** argv) {

	_setmode(_fileno(stdout), _O_U16TEXT);

	// ���÷��� ���ϵ��� �˻��� Source Folder ���
	std::wstring startingSourceFolderPath;

	// ���÷��� ���ϵ� �� ��� ������ �����س��� Destination Folder ���
	std::wstring destinationFolderPath;
	
	//for (int i = 0; i < argc; i++) {
	//	wprintf("%s\n", argv[i]);
	//}

	if (argc == 3) {
		// argv[1], argv[2] �� �������� ���� ������� �ƴ��� üũ�� �� ���� ��
		startingSourceFolderPath = argv[1];
		destinationFolderPath = argv[2];

		if (0 != _wchdir(startingSourceFolderPath.c_str())) {
			wprintf(L"Error in source folder path : %s\n", startingSourceFolderPath.c_str());
			return 0;
		}

		if (0 != _wchdir(destinationFolderPath.c_str())) {
			wprintf(L"Error in destination folder path : %s\n", destinationFolderPath.c_str());
			return 0;
		}

		wprintf(L"--- Starcraft : Broodwar Replay File Copy Program ---\n");
		wprintf(L"    From : %s\n", startingSourceFolderPath.c_str());
		wprintf(L"    To   : %s\n", destinationFolderPath.c_str());
	}
	else {
		wprintf(L"Usage : ReplayCopyEXE \"SOURCE_FOLDER_PATH\" \"DESTINATION_FOLDER_PATH\" \n");
		return 0;
	}

	// ���� ���� ����� ���� ��� ����
	std::wofstream listFileStream;
	std::wstring listFilePath = destinationFolderPath + L"\\list.csv";
	listFileStream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	listFileStream.open(listFilePath.c_str(), std::ofstream::out);

	// �湮��� ���� stack -> ������ġ�κ��� �����ؼ� Depth First Search ������� �湮�Ѵ�
	std::stack<std::wstring> folders;

	// ���� ���� ���
	std::wstring currentWorkingFolder;

	// Source File �� ��� + ���ϸ�
	wchar_t sourceFileFullPath[_MAX_PATH];

	// ���� ã�� �ڵ�
	long handle;
	
	// ���� ���� ����ü
	struct _wfinddata_t fileInfo;   

	// Source File �� MD5 Hash ��
	std::wstring sourceFileMD5HashString;

	// 260�� �Ѵ� �н� �ٷ��
	//wchar_t* _startingFolder = _wgetcwd(NULL, 0);
	//startingSourceFolderPath = _startingFolder;
	//free(_startingFolder);

	// �湮 ��� ���� stack �� Source ������ �߰��Ѵ�
	folders.push(startingSourceFolderPath);

	while (folders.empty() == false) {

		// stack �� ���� ���� �ִ� ���� ��θ� ����ͼ� �ű�� �̵��� �� 
		currentWorkingFolder = folders.top();
		folders.pop();
		if (0 != _wchdir(currentWorkingFolder.c_str())) {
			wprintf(L"Error in source folder path : %s\n", currentWorkingFolder.c_str());
			break;
		}

		wprintf(L"\nCurrent Visiting Folder : %s\n", currentWorkingFolder.c_str());
		
		// ���� ������ ���� �� �������� ����� ��� ���´�
		handle = _wfindfirst(wstring(currentWorkingFolder + L"\\*").c_str(), &fileInfo);
		if (-1 == handle) {
			// No file in directory 
		}
		else {
			do {
				// ���� �����̸� �湮 ��� ���� stack �� �߰��Ѵ�
				if (_A_SUBDIR == fileInfo.attrib)
				{
					if (wcscmp(fileInfo.name, L".") != 0 && wcscmp(fileInfo.name, L"..") != 0) {

						if (_wfullpath(sourceFileFullPath, fileInfo.name, _MAX_PATH) != NULL) {
							wprintf(L"Add folder to visit : %s\n", sourceFileFullPath);
							folders.push(sourceFileFullPath);
						}
					}
				}
				// ���� ������ �ƴϸ� ���� ���縦 �ϰ�, ���� ��Ͽ� �߰��Ѵ�
				else
				{
					if (_wfullpath(sourceFileFullPath, fileInfo.name, _MAX_PATH) != NULL) {

						if (NULL == wcsstr(fileInfo.name, L".rep")) {
							continue;
						}

						// �ű� ���� �̸� : Ÿ�ӽ�����
						time_t currentTime;
						time(&currentTime);
						struct _timeb timebuffer;
						_ftime64_s(&timebuffer);
						std::wstring newFileName = to_wstring(currentTime) + to_wstring(timebuffer.millitm);

						// ������ md5 �ؽð� ����ϱ� : https://msdn.microsoft.com/en-us/library/windows/desktop/aa382380(v=vs.85).aspx
						sourceFileMD5HashString = L"";
						getMD5(fileInfo.name, sourceFileMD5HashString);
						
						wprintf(L"\n");
						wprintf(L"File Name : %s\n", sourceFileFullPath);
						wprintf(L"MD5 Hash : %s\n", sourceFileMD5HashString.c_str());
						
						if (sourceFileMD5HashString.size() > 0) {
							
							// Destination File �̸�
							std::wstring destinationFileName = newFileName + L".rep";
							// Destination File ��ü��� + �����̸�
							std::wstring destinationFileFullPath = destinationFolderPath + L"\\" + destinationFileName;

							// ������ �����Ѵ�.  ������ ������ ������ �ִٸ�, ������ʰ� Fail �� ����Ѵ�							
							wprintf(L"Copy to : %s\n", destinationFileFullPath.c_str());							
							BOOL copyResult = CopyFile(wstring(sourceFileFullPath).c_str(), destinationFileFullPath.c_str(), true);
							std::wstring copyResultString;
							if (copyResult != 0) {
								copyResultString = L"1";
							}
							else {
								getLastErrorString(copyResultString);
								wprintf(L"Error : %s\n", copyResultString.c_str());
								copyResultString = L"0";
							}
							
							// ������ ������ ��¥ �� �ð� ���ڿ��� �����
							struct tm fileDateTime;
							_localtime64_s(&fileDateTime, &fileInfo.time_write);
							wchar_t fileDateString[11];
							wchar_t fileTimeString[9];
							swprintf_s(fileDateString, 11, L"%04d-%02d-%02d\0", (fileDateTime.tm_year + 1900), (fileDateTime.tm_mon + 1), fileDateTime.tm_mday);
							swprintf_s(fileTimeString, 9, L"%02d:%02d:%02d\0", fileDateTime.tm_hour, fileDateTime.tm_min, fileDateTime.tm_sec);

							// ���� ���.csv �� append �ϰ�, �ٷ� flush �Ѵ�
							// �׸� : startingSourceFolderPath ��, ���� ���� ���, ���� ���ϸ�, ���� ���� �����Ͻ�, destinationFolderPath ��, �ű� ���� �̸�, ���� ũ��, ���� �ؽ�, ������
							listFileStream << startingSourceFolderPath.c_str()
								<< "," << currentWorkingFolder.c_str()
								<< "," << fileInfo.name
								<< "," << fileDateString
								<< "," << fileTimeString
								<< "," << destinationFolderPath.c_str()
								<< "," << destinationFileName.c_str()
								<< "," << fileInfo.size
								<< "," << sourceFileMD5HashString.c_str()
								<< "," << copyResultString.c_str()
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

