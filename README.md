# ParallelAlgorithms
## General idea
The goal of this repository is to present multi-thread solutions of several basic problems, providing some tools to simplify dividing huge problems to parts and do it with minimal resource consumption. 

## Multi-thread engine
The main pearl of a repository is the MultiThreadEngine class. It's a tool which provides the capability to create a pool of threads in advance and use them when needed. Created threads are sleeping and waiting for tasks, so they consume a little resources. It allows you to avoid extra creations and utilisations of threads and save much resources on it.

### Usage
#### Example of usage

Program:
'''cpp
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

    // waiting until all tasks are finished
    mte.wait();
    std::cout << "\nfinish 2" << std::endl;
}
'''


Output:
'''
0 01  21  23  34  40  1 2 3 4
finish 1
0 1 0 2 13  24  3 04  10  21  32  3 44
finish 2
'''
