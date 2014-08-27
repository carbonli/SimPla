/**
 * \file multi_thread_test.cpp
 *
 * \date    2014年8月27日  上午7:25:40 
 * \author salmon
 */
#include "multi_thread.h"
//#include "../utilities/sp_iterator.h"
//#include <iostream>
//#include <vector>
#include <thread>
#include <future>
#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
using namespace simpla;

int main(int argc, char **argv)
{

	std::vector<int> d(100);

	for (int i = 0; i < 100; ++i)
	{
		d[i] = 1;
	}

	int total = 1;

	auto range = make_range(d.begin(), d.end());

	typedef decltype(range) range_type;

	parallel_reduce(range, &total,

	[](range_type const &r, int *res )
	{	for(auto const & v:r)
		{	*res+=v;}},

	[](int l,int *r)
	{	*r+=l;}

	);

	std::cout << total << std::endl;
//
////	parallel_do([](int num_threads,int thread_num)
////	{
////		std::cout<< thread_num <<" in " << num_threads<<std::endl;
////	}, 4);
//
////	std::vector<int> data(10);
////	for (auto & v : data)
////	{
////		v = 1;
////	}
////	int total = 0;
////	std::function<void(int const &, int*)> foo = [](int const &a ,int*b)
////	{	*b+=a;};
////
////	parallel_reduce(std::make_pair(data.begin(), data.end()), foo, &total);
////	std::cout << " total = " << total << std::endl;
//	auto f1 = std::async(std::launch::async, [&]()
//	{
//		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//		std::cout<<"This is 1"<<std::endl;
//
//	});
//
//	auto f2 = std::async(std::launch::async, [&]()
//	{
//		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
//		std::cout<<"This is 2"<<std::endl;
//
//	});
//
//	std::this_thread::sleep_for(std::chrono::milliseconds(1600));
//			std::cout<<"This is 0"<<std::endl;
//
//	f1.get();
//	f2.get();

}

