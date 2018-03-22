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

	// ���ϵ��� �����س��� ���� ����
	std::string destinationFolder = "D:\\StarCraftReplays\\REP";

	//for (int i = 0; i < argc; i++) {
	//	printf("%s\n", argv[i]);
	//}

	if (argc >= 2) {
		// TODO : argv[1] �� ���� �������� �ƴ��� üũ�Ѵ�
		destinationFolder = argv[1];
	}

	// �湮��� ���� stack -> ������ġ�κ��� �����ؼ� Depth First Search ������� �湮�Ѵ�
	std::stack<std::string> folders;

	// ���� ���
	std::string startingFolder;
	std::string nextFolder;

	// ���� ã�� �ڵ�
	long handle;
	
	// ���� ���� ����ü
	struct _finddata_t fileInfo;   


	// 260�� �Ѵ� �н� �ٷ��
	char* _startingFolder = _getcwd(NULL, 0);
	startingFolder = _startingFolder;
	free(_startingFolder);

	// �湮 ��� ���� stack �� ���� ������ �߰��Ѵ�
	folders.push(startingFolder);
	printf("startingFolder : %s\n", startingFolder);

	char fullpath[_MAX_PATH];

	// stack �� ���� ���� �ִ� ���� ��η� �̵��Ѵ�
	while (folders.empty() == false) {
		std::string nextFolder = folders.top();
		folders.pop();

		_chdir(nextFolder.c_str());
		printf("current visiting folder %s\n", nextFolder);

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

						if (_fullpath(fullpath, fileInfo.name, _MAX_PATH) != NULL) {
							printf("Add folder to visit : %s\n", fullpath);
							folders.push(fullpath);
						}
					}
				}
				// ���� ������ �ƴϸ� ���� ���縦 �ϰ�, ���� ��Ͽ� �߰��Ѵ�
				else
				{
					if (_fullpath(fullpath, fileInfo.name, _MAX_PATH) != NULL) {


						// �ű� ���� �̸� : ������ md5 �ؽð����� ���� https://msdn.microsoft.com/en-us/library/windows/desktop/aa382380(v=vs.85).aspx



						// ���� ���.csv �� append �Ѵ�

						// . �׸� : startingFolder ��, ���� ���� ���, ���� ���ϸ�, ���� ���� �����Ͻ�, destinationFolder ��, �ű� ���� �̸�, ���� ũ��
						// . �߰���� : cout ���� �ٷιٷ�. flush


						printf("Copy file %s to %s \n", fullpath, destinationFolder);
					}
				}
			} while (_findnext(handle, &fileInfo) == 0);
		}
	}

	_findclose(handle);  

	return 0;
}



