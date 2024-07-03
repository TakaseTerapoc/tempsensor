#include <iostream>
#include <thread>

// 
void printCoreInfo(const std::string& threadName) {
    int core = sched_getcpu();
    std::cout << threadName << " running on core: " << core << std::endl;
}

void threadFunction1() {
    for (int i = 0; i < 10000; ++i) {
        printCoreInfo("Thread 1");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));// 0.5秒待つ
        printCoreInfo("Thread 1_2");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        printCoreInfo("Thread 1_3");
    }
}

void threadFunction2() {
    for (int i = 0; i < 10000; ++i) {
        printCoreInfo("Thread 2_1");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));// 0.5秒待つ
        printCoreInfo("Thread 2_2");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));// 0.5秒待つ
    }
}

int main() {
    std::thread t1(threadFunction1);
    std::thread t2(threadFunction2);

    t1.join();
    t2.join();

    return 0;
}
