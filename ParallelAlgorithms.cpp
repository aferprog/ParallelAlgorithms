// ParallelAlgorithms.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "ParallelAlgorithms.h"
#include "MultyThreadEngine.h"

template<typename Func>
double mesure_func(Func func){
    puts("Now");
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time taken to fill array: " << duration.count() << " seconds" << std::endl;
    std::cout << std::endl;
    return duration.count();
}

void test() {
    int sum;
    for (int i = 0; i < 11000; i++)
        std::cout <<'('<< std::this_thread::get_id() << ") -- " << i << '\n';
}

void testMTE(int taskCount, int threadsCount) {
    MultyThreadEngine mte(threadsCount);
    for (int i=0; i< taskCount; i++)
        mte.addTask(test);
    
}

int main()
{
    mesure_func(std::bind(testMTE, 10, 2));
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    mesure_func(std::bind(testMTE, 10, 5));


    //srand(time(0));
    //size_t len = 1000000;
    //std::vector<int> arr(len), arr1(len);

    //for (int i = 0; i < arr.size(); i++)
    //    arr[i] = arr1[i] = rand();
    //
    ////mesure_func(std::bind(std::sort<std::vector<int>::iterator>, arr.begin(), arr.end()));
    //
    //mesure_func(std::bind(pal::sort_by_parts<std::vector<int>::iterator>, arr1.begin(), arr1.end(), 11));
    //for (size_t i=1; i<len; ++i)
    //    if (arr1[i-1] > arr1[i]){
    //        printf("incorrect in %llu\n", i);
    //        for (size_t j = std::max(i - 50llu, 0llu); j < std::min(len, i + 50llu); ++j)
    //            if (i!=j)
    //                printf("%i\n", arr1[j]);
    //            else
    //                printf("-- %i\n", arr1[j]);
    //        break;
    //    }
}
