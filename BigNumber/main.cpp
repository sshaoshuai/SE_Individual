#include <iostream>
#include <cstdio>
#include <string>
using namespace std;

const int MAXL = 1000;
const int SCALE = 4;
const int BASE[SCALE] = { 1, 10, 100, 1000 };

class BigNumber{
private:
	int size;
	int d[MAXL];	// d[1]: lowest digit

public:
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
};




int main()
{
	BigNumber A("0012003400012567890000000003452345000");
	string ans = A.toString();
	cout << ans << endl;

	getchar();
	return 0;
}