#include <iostream>
#include <cstdio>
#include <string>
#include <algorithm>
using namespace std;

const int MAXL = 1000;
const int SCALE = 4;
const int MAX_BASE = 10000;
const int BASE[SCALE] = { 1, 10, 100, 1000 };

class BigNumber{
private:
	int size;
	int d[MAXL];	// d[1]: lowest digit

public:
	BigNumber(){
		size = 0;
	}
	// initialize the class from a number in string
	BigNumber(string s){
		int len = s.length();
		memset(d, 0, sizeof(d));
		size = (len - 1) / SCALE + 1;
		for (int i = len - 1; i >= 0; i--){
			int pos = (len - i - 1) / SCALE + 1;
			int k = (len - i - 1) % SCALE;
			d[pos] += BASE[k] * (s[i] - '0');
		}
		while (size > 1 && d[size] == 0) size--;
	}

	// return the BigNumber by string
	string toString(){
		string ans = "";
		int curNum = 0;
		if (size == 1 && d[1] == 0) return "0";
		for (int i = SCALE - 1; i >= 0; i--){
			if (d[size] >= BASE[i]){
				curNum = d[size];
				while (i >= 0){
					ans = ans + char(curNum / BASE[i] + '0');
					curNum %= BASE[i--];
				}
				break;
			}
		}
		for (int i = size - 1; i > 0; i--){
			curNum = d[i];
			for (int k = SCALE - 1; k >= 0; k--){
				ans = ans + char(curNum / BASE[k] + '0');
				curNum %= BASE[k];
			}
		}
		return ans;
	}

	// BigNumber Add Operation
	friend BigNumber operator + (const BigNumber & A, const BigNumber & B){
		BigNumber ans;
		int delta = 0;
		ans.size = max(A.size, B.size);
		for (int i = 1; i <= ans.size; i++){
			delta = A.d[i] + B.d[i] + delta;
			ans.d[i] = delta % MAX_BASE;
			delta /= MAX_BASE;
		}
		while (delta){
			ans.d[++ans.size] = delta % MAX_BASE;
			delta /= MAX_BASE;
		}
		return ans;
	}
};




int main()
{
	BigNumber A("13543541646588824354350");
	BigNumber B("465412325441353125343152433152345");

	cout << (A + B).toString() << endl;
	getchar();
	return 0;
}