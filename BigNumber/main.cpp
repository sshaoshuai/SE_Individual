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
		memset(d, 0, sizeof(d));
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

	// check if (A << offset) < B
	friend bool div_smaller(const BigNumber & A, const BigNumber & B, int offset){
		if (A.size + offset != B.size) return A.size + offset < B.size;
		for (int i = A.size; i > 0; i--)
		if (A.d[i] != B.d[i + offset])
			return A.d[i] < B.d[i + offset];
		return true;
	}

	// B - (A << offset)
	friend void div_minus(BigNumber & A, const BigNumber & B, int offset){
		int delta = 0;
		for (int i = 1; i <= A.size - offset; i++){
			delta = MAX_BASE + A.d[i + offset] - B.d[i] + delta;
			A.d[i + offset] = delta % MAX_BASE;
			delta = delta / MAX_BASE - 1;
		}
		while (A.size > 1 && A.d[A.size] == 0) A.size--;
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

	// multiplication operation
	friend BigNumber operator * (const BigNumber & A, const BigNumber & B){
		BigNumber ans;
		ans.size = A.size + B.size;
		for (int i = 1; i <= A.size; i++){
			int delta = 0;
			for (int j = 1; j <= B.size; j++){
				delta = A.d[i] * B.d[j] + delta + ans.d[i + j - 1];
				ans.d[i + j - 1] = delta % MAX_BASE;
				delta /= MAX_BASE;
			}
			ans.d[i + B.size] = delta;
		}
		while (ans.size > 1 && ans.d[ans.size] == 0) ans.size--;
		ans.sign = A.sign * B.sign;
		if (ans.size == 1 && ans.d[1] == 0) ans.sign = 1;
		return ans;
	}

	// multiplication operation with a int
	friend BigNumber operator * (const BigNumber & A, const int & _factor){
		BigNumber ans;
		int delta = 0;
		int sign = 1, factor = _factor;
		if (factor < 0){
			sign = -1; factor = abs(factor);
		}
		ans.size = A.size;
		for (int i = 1; i <= A.size; i++){
			delta = A.d[i] * factor + delta;
			ans.d[i] = delta % MAX_BASE;
			delta /= MAX_BASE;
		}
		while (delta > 0){
			ans.d[++ans.size] = delta % MAX_BASE;
			delta /= MAX_BASE;
		}
		while(ans.size > 1 && ans.d[ans.size] == 0) ans.size--;
		ans.sign = A.sign * sign;
		if (ans.size == 1 && ans.d[1] == 0) ans.sign = 1;
		return ans;
	}

	// division operation
	friend BigNumber operator / (const BigNumber & A, const BigNumber & B){
		BigNumber ans;
		Rem = A;
		Mid[0] = B;
		for (int i = 1; i <= 13; i++) Mid[i] = Mid[i - 1] * 2;
		for (int offset = A.size - B.size; offset >= 0; offset--){
			int delta = 1 << 13;
			for (int j = 13; j >= 0; j--){
				if (div_smaller(Mid[j], Rem, offset)){
					div_minus(Rem, Mid[j], offset);
					ans.d[offset + 1] += delta;
				}
				delta >>= 1;
			}
		}
		ans.size = max(1, A.size - B.size + 1);
		while (ans.size > 1 && ans.d[ans.size] == 0) ans.size--;
		ans.sign = A.sign * B.sign;
		Rem.sign = A.sign;
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


}Mid[14], Rem;




int main()
{
	BigNumber A("-135022155555");
	BigNumber B("2");
	BigNumber C("465412325427809583696563608797995");
	BigNumber D("10561");

	cout << (D / B).toString() << endl;
	cout << Rem.toString() << endl;

	cout << A.toString() << endl;
	cout << B.toString() << endl;
	cout << C.toString() << endl;
	D = A / C;
	cout << D.toString() << endl;
	cout << Rem.toString() << endl;
	cout << (D * A + Rem).toString() << endl;
	getchar();
	return 0;
}