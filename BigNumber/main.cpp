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
	int sign;		// -1: negative, 1: not negative
	int d[MAXL];	// d[1]: lowest digit

public:
	BigNumber(){
		size = 0; sign = 1;
	}
	// initialize the class from a number in string
	BigNumber(string src){
		if (src == "") src = "0";
		int len = src.length();
		memset(d, 0, sizeof(d));
		if (src[0] == '-'){
			sign = -1; size = (len - 1 - 1) / SCALE + 1;
		}
		else{
			sign = 1; size = (len - 1) / SCALE + 1;
		}
		int MSB = (sign == 1) ? 0 : 1;
		for (int i = len - 1; i >= MSB; i--){
			int pos = (len - i - 1) / SCALE + 1;
			int k = (len - i - 1) % SCALE;
			d[pos] += BASE[k] * (src[i] - '0');
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
		if (sign == -1) ans = '-' + ans;
		return ans;
	}
	
	// add operation, don't care the sign
	friend void Add(const BigNumber & A, const BigNumber & B, BigNumber & ans){
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
	}

	// minus operation, don't care the sign
	friend void Minus(const BigNumber & A, const BigNumber & B, BigNumber & ans){
		int delta = 0;
		ans.size = A.size; ans.sign = 1;
		for (int i = 1; i <= ans.size; i++){
			delta = MAX_BASE + A.d[i] - B.d[i] + delta;
			ans.d[i] = delta % MAX_BASE;
			delta = delta / MAX_BASE - 1;
		}
		while (ans.size > 1 && ans.d[ans.size] == 0) ans.size--;
	}

	// abs comparison
	friend bool abs_smaller(const BigNumber & A, const BigNumber & B){
		if (A.size != B.size) return A.size < B.size;
		for (int i = A.size; i > 0; i--)
		if (A.d[i] != B.d[i]) {
			return A.d[i] < B.d[i];
		}
		return false;
	}

	// BigNumber Add Operation
	friend BigNumber operator + (const BigNumber & A, const BigNumber & B){
		BigNumber ans;
		if (A.sign == 1 && B.sign == -1){
			// A - B
			if (abs_smaller(A, B)){
				// |A| < |B|
				Minus(B, A, ans); ans.sign = -1;
			}
			else{
				Minus(A, B, ans); ans.sign = 1;
			}
			return ans;
		}
		if (A.sign == -1 && B.sign == 1){
			// B - A
			if (abs_smaller(B, A)){
				Minus(A, B, ans); ans.sign = -1;
			}
			else{
				Minus(B, A, ans); ans.sign = 1;
			}
			return ans;
		}
		Add(A, B, ans);
		ans.sign = A.sign;
		return ans;
	}

	// BigNumber  Minus Operation
	friend BigNumber operator - (const BigNumber & A, const BigNumber & B){
		BigNumber ans;
		if (A.sign == 1 && B.sign == 1){
			if (abs_smaller(A, B)){
				Minus(B, A, ans); ans.sign = -1;
			}
			else{
				Minus(A, B, ans); ans.sign = 1;
			}
			return ans;
		}
		if (A.sign == -1 && B.sign == -1){
			if (abs_smaller(B, A)){
				Minus(A, B, ans); ans.sign = -1;
			}
			else{
				Minus(B, A, ans); ans.sign = 1;
			}
			return ans;
		}
		Add(A, B, ans);
		ans.sign = A.sign;
		return ans;
	}

	// A < B comparison
	friend bool operator < (const BigNumber & A, const BigNumber & B){
		if (A.sign != B.sign){
			if (A.sign == -1) return true;
			return false;
		}
		if (A.sign == -1) 
			return !abs_smaller(A, B);
		return abs_smaller(A, B);
	}

};




int main()
{
	BigNumber A("-13543541646588824354350");
	BigNumber B("-41353125343152433152345");
	BigNumber C("22441353125343152433152344");
	cout << (A + B).toString() << endl;
	cout << (A - C).toString() << endl;
	getchar();
	return 0;
}