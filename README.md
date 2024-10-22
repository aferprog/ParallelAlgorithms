# ParallelAlgorithms
## General idea
The goal of this repository is to present multi-thread solutions of several basic problems, providing some tools to simplify dividing huge problems to parts and do it with minimal resource consumption. 

## Multi-thread engine
The main pearl of a repository is the MultiThreadEngine class. It's a tool which provides the capability to create a pool of threads in advance and use them when needed. Created threads are sleeping and waiting for tasks, so they consume a little resources. It allows you to avoid extra creations and utilisations of threads and save much resources on it.

### Usage
#### Example of usage

Program:
```cpp
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
```


Output:
```
0 01  21  23  34  40  1 2 3 4
finish 1
0 1 0 2 13  24  3 04  10  21  32  3 44
finish 2
```

#### Step 1. Create a MultiThreadEngine instance.
You have to create a MultiThreadEngine instance in advance to use its advantages. Constructor takes a count of threads you need as a parameter. Currently you can't change this number later.

```cpp
// Creates a pool of 2 threads
MultiThreadEngine mte(2);
```

#### Step 2. Prepare a task.
A role of a task can serve any functor, which takes no parameters and returns void. Here is an example with a simple function which just prints numbers from 0 to 4.

```cpp
void test() {
    for (int i = 0; i < 5; ++i) {
        std::cout << i << '\n';
    }
}
```
#### Step 3. Put tasks into the engine.
You can send functions for performing with the MultiThreadEngine::addTask(std::function<void()>) method. Every thread takes the first task from the queue and removes it.
```cpp
    // Adding the same task 3 times
    mte.addTask(test);
    mte.addTask(test);
    mte.addTask(test);
```

#### Step 4. Waiting for result.
You can put your main thread to sleep until all queued tasks are completed with the MultiThreadEngine::wait() method. 
```cpp
// Creation of engine
MultiThreadEngine mte(2);

// Adding the task 3 times
mte.addTask(test);
mte.addTask(test);
mte.addTask(test);

// waiting until all tasks are finished
mte.wait();
std::cout << "finish 1" << std::endl;
```
#### Destructor
The destructor of MultiThreadEngine waits until the queue of tasks is empty and then joins all threads to the main thread. So it's a thread-safe class. This program demonstrates how it works:
```cpp
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

    std::cout << "\nfinish 2" << std::endl;
}
```
Output:
```
0 01  21  32  43  0 41  2 3 4
finish 1
0
finish 2
 10 1  2 2 3 34 0 1 2 4 3  0 1 2 3 4 4
```
#### Premature thread closing
If you want for some reason to cancel all tasks and close all threads immediately you can use MultiThreadEngine::stop(). In this case the queue of tasks will be cleaned and all sleeping threads will be closed immediately. But if a thread already began execution of a task, it's impossible to terminate so it will be closed after the finish of the execution. Example of such program:
```cpp
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
```
Output:
```
0 01  12 2 3 4  30  41  2 3 4
finish 1
0
finish 21
 2 3 4
```
## ParallelAlgorithms.h
This file contains in the namespace pal realizations of several basic algorithms. Currently all of them create their own threads to make parallel computations. 

### pal::sort_by_parts
Takes a random iterator to the begin and end of data and count of threads it's allowed to use. It divides the data into parts, sort them with std::sort and then merge them with std::inplace_merge.

Example of usage:
```cpp
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

int main()
{
    size_t len = 10000000;
    srand(time(NULL));
    std::vector<int> arr(len), arr1(len);
    for (int i = 0; i < len; i++)
        arr[i] = arr1[i] = rand();

    mesure_func([&arr]() {
        pal::sort_by_parts(arr.begin(), arr.end(), 10);
        });

    mesure_func([&arr1]() {
        std::sort(arr1.begin(), arr1.end());
        });

}
```
Output:
```Now
Time taken to fill array: 1.30108 seconds

Now
Time taken to fill array: 4.88208 seconds
```
### pal::make_by_pairs
This takes a random iterator to the beginning and end of data, a function which takes two arguments, and a count of threads it's allowed to use. This function applies the taken function to each pair. If the count of elements is odd, the last element will be ignored. It is used in pal::sort_by_parts to perform merging.
