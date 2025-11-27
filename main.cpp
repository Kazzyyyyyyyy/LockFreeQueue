#include <iostream> 
#include "test.cpp"
#include <thread>
#include <vector>

using namespace std; 

int main() {
    LockFreeQueueArrayTest qt; 
    qt.run_tests_loop();

    return 0; 
}