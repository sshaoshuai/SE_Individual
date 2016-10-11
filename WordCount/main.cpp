#include <iostream>
#include <cstdio>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <queue>
#include <io.h>
#include <time.h>

using namespace std;

typedef pair<string, int> PSI;
map<string, int> M;
int topN;

clock_t		tStart = 0;
void setTime(const char * prompt)
{
	printf("\n-------------%s--------------\n\n", prompt);
	tStart = clock();
}
void getTime(const char * prompt)
{
	double time = (double)(clock() - tStart) / (double)CLOCKS_PER_SEC;
	printf("\n-------------%s %f--------------\n\n", prompt, time);
}


inline bool isLegal(char c){
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'));
}

void wordCount(string & filePath){
	cout << filePath << endl;
	FILE *fp = fopen(filePath.c_str(), "r");
	char srcWord[100];
	int len;
	string curWord;

	while (!feof(fp)){
		fscanf(fp, " %s", srcWord);
		len = strlen(srcWord);
		while (len > 0 && !isLegal(srcWord[len - 1])) len--;
		if (len == 0) continue;
		srcWord[len] = '\0';
		curWord = string(srcWord);
		if (M.find(curWord) == M.end()) 
			M[curWord] = 1;
		else
			M[curWord]++;
	}
	fclose(fp);
}

bool compareByValue(const PSI & A, const PSI & B){
	return A.second > B.second;
}
struct cmp{
	bool operator ()(const PSI & A, const PSI & B){
		if (A.second == B.second) return A.first < B.first;
		return A.second > B.second;
	}
};


void find_topN_by_sort(int topN){
	setTime("find_topN_by_sort");

	vector< PSI > A(M.begin(), M.end());

	sort(A.begin(), A.end(), compareByValue);

	for (int i = 0; i < topN; i++){
		printf("%s %d\n", A[i].first.c_str(), A[i].second);
	}
	
	getTime("find_topN_by_sort");
}

void find_topN_by_heap(int topN){
	setTime("find_topN_by_heap");
	priority_queue<PSI, vector<PSI>, cmp> Q;
	while (!Q.empty()) Q.pop();

	map<string, int>::iterator it;
	int cnt = 0;
	for (it = M.begin(), cnt = 0; it != M.end() && cnt < topN; it++, cnt++) 
		Q.push(*it);
	for (; it != M.end(); it++){
		if (it->second < Q.top().second) continue;
		Q.push(*it); Q.pop();
	}

	vector<PSI> output;
	output.clear();
	while (!Q.empty()){
		output.push_back(Q.top());
		Q.pop();
	}
	for (int i = output.size() - 1; i >= 0; i--)
		printf("%s %d\n", output[i].first.c_str(), output[i].second);
	
	getTime("find_topN_by_heap");
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
	scanf("%d", &topN);
	M.clear();
	dfsFolder("novel\\AB");

	find_topN_by_sort(topN);
	find_topN_by_heap(topN);

	cout << endl << "Program Completed!" << endl;
	getchar();
	getchar();

	return 0;
}