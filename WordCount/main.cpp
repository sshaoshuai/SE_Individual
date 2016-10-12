#include <iostream>
#include <cstdio>
#include <string>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <Windows.h>
#include <io.h>
#include <time.h>
using namespace std;
const int MAX_THREAD = 4;
const int MAX_BRANCH = 26;

clock_t		tStart = 0;
void setTime(const char * prompt)
{
	printf("---------------------------------\nStart: %s\n\n", prompt);
	tStart = clock();
}
void getTime(const char * prompt)
{
	double time = (double)(clock() - tStart) / (double)CLOCKS_PER_SEC;
	printf("\nComplete: %s %f second\n--------------------------------\n", prompt, time);
}

inline bool isLegal(char c){
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}


int topN;
mutex m_lock, m_print_lock;
queue<string> task_list;

struct WordNode{
	char now[1000];
	bool operator== (const WordNode & A) const
	{
		return strcmp(now, A.now) == 0;
	}
};
struct ptrCmp{
	bool operator()(const WordNode A, const WordNode B) const
	{
		return strcmp(A.now, B.now) < 0;
	}
};
struct MyHash{
	size_t operator() (const WordNode & A) const
	{
		return hash_value(A.now);
	}
};


typedef pair<WordNode, int> PSI;
typedef pair<char*, int> PCI;
typedef unordered_map<WordNode, int, MyHash> Map_PtrChar_Int;
Map_PtrChar_Int M;
Map_PtrChar_Int M_thread[MAX_THREAD];


bool compareByValue(const PSI & A, const PSI & B){
	return A.second > B.second;
}
struct cmp{
	bool operator ()(const PSI & A, const PSI & B){
		if (A.second == B.second) return strcmp(A.first.now, B.first.now) < 0;;
		return A.second > B.second;
	}
};

struct cmpPCI{
	bool operator ()(const PCI & A, const PCI & B){
		if (A.second == B.second) return strcmp(A.first, B.first) < 0;;
		return A.second > B.second;
	}
};



struct TrieNode{
	char *word;
	int cnt;
	TrieNode *next_branch[MAX_BRANCH];

public:
	TrieNode(){
		cnt = 0; word = NULL;
		memset(next_branch, NULL, sizeof(TrieNode*)* MAX_BRANCH);
	}
};
TrieNode *TrieNodeBuf = new TrieNode[2000000];
char * wordBuf = new char[10000000];
int buf_offset = 0, word_buf_offset = 0;
mutex buf_lock;


class Trie{
	TrieNode * root;
	priority_queue<PCI, vector<PCI>, cmpPCI> Q;
public:
	Trie(){
		root = new TrieNode();
	}
	void insert(const char * src, int delta){
		if (src == NULL) return;
		TrieNode *curNode = root;
		int index = 0;
		for (int i = 0; src[i]; i++){
			index = src[i] - 'a';
			if (index < 0 || index >= MAX_BRANCH) return;
			if (curNode->next_branch[index] == NULL){
				buf_lock.lock();
				curNode->next_branch[index] = new (TrieNodeBuf + (buf_offset++)) TrieNode();
				buf_lock.unlock();
			}
			curNode = curNode->next_branch[index];
		}
		if (curNode->word == NULL){
			int len = strlen(src);
			curNode->word = new (wordBuf + word_buf_offset) char[len + 3];
			word_buf_offset += len + 3;
			strcpy(curNode->word, src);
		}
		curNode->cnt += delta;
	}

	void dfs_merge(const TrieNode * cur){
		if (cur == NULL) return;
		if (cur->word != NULL){
			insert(cur->word, cur->cnt);
		}
		for (int i = 0; i < MAX_BRANCH; i++)
			dfs_merge(cur->next_branch[i]);
	}

	void merge(const Trie * A){
		dfs_merge(A->root);
	}

	void dfs_find_topN(const TrieNode * cur, int topN){
		if (cur == NULL) return;
		if (cur->word != NULL){
			if (Q.size() < topN){
				Q.push(make_pair(cur->word, cur->cnt));
			}
			else{
				if (cur->cnt > Q.top().second){
					Q.pop();
					Q.push(make_pair(cur->word, cur->cnt));
				}
			}
		}
		for (int i = 0; i < MAX_BRANCH; i++)
			dfs_find_topN(cur->next_branch[i], topN);
	}

	void find_topN(int topN){
		setTime("find_topN_by_heap");

		while (!Q.empty()) Q.pop();

		dfs_find_topN(root, topN);


		vector<PCI> output;
		output.clear();
		while (!Q.empty()){
			output.push_back(Q.top());
			Q.pop();
		}
		for (int i = output.size() - 1; i >= 0; i--)
			printf("%s %d\n", output[i].first, output[i].second);

		getTime("find_topN_by_heap");

	}


} *word_count[MAX_THREAD];


class ThreadPool{
	vector<thread*> thread_list;
	bool thread_exit;
	bool thread_busy[MAX_THREAD];

	void wordCount(string filePath, int id){
		FILE *fp = fopen(filePath.c_str(), "rb");
		if (fp == NULL){
			m_print_lock.lock();
			printf("Thread %d: Not Found File => %s\n", id, filePath.c_str());
			m_print_lock.unlock();
			return;
		}
		int file_len = 0;
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		rewind(fp);

		char *srcWord = new char[file_len + 6];
		char temp[1000];
		int word_st = 0, word_len = 0;
		fread(srcWord, 1, file_len, fp);

		for (int i = 0; i < file_len; i++){
			//if (isLegal(srcWord[i])){
			//	if (srcWord[i] >= 'A' && srcWord[i] <= 'Z')
			//		temp[word_len++] = srcWord[i] - 'A' + 'a';
			//	else
			//		temp[word_len++] = srcWord[i];
			//	continue;
			//}
			if (srcWord[i] >= 'A' && srcWord[i] <= 'Z'){
				temp[word_len++] = srcWord[i] - 'A' + 'a';
				continue;
			}
			else if (srcWord[i] >= 'a' && srcWord[i] <= 'z'){
				temp[word_len++] = srcWord[i];
				continue;
			}
			if (word_len == 0){
				word_st = i + 1;
				continue;
			}
			temp[word_len] = '\0';
			word_count[id]->insert(temp, 1);
			word_st = i + 1; word_len = 0;
		}
		delete [] srcWord;
		fclose(fp);
	}

	void wordCount_for_thread(int id){
		string nowFile;
		int cnt_run_free = 0;
		while (!thread_exit){
			thread_busy[id] = false;
			/*m_print_lock.lock();
			printf("Run Empty: Thread %d\n", id);
			m_print_lock.unlock();*/
			cnt_run_free++;
			if (cnt_run_free > 10) return;


			m_lock.lock();
			if (task_list.empty()){
				m_lock.unlock();
				continue;
			}
			cnt_run_free = 0;
			thread_busy[id] = true;
			nowFile = task_list.front();
			task_list.pop();
			m_lock.unlock();
	/*		m_print_lock.lock();
			printf("Start: Thread %d: %s\n", id, nowFile.c_str());
			m_print_lock.unlock();*/
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
		Map_PtrChar_Int::iterator it;
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

	void dfs_merge_trie(int l, int r){
		if (l == r) return;
		int mid = (l + r) >> 1;

		if (r - l > 1){
			thread l_thread(&ThreadPool::dfs_merge_trie, this, l, mid);
			thread r_thread(&ThreadPool::dfs_merge_trie, this, mid + 1, r);
			l_thread.join(); r_thread.join();
		}
		mid++;
		
		
		Trie *left = word_count[l];
		Trie *right = word_count[mid];

		left->merge(right);

		m_print_lock.lock();
		//printf("Merge (%d, %d)\n", l, r);
		m_print_lock.unlock();
	}



public:
	ThreadPool(int threadNum){
		thread_exit = false;
		for (int i = 0; i < threadNum; i++) M_thread[i].clear();
		for (int i = 0; i < threadNum; i++) word_count[i] = new Trie();
		for (int i = 0; i < threadNum; i++)
			thread_list.push_back(new thread(&ThreadPool::wordCount_for_thread, this, i));
	}

	void mul_merge(){
		setTime("Mul_Merge");
		cout << buf_offset << endl << word_buf_offset << endl;
		dfs_merge_trie(0, thread_list.size() - 1);
		getTime("Mul_Merge");
	}

	void merge(){
		setTime("Single Merge");
		M.clear();
		Map_PtrChar_Int::iterator it;
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
		if (thread_busy[i] == true) {
			//printf("Run %d\n", i);
			return false;
		}
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

//void wordCount(string & filePath){
//	cout << filePath << endl;
//	FILE *fp = fopen(filePath.c_str(), "r");
//	char srcWord[100];
//	int len;
//	string curWord;
//
//	while (!feof(fp)){
//		fscanf(fp, " %s", srcWord);
//		len = strlen(srcWord);
//		while (len > 0 && !isLegal(srcWord[len - 1])) len--;
//		if (len == 0) continue;
//		srcWord[len] = '\0';
//		curWord = string(srcWord);
//		if (M.find(curWord) == M.end()) 
//			M[curWord] = 1;
//		else
//			M[curWord]++;
//	}
//	fclose(fp);
//}


void find_topN_by_sort(Map_PtrChar_Int & M, int topN){
	setTime("find_topN_by_sort");

	vector< PSI > A(M.begin(), M.end());

	sort(A.begin(), A.end(), compareByValue);

	for (int i = 0; i < topN; i++){
		printf("%s %d\n", A[i].first.now, A[i].second);
	}
	
	getTime("find_topN_by_sort");
}

void find_topN_by_heap(Map_PtrChar_Int & M, int topN){
	setTime("find_topN_by_heap");
	priority_queue<PSI, vector<PSI>, cmp> Q;
	while (!Q.empty()) Q.pop();

	Map_PtrChar_Int::iterator it;
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
		printf("%s %d\n", output[i].first.now, output[i].second);
	
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
		//m_lock.lock();
		if (task_list.size() == 0){
			while (!thread_pool.threadsFree());
			thread_pool.exit();
			//m_lock.unlock();
			break;
		}
		//m_lock.unlock();
	}
	getTime("Word Count");

	//thread_pool.merge();
	thread_pool.mul_merge();
	//find_topN_by_heap(M_thread[0], topN);


	word_count[0]->find_topN(topN);



	cout << endl << "Program Completed!" << endl;
	
	
	getchar();
	getchar();
	return 0;
}