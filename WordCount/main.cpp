#include <iostream>
#include <cstdio>
#include <string>
#include <algorithm>
#include <io.h>

using namespace std;

void wordCount(string filePath){
	cout << filePath << endl;
}

void dfsFolder(string folderPath){
	_finddata_t fileInfo;
	string formatPath = folderPath + "\\*";
	long fileHandle = _findfirst(formatPath.c_str(), &fileInfo);

	if (fileHandle == -1L){
		cout << folderPath << ": Not Found File" << endl;
		return;
	}
	do{
		if (fileInfo.attrib == _A_SUBDIR){
			if (strcmp(fileInfo.name, ".") == 0 || strcmp(fileInfo.name, "..") == 0) continue;
			dfsFolder(folderPath + "\\" + fileInfo.name);
			continue;
		}
		wordCount(folderPath + "\\" + fileInfo.name);
	} while (_findnext(fileHandle, &fileInfo) == 0);
	_findclose(fileHandle);
}

int main(void)
{
	dfsFolder("novel");


	cout << endl << "Program Completed!" << endl;
	getchar();
	return 0;
}