#include <iostream> 
#include "test.cpp"
#include <thread>
#include <vector>

using namespace std; 

int main() {
    LockFreeQueueLinkedListTest qt; 
    qt.run_tests_loop();

    return 0; 
}