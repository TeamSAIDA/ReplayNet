#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <io.h>
#include <string>
#include <direct.h>
#include <queue>
#include <stack>

using namespace std;

int main(int argc, char** argv) {

	// 파일들을 복사해넣을 목적 폴더
	std::string destinationFolder = "D:\\StarCraftReplays\\REP";

	//for (int i = 0; i < argc; i++) {
	//	printf("%s\n", argv[i]);
	//}

	if (argc >= 2) {
		// TODO : argv[1] 이 정상 폴더인지 아닌지 체크한다
		destinationFolder = argv[1];
	}

	// 방문대상 폴더 stack -> 현재위치로부터 시작해서 Depth First Search 방식으로 방문한다
	std::stack<std::string> folders;

	// 폴더 경로
	std::string startingFolder;
	std::string nextFolder;

	// 파일 찾기 핸들
	long handle;
	
	// 파일 정보 구조체
	struct _finddata_t fileInfo;   


	// 260자 넘는 패스 다루기
	char* _startingFolder = _getcwd(NULL, 0);
	startingFolder = _startingFolder;
	free(_startingFolder);

	// 방문 대상 폴더 stack 에 현재 폴더를 추가한다
	folders.push(startingFolder);
	printf("startingFolder : %s\n", startingFolder);

	char fullpath[_MAX_PATH];

	// stack 의 제일 위에 있는 폴더 경로로 이동한다
	while (folders.empty() == false) {
		std::string nextFolder = folders.top();
		folders.pop();

		_chdir(nextFolder.c_str());
		printf("current visiting folder %s\n", nextFolder);

		// 현재 폴더의 파일 및 하위폴더 목록을 모두 얻어온다
		handle = _findfirst(string(nextFolder + "\\*.*").c_str(), &fileInfo);
		if (-1 == handle) {
			// No file in directory 
		}
		else {
			do {
				// 하위 폴더이면 방문 대상 폴더 stack 에 추가한다
				if (_A_SUBDIR == fileInfo.attrib)
				{
					if (strcmp(fileInfo.name, ".") != 0 && strcmp(fileInfo.name, "..") != 0) {

						if (_fullpath(fullpath, fileInfo.name, _MAX_PATH) != NULL) {
							printf("Add folder to visit : %s\n", fullpath);
							folders.push(fullpath);
						}
					}
				}
				// 하위 폴더가 아니면 파일 복사를 하고, 파일 목록에 추가한다
				else
				{
					if (_fullpath(fullpath, fileInfo.name, _MAX_PATH) != NULL) {


						// 신규 파일 이름 : 파일의 md5 해시값으로 정의 https://msdn.microsoft.com/en-us/library/windows/desktop/aa382380(v=vs.85).aspx



						// 파일 목록.csv 에 append 한다

						// . 항목 : startingFolder 명, 원본 폴더 경로, 원본 파일명, 원본 파일 생성일시, destinationFolder 명, 신규 파일 이름, 파일 크기
						// . 추가방법 : cout 으로 바로바로. flush


						printf("Copy file %s to %s \n", fullpath, destinationFolder);
					}
				}
			} while (_findnext(handle, &fileInfo) == 0);
		}
	}

	_findclose(handle);  

	return 0;
}



