#include "main.h"

int wmain(int argc, wchar_t** argv) {

	_setmode(_fileno(stdout), _O_U16TEXT);

	// 리플레이 파일들을 검색할 Source Folder 경로
	std::wstring startingSourceFolderPath;

	// 리플레이 파일들 및 목록 파일을 복사해넣을 Destination Folder 경로
	std::wstring destinationFolderPath;
	
	//for (int i = 0; i < argc; i++) {
	//	wprintf("%s\n", argv[i]);
	//}

	if (argc == 3) {
		// argv[1], argv[2] 가 정상적인 폴더 경로인지 아닌지 체크한 후 폴더 다
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

	// 파일 복사 기록을 적을 목록 파일
	std::wofstream listFileStream;
	std::wstring listFilePath = destinationFolderPath + L"\\list.csv";
	listFileStream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	listFileStream.open(listFilePath.c_str(), std::ofstream::out);

	// 방문대상 폴더 stack -> 현재위치로부터 시작해서 Depth First Search 방식으로 방문한다
	std::stack<std::wstring> folders;

	// 현재 폴더 경로
	std::wstring currentWorkingFolder;

	// Source File 의 경로 + 파일명
	wchar_t sourceFileFullPath[_MAX_PATH];

	// 파일 찾기 핸들
	long handle;
	
	// 파일 정보 구조체
	struct _wfinddata_t fileInfo;   

	// Source File 의 MD5 Hash 값
	std::wstring sourceFileMD5HashString;

	// 260자 넘는 패스 다루기
	//wchar_t* _startingFolder = _wgetcwd(NULL, 0);
	//startingSourceFolderPath = _startingFolder;
	//free(_startingFolder);

	// 방문 대상 폴더 stack 에 Source 폴더를 추가한다
	folders.push(startingSourceFolderPath);

	while (folders.empty() == false) {

		// stack 의 제일 위에 있는 폴더 경로를 갖고와서 거기로 이동한 후 
		currentWorkingFolder = folders.top();
		folders.pop();
		if (0 != _wchdir(currentWorkingFolder.c_str())) {
			wprintf(L"Error in source folder path : %s\n", currentWorkingFolder.c_str());
			break;
		}

		wprintf(L"\nCurrent Visiting Folder : %s\n", currentWorkingFolder.c_str());
		
		// 현재 폴더의 파일 및 하위폴더 목록을 모두 얻어온다
		handle = _wfindfirst(wstring(currentWorkingFolder + L"\\*").c_str(), &fileInfo);
		if (-1 == handle) {
			// No file in directory 
		}
		else {
			do {
				// 하위 폴더이면 방문 대상 폴더 stack 에 추가한다
				if (_A_SUBDIR == fileInfo.attrib)
				{
					if (wcscmp(fileInfo.name, L".") != 0 && wcscmp(fileInfo.name, L"..") != 0) {

						if (_wfullpath(sourceFileFullPath, fileInfo.name, _MAX_PATH) != NULL) {
							wprintf(L"Add folder to visit : %s\n", sourceFileFullPath);
							folders.push(sourceFileFullPath);
						}
					}
				}
				// 하위 폴더가 아니면 파일 복사를 하고, 파일 목록에 추가한다
				else
				{
					if (_wfullpath(sourceFileFullPath, fileInfo.name, _MAX_PATH) != NULL) {

						if (NULL == wcsstr(fileInfo.name, L".rep")) {
							continue;
						}

						// 신규 파일 이름 : 타임스탬프
						time_t currentTime;
						time(&currentTime);
						struct _timeb timebuffer;
						_ftime64_s(&timebuffer);
						std::wstring newFileName = to_wstring(currentTime) + to_wstring(timebuffer.millitm);

						// 파일의 md5 해시값 계산하기 : https://msdn.microsoft.com/en-us/library/windows/desktop/aa382380(v=vs.85).aspx
						sourceFileMD5HashString = L"";
						getMD5(fileInfo.name, sourceFileMD5HashString);
						
						wprintf(L"\n");
						wprintf(L"File Name : %s\n", sourceFileFullPath);
						wprintf(L"MD5 Hash : %s\n", sourceFileMD5HashString.c_str());
						
						if (sourceFileMD5HashString.size() > 0) {
							
							// Destination File 이름
							std::wstring destinationFileName = newFileName + L".rep";
							// Destination File 전체경로 + 파일이름
							std::wstring destinationFileFullPath = destinationFolderPath + L"\\" + destinationFileName;

							// 파일을 복사한다.  기존에 동일한 파일이 있다면, 덮어쓰지않고 Fail 로 기록한다							
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
							
							// 파일이 수정된 날짜 및 시각 문자열을 만든다
							struct tm fileDateTime;
							_localtime64_s(&fileDateTime, &fileInfo.time_write);
							wchar_t fileDateString[11];
							wchar_t fileTimeString[9];
							swprintf_s(fileDateString, 11, L"%04d-%02d-%02d\0", (fileDateTime.tm_year + 1900), (fileDateTime.tm_mon + 1), fileDateTime.tm_mday);
							swprintf_s(fileTimeString, 9, L"%02d:%02d:%02d\0", fileDateTime.tm_hour, fileDateTime.tm_min, fileDateTime.tm_sec);

							// 파일 목록.csv 에 append 하고, 바로 flush 한다
							// 항목 : startingSourceFolderPath 명, 원본 폴더 경로, 원본 파일명, 원본 파일 생성일시, destinationFolderPath 명, 신규 파일 이름, 파일 크기, 파일 해시, 복사결과
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

