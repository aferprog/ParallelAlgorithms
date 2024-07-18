// ParallelAlgorithms.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define example 1

#if example==1
#include <iostream>
#include "MultiThreadEngine.h"

//Example of task
void test() {
    for (int i = 0; i < 5; ++i) {
        std::cout << i << ' ';
    }
}

int main() {
    // Creation of engine
    MultiThreadEngine mte(2);

    // Adding the task 3 times
    mte.addTask(test);
    mte.addTask(test);
    mte.addTask(test);

    // waiting until all tasks are finished
    mte.wait();
    std::cout << "\nfinish 1" << std::endl;

    // Adding the task 4 more times
    mte.addTask(test);
    mte.addTask(test);
    mte.addTask(test);
    mte.addTask(test);

    // Premature thread closing
    mte.stop();
    std::cout << "\nfinish 2" << std::endl;
}

#endif // example==1

#if example == 2

#include <iostream>
#include <chrono>
#include <fstream>
#include "ParallelAlgorithms.h"

template<typename Func>
double mesure_func(Func func) {
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
    pal::test_function(std::ref(gen), test, check, 50, 2);
}

#endif