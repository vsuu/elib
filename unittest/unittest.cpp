// unittest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Singleton.h"
#include "OnScopeExit.h"
#include "strop.h"
#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>
#include <algorithm>
#include <array>
#include <deque>
#include <set>
#include <thread>
#include <future>
#include <iterator>
#include <condition_variable>
#include "threadpool.h"
#include "TimeCost.h"
#include "ObjectPool.h"
#include "range.h"
#include "ClassAttr.h"
#include <cstring>
#include <cstdarg>
#include "Log.h"
#include <cstdio>
#include <numeric>
#include "BerTLV.h"
#include <set>
#include <functional>
#include "Log.h"
#include <queue>
#include <map>
using namespace std;

std::condition_variable cv;
std::mutex m;
int x = 0;

void fun1(const char *s)
{
	unique_lock<mutex> locker(m);
	cv.wait(locker);
	for (int i = 0; i < 100; ++i)
		puts(s);
}

thread *ptr = nullptr;
void destroy()
{
	ptr->join();
	delete ptr;
}

void out(const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	//    cout << vsnprintf(nullptr, 0, format, arg);
	va_end(arg);
}

using namespace elib;
//int _tmain(int argc, _TCHAR* argv[])
//{
//    try{
//        Logger x;
//        x.AddLogFile(unique_ptr<LogFileBase>(new LogFile("b:\\", "test", LogType::INFO)));
//
//        string log(300, 'r');
//        COUNT_TIME_COST;
//        for (int i = 0; i < 1000000; ++i)
//        {
//            LOG_INFO(x) << log;
//        }
//    }
//    catch (const exception &e)
//    {
//        cout << e.what() << endl;
//    }
//
//    PRINT_TIME_COST(cout);
//
//    system("pause");
//
//    return 0;
//}

template<typename Container>
void output(const Container &arr)
{
	for (auto x : arr)
		cout << x << ',';
	cout << endl;
}

template<typename T>
void Qsort(T *arr, int len)
{
	if (len < 2)return;
	int i = 1;
	int j = len - 1;
	while (i <= j)
	{
		while ((i <= j) && (arr[i] <= arr[0]))++i;

		while ((j >= i) && (arr[0] < arr[j]))--j;

		if (i > j)
		{
			break;
		}

		swap(arr[i], arr[j]);
		++i;
		--j;
	}

	swap(arr[0], arr[i - 1]);

	Qsort(arr, i - 1);
	Qsort(arr + i, len - i);
}

template<typename T>
void _MergeSort(T *arr, size_t len, T* tmp)
{
	assert(nullptr != arr);

	if (len == 1)
	{
		return;
	}
	if (len == 2)
	{
		if (arr[0] > arr[1])swap(arr[0], arr[1]);
		return;
	}

	size_t len1 = len / 2;
	size_t len2 = len - len1;
	T * ptr = tmp + len1;
	_MergeSort(arr, len1, tmp);
	_MergeSort(arr + len1, len2, ptr);
	size_t i = 0, j = 0, k = 0;
	ptr = arr + len1;
	while (i < len)
	{
		if (j >= len1)
		{
			tmp[i++] = ptr[k++];
		}
		else if (k >= len2)
		{
			tmp[i++] = arr[j++];
		}
		else if (arr[j] < ptr[k])
		{
			tmp[i++] = arr[j++];
		}
		else
		{
			tmp[i++] = ptr[k++];
		}
	}
	for (size_t m = 0; m < len; ++m)
	{
		arr[m] = tmp[m];
	}
}
template<typename T>
void MergeSort(T *arr, size_t len)
{
	std::vector<T> tmp(len);
	_MergeSort(arr, len, tmp.data());
}

void CfgTest();
void TestBerTLV();
void ApduTest();
void PCSC_Test();

bool bertlv_compare(const BerTLV &a, const BerTLV &b)
{
	return a.Tag() < b.Tag();
}

struct defaulttest
{
	int x;
	int y;
};

void BubbleSort(vector<int> &data)
{
	auto len = data.size() - 1;
	size_t pos = 0;
	size_t pos1 = 0;
	while (len > 0)
	{
		pos = 0;
		pos1 = 0;
		while (pos < len)
		{
			if (data[pos]>data[pos + 1])
			{
				swap(data[pos], data[pos + 1]);
				pos1 = pos;
			}
			++pos;
		}
		len = pos == len ? len - 1 : pos1 - 1;
	}
}

int i = 5;
class test2
{
public:
	int i = 6;
};
class test3
{
public:
	int i = 7;
	class  test1 :public test2
	{
	public:
		void fun()
		{
			cout << i << endl;
		}
	private:
		//int i = 4;
	};
};

int main(int argc, char* argv[])
{
	/*PCSC_Test();
	CfgTest();
	TestBerTLV();
	ApduTest();
	*/
	test3::test1 y;
	y.fun();
	system("pause");
	return 0;
}