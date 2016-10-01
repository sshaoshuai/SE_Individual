#include <iostream>
#include <cstdio>
#include <algorithm>
#include <time.h>
using namespace std;

const int N = 20000000;

clock_t		tStart = 0;
void setTime(const char * prompt)
{
	printf("%s\n", prompt);
	tStart = clock();
}
void getTime(const char * prompt)
{
	double time = (double)(clock() - tStart) / (double)CLOCKS_PER_SEC;
	printf("%s %f\n", prompt, time);
}


bool isPrime[N + N];
int ans[N + N], cnt = 0;

void make_prime(int Top)  {
	memset(isPrime, true, sizeof(isPrime));
	cnt = 0;
	int limit = sqrt(Top) + 6;
	for (int i = 2; i < limit; ++i){
		if (isPrime[i]){
			ans[++cnt] = i;
			for (int k = i + i; k <= Top; k += i){
				isPrime[k] = false;
			}
		}
	}
	for (int i = limit; i <= Top; ++i){
		if (isPrime[i] == true){
			ans[++cnt] = i;
		}
	}
	//printf("Tot Prime: %d\n", cnt);
	return;
}

void linear_make_prime(int Top)  {
	memset(isPrime, true, sizeof(isPrime));
	cnt = 0;
	for (int i = 2; i <= Top; ++i){
		if (isPrime[i])
			ans[++cnt] = i;
		for (int k = 1; k <= cnt; k++){
			if (i * ans[k] > Top) break;
			isPrime[i * ans[k]] = false;
			if (i % ans[k] == 0) break;
		}
		
	}
	//printf("Tot Prime: %d\n", cnt);
	return;
}


void PrintNum(int x){
	int temp[4], pos = 0;
	while (x > 0){
		temp[pos++] = x % 1000;
		x /= 1000;
	}
	for (int i = pos - 1; i > 0; i--) printf("%d ", temp[i]);
	printf("%d\n", temp[0]);
}

int main()
{
	int n = 1000000;
	int MAX = 16000000;
	setTime("Make Prime: ");
	make_prime(MAX);
	PrintNum(ans[n]);
	getTime("Make End");

	setTime("Make Prime: ");
	linear_make_prime(MAX);
	PrintNum(ans[n]);
	getTime("Make End");
	

	getchar();
	return 0;
}


