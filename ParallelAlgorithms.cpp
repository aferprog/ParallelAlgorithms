// ParallelAlgorithms.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>

template<typename Func>
double mesure_func(Func func){
    puts("Now");
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    auto t = end - start;
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time taken to fill array: " << duration.count() << " seconds" << std::endl;
    std::cout << std::endl;
    return duration.count();
}

#define example 1

#if example==1
#include "MultiThreadEngine.h"
#include "Devider.h"

void test() {
    for (int i = 0; i < 5; ++i) {
        std::cout << i << '\n';
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    MultiThreadEngine mte(2);

    mte.addTask(test);
    mte.addTask(test);
    mte.addTask(test);

    mte.wait();
    std::cout << "next pool" << std::endl;

    mte.addTask(test);
    mte.addTask(test);
    mte.addTask(test);
    mte.addTask(test);
    mte.stop();
    std::cout << "finish" << std::endl;
}

#endif // example==1

#if example == 2
#include <fstream>
#include "ParallelAlgorithms.h"

struct Generator{
    int seed = 0;
    size_t len = 10000000;
    std::vector<int> operator()() {
        srand(seed++);
        std::vector<int> arr(len);
        for (int i = 0; i < arr.size(); i++)
            arr[i] = rand();
        return arr;
    }
};
struct Checker {
    int id = 0;
    void operator()(std::vector<int>& data, std::vector<int>& res) {
        id++;
        for (size_t i = 1; i < res.size(); ++i)
            if (res[i - 1] > res[i])
            {
                std::cout << "failed\n";
                std::ofstream fout(std::string("tests/") + std::to_string(id) + ".txt");
                for (auto t : data)
                    fout << t << '\n';
                fout.close();
                return;
            }
        std::cout << "passed\n";
    }
};

int main()
{    
    Generator gen;
    auto test = [](std::vector<int> arr) {
        pal::sort_by_parts(arr.begin(), arr.end(), 4);
        return arr;
    };
    Checker check;
    pal::test_function(gen, test, check, 50, 2);
    
}

#endif