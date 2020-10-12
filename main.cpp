#include "ThreadSafeQueue.h"

int main() {
    ThreadSafeQueue<int> safeQueue;

    std::thread([&]() {safeQueue.pop(); }).detach();
    std::thread([&]() {safeQueue.push(1); }).join();
    //safeQueue.pop();


    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}