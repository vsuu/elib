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
void CfgTest();
void TestBerTLV();
void ApduTest();
int main(int argc, char* argv[])
{
    CfgTest();
    TestBerTLV();
    ApduTest();
    system("pause");
    return 0;
}