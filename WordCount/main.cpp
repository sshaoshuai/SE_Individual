#include <iostream>
#include <cstdio>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <Windows.h>
#include <io.h>
#include <time.h>

using namespace std;
const int MAX_THREAD = 64;

typedef pair<string, int> PSI;
map<string, int> M;
map<string, int> M_thread[MAX_THREAD];
int topN;
mutex m_lock, m_print_lock;
queue<string> task_list;


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

class ThreadPool{
	vector<thread*> thread_list;
	bool thread_exit;
	bool thread_busy[MAX_THREAD];

	void wordCount(string filePath, int id){
		//m_print_lock.lock();
		//printf("Inner: Thread %d: %s\n", id, filePath.c_str());
		//m_print_lock.unlock();

		FILE *fp = fopen(filePath.c_str(), "r");
		char srcWord[1000];
		int len;
		string curWord;

		while (!feof(fp)){
			fscanf(fp, " %s", srcWord);
			len = strlen(srcWord);
			while (len > 0 && !isLegal(srcWord[len - 1])) len--;
			if (len == 0) continue;
			srcWord[len] = '\0';
			curWord = string(srcWord);

			if (M_thread[id].find(curWord) == M_thread[id].end())
				M_thread[id][curWord] = 1;
			else
				M_thread[id][curWord]++;
		}
		fclose(fp);
	}

	void wordCount_for_thread(int id){
		m_print_lock.lock();
		//printf("Thread %d\n", id);
		m_print_lock.unlock();
		string nowFile;
		while (!thread_exit){
			thread_busy[id] = false;
			m_lock.lock();
			if (task_list.empty()){
				m_lock.unlock();
				continue;
			}
			thread_busy[id] = true;
			nowFile = task_list.front();
			task_list.pop();
			m_lock.unlock();
			m_print_lock.lock();
			printf("Start: Thread %d: %s\n", id, nowFile.c_str());
			m_print_lock.unlock();
			wordCount(nowFile, id);

			//m_print_lock.lock();
			//printf("Complete: Thread %d: %s\n", id, nowFile.c_str());
			//m_print_lock.unlock();
		}
		/*m_print_lock.lock();
		printf("Thread %d exit\n", id);
		m_print_lock.unlock();*/
		return;
	}

	void dfs_merge(int l, int r){
		if (l == r) return;
		int mid = (l + r) >> 1;

		if (r - l > 1){
			thread l_thread(&ThreadPool::dfs_merge, this, l, mid);
			thread r_thread(&ThreadPool::dfs_merge, this, mid + 1, r);
			l_thread.join(); r_thread.join();
		}
		mid++;
		map<string, int>::iterator it;
		for (it = M_thread[mid].begin(); it != M_thread[mid].end(); it++){
			if (M_thread[l].find(it->first) == M_thread[l].end())
				M_thread[l][it->first] = it->second;
			else
				M_thread[l][it->first] += it->second;
		}
		m_print_lock.lock();
		printf("Merge (%d, %d)\n", l, r);
		m_print_lock.unlock();
	}

public:
	ThreadPool(int threadNum){
		thread_exit = false;
		for (int i = 0; i < threadNum; i++) M_thread[i].clear();
		for (int i = 0; i < threadNum; i++)
			thread_list.push_back(new thread(&ThreadPool::wordCount_for_thread, this, i));
	}

	void mul_merge(){
		setTime("Mul_Merge");
		dfs_merge(0, thread_list.size() - 1);
		getTime("Mul_Merge");
	}

	void merge(){
		setTime("Single Merge");
		M.clear();
		map<string, int>::iterator it;
		for (int id = 0; id < thread_list.size(); id++){
			for (it = M_thread[id].begin(); it != M_thread[id].end(); it++){
				if (M.find(it->first) == M.end())
					M[it->first] = it->second;
				else
					M[it->first] += it->second;
			}
		}
		getTime("Single Merge");
	}

	bool threadsFree(){
		for (int i = 0; i < thread_list.size(); i++)
			if (thread_busy[i] == true) return false;
		return true;
	}

	void join(){
		for (int i = 0; i < thread_list.size(); i++)
			thread_list[i]->join();
	}
	void exit(){
		thread_exit = true;
	}

};

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


void find_topN_by_sort(map<string, int> & M, int topN){
	setTime("find_topN_by_sort");

	vector< PSI > A(M.begin(), M.end());

	sort(A.begin(), A.end(), compareByValue);

	for (int i = 0; i < topN; i++){
		printf("%s %d\n", A[i].first.c_str(), A[i].second);
	}
	
	getTime("find_topN_by_sort");
}

void find_topN_by_heap(map<string, int> & M, int topN){
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
		m_lock.lock();
		task_list.push(folderPath + "\\" + fileInfo.name);
		m_lock.unlock();
		//wordCount(folderPath + "\\" + fileInfo.name);
	} while (_findnext(fileHandle, &fileInfo) == 0);
	_findclose(fileHandle);
}

int main(void)
{
	scanf("%d", &topN);
	M.clear();


	//find_topN_by_sort(topN);
	//dfsFolder("novel\\AB");
	//find_topN_by_heap(topN);
	
	dfsFolder("novel");

	setTime("Word Count");
	ThreadPool thread_pool(MAX_THREAD);
	
	while (true){
		m_lock.lock();
		if (task_list.size() == 0 && thread_pool.threadsFree()){
			thread_pool.exit();
			m_lock.unlock();
			break;
		}
		m_lock.unlock();
	}
	getTime("Word Count");

	thread_pool.merge();
	thread_pool.mul_merge();
	find_topN_by_heap(M_thread[0], topN);

	cout << endl << "Program Completed!" << endl;
	
	
	getchar();
	getchar();
	return 0;
}