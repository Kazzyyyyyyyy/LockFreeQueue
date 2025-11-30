#include <iostream>
#include <thread>
#include <atomic> 
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include <fstream>

#include "test-lockFreeQueues.h"

using namespace std; 
using namespace chrono; 

constexpr uint32_t        DEFAULT_THREAD_AMNT         = 10000;

class TestState {
    private: 
        enum Tests { PUSH = 0, POP, PUSH_AND_POP, EMPTY_POP, SINGLE_PUSH_AND_POP };
        
        uint16_t    workingTests,
                    errorTests;

        time_point<high_resolution_clock> startTime;
        vector<vector<string>> errorSafe;

    public: 
        //data
        TestState() {
            workingTests    = 0;
            errorTests      = 0;
            
            //init errorSafe
            for(uint8_t i = 0; i < 5/*Tests.size()*/; i++)
                errorSafe.push_back({});
        }

        const uint16_t get_working_tests() const {
            return workingTests; 
        }
        
        const uint16_t get_error_tests() const {
            return workingTests; 
        }
    
        void increase_count(const bool working) {
            if(working) workingTests++;
            else        errorTests++; 
        }


        //timer
        inline void start_clock() {
            startTime = high_resolution_clock::now();
        }

        const inline int get_current_clock_time() const {
            return duration_cast<minutes>(high_resolution_clock::now() - startTime).count();
        }
 

        //prints
        const string default_print(const uint32_t currThreadAmnt) const {
            return  string(100, '\n') + 
                    "(this run after print)\n" + 
                    "curr threads: " + to_string(currThreadAmnt) + "\n\n" + 

                    "working: " + to_string(workingTests) + "\n" + 
                    "error: " + to_string(errorTests)  + "\n\n" + 

                    "run: " + to_string((workingTests + errorTests) / 5) + "\n" +
                    "running for: " + to_string(get_current_clock_time()) + "m (" + to_string(((double)get_current_clock_time() / 60)) + 
                    "h)";
        }

        const string tests_finished_print() {
            string s =  "working: " + to_string(workingTests) + 
                        "\nerrors: "+ to_string(errorTests)+  
                        "\nruns: " + to_string((workingTests + errorTests) / 5) +
                        "\nran for: " + to_string(get_current_clock_time()) + "m (" + to_string(((double)get_current_clock_time() / 60)) + "h)" +
                        "\n"; 
            
                    
            s += "\n\nPUSH ERRORS:\n";
            for(uint8_t i = 0; i < errorSafe[PUSH].size(); i++) 
                s += errorSafe[PUSH][i] + "\n"; 
            

            s += "\n\nPOP ERRORS:\n";
            for(uint8_t i = 0; i < errorSafe[POP].size(); i++) 
                s += errorSafe[POP][i] + "\n"; 
            

            s += "\n\nPUSH_AND_POP ERRORS:\n";
            for(uint8_t i = 0; i < errorSafe[PUSH_AND_POP].size(); i++) 
                s += errorSafe[PUSH_AND_POP][i] + "\n"; 
            

            s += "\n\nEMPTY_POP ERRORS:\n";
            for(uint8_t i = 0; i < errorSafe[EMPTY_POP].size(); i++) 
                s += errorSafe[EMPTY_POP][i] + "\n"; 
            

            s += "\n\nSINGLE_PUSH_AND_POP ERRORS:\n";
            for(uint8_t i = 0; i < errorSafe[SINGLE_PUSH_AND_POP].size(); i++) 
                s += errorSafe[SINGLE_PUSH_AND_POP][i] + "\n"; 


            return s; 
        }


        //error prints
        void safe_push_error(const uint32_t failedAt, const uint32_t queueSize, const uint32_t currThreadAmnt) {
            const string s =    "failed at num: " + to_string(failedAt) + 
                                "\nqueue size: " + to_string(queueSize) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) +
                                "\nrun: " + to_string((workingTests + errorTests) / 5) +
                                "\n";

            errorSafe[PUSH].push_back(s);
        }

        void safe_pop_error(const uint32_t failedAt, const uint32_t popValsSize, const uint32_t queueSize, const uint32_t currThreadAmnt) {
            const string s =    "failed at num: " + to_string(failedAt) + 
                                "\npopVals size: " + to_string(popValsSize) +
                                "\nqueue size: " + to_string(queueSize) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                                "\n";
                    
            errorSafe[POP].push_back(s);
        }

        void safe_push_and_pop_error(const uint32_t failedAt, const uint32_t popValsSize, const uint32_t queueSize, const uint32_t currThreadAmnt) {
            const string s =    "failed at num: " + to_string(failedAt) + 
                                "\npopVals size: " + to_string(popValsSize) + 
                                "\nqueue size: " + to_string(queueSize) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                                "\n";
                                
            errorSafe[PUSH_AND_POP].push_back(s);
        }

        void safe_empty_pop_error(const bool failedAtFirstCheck, const uint32_t popValsSize, const uint32_t queueSize, const uint32_t currThreadAmnt) {
            string s = (failedAtFirstCheck ? "failed at: 1" : "failed at: 2"); 
            
            s +=                "\npopVals size: " + to_string(popValsSize) + 
                                "\nqueueSize: " + to_string(queueSize) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                                "\n";   

            errorSafe[EMPTY_POP].push_back(s);
        }

        void safe_single_push_and_pop_error(const uint32_t popValsSize, const uint32_t queueSize, const uint32_t falseReturnPops, const uint32_t currThreadAmnt) {
            const string s =    "popVals size: " + to_string(popValsSize) +
                                "\nqueueSize: " + to_string(queueSize) + 
                                "\nfalseReturnPops: " + to_string(falseReturnPops) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\nrun: " + to_string((workingTests + errorTests) / 5) +    
                                "\n"; 

            errorSafe[SINGLE_PUSH_AND_POP].push_back(s);
        }
}; 


class LockFreeQueueLinkedListTest { 
    private:
        TestState state;
    
        //queue node
        struct Node {
            const uint32_t data;
            atomic<Node*> next; 

            Node(const uint32_t &val) : data(val), next(nullptr) {}
        }; 
        
        
        //utils
        const inline uint32_t random_thread_count(const uint32_t min, const uint32_t max) const {
            static random_device rd;
            static mt19937 gen(rd());
            uniform_int_distribution<> dist(min, max);
            return dist(gen);
        }

        inline void push_threads(LockFreeQueueLinkedList<uint32_t> *q, const uint32_t threadAmnt) {
            vector<thread> v; 

            for(uint32_t i = 0; i < threadAmnt; i++)
                v.push_back(thread(&LockFreeQueueLinkedList<uint32_t>::push, q, i)); //create push threads with value 'i'
            
            for(auto &t : v) 
                t.join();
        } 

        const inline vector<uint32_t> pop_threads(LockFreeQueueLinkedList<uint32_t> *q, const uint32_t threadAmnt) {
            vector<thread> v;
            vector<uint32_t> vals(threadAmnt);

            for(uint32_t i = 0; i < threadAmnt; i++) 
                v.push_back(thread(&LockFreeQueueLinkedList<uint32_t>::pop, q, ref(vals[i]))); //create pop threads that return value to 'vals[i]'
            
            for(auto &t : v) 
                t.join();

            return vals; //returns popped values as vector<int>
        }


        //tests
        bool test_push(const uint32_t threadAmnt) {
            auto *q = new LockFreeQueueLinkedList<uint32_t>;
            push_threads(q, threadAmnt);

            //check if pushed correctly
            bool foundNum;
            Node *tmp;
            for(uint32_t num = 0; num < threadAmnt; num++) {
                tmp = (Node*)q->get_head(); 
                foundNum = false;

                while(tmp != nullptr) { //go through whole list (threads are async and push's are not sorted)
                    if(tmp->data == num) {
                        foundNum = true;
                        break;
                    }

                    tmp = tmp->next; //advance through queue
                }

                if(!foundNum) { //num not found => error in push
                    state.safe_push_error(num, q->size(), threadAmnt);
                    delete q; 
                    return false;
                }
            }

            //no error => working
            delete q; 
            return true;
        }

        bool test_pop(const uint32_t threadAmnt) {
            auto *q = new LockFreeQueueLinkedList<uint32_t>;

            push_threads(q, threadAmnt);
            vector<uint32_t> v = pop_threads(q, threadAmnt);

            sort(v.begin(), v.end()); //sort values, so check for value is O(1)

            for(uint32_t num = 0; num < v.size(); num++) {
                if(v[num] == num)
                    continue;
                
                //not all values got popped 
                state.safe_pop_error(num, v.size(), q->size(), threadAmnt); 
                delete q; 
                return false; 
            }
            
            //all values correct
            delete q; 
            return true; 
        }

        bool test_push_and_pop(const uint32_t threadAmnt) {
            auto *q = new LockFreeQueueLinkedList<uint32_t>; 

            //create threads
            vector<thread> pushThrs; 
            vector<thread> popThrs;
            
            for(uint32_t i = 0; i < threadAmnt; i++) 
                pushThrs.push_back(thread(&LockFreeQueueLinkedList<uint32_t>::push, q, i));

            //only half as many pops as pushes
            vector<uint32_t> popVals(threadAmnt); 
            for(uint32_t i = 0; i <= threadAmnt / 2; i++) 
                popThrs.push_back(thread(&LockFreeQueueLinkedList<uint32_t>::pop, q, ref(popVals[i])));

            //join
            for(uint32_t i = 0; i < threadAmnt; i++) {
                pushThrs[i].join(); 
                
                if(i <= threadAmnt / 2) {
                    popThrs[i].join(); 
                }
            }

            bool foundNum;
            Node *tmp;
            for(uint32_t num = 0; num < threadAmnt; num++) {
                tmp = (Node*)q->get_head();
                foundNum = false; 

                while(tmp != nullptr) { //go through whole list (threads are async and push's are not sorted)
                    if(tmp->data == num) {
                        foundNum = true; 
                        break;
                    }

                    tmp = tmp->next; //advance queue
                }

                if(!foundNum) { //'num' not found in queue

                    //50% of the values got popped, this means 50% of the pushed values are in 'popVals'
                    //so now that we didnt found 'num' in the queue, we check 'popVals' if it got popped (and catched correctly)
                    for(uint32_t i = 0; i < popVals.size(); i++) {
                        if(popVals[i] == num) {
                            foundNum = true; 
                            break;
                        }
                    }
                    
                    if(!foundNum) { //'num' also not found in 'popVals' => error
                        state.safe_push_and_pop_error(num, popVals.size(), q->size(), threadAmnt); 
                        delete q; 
                        return false;
                    }
                }
            }

            delete q; 
            return true; 
        }

        bool test_empty_pop(const uint32_t threadAmnt) {
            auto *q = new LockFreeQueueLinkedList<uint32_t>; 

            vector<uint32_t> v = pop_threads(q, threadAmnt); //pop without pushing

            for(uint32_t num = 0; num < v.size(); num++) {
                if(v[num] != 0) { //'v' gets initialized with all '0's, and because there are not values in queue to pop, 
                                  //all index's of 'v' should still be value '0'

                    const bool failedAtFirstCheck = true;
                    state.safe_empty_pop_error(failedAtFirstCheck, v.size(), q->size(), threadAmnt);
                    delete q;
                    return false;
                }
            }

            const bool working = (q->size() == 0); //check if queue size is '0', if yes => working

            if(!working) {
                const bool failedAtFirstCheck = false;
                state.safe_empty_pop_error(failedAtFirstCheck, v.size(), q->size(), threadAmnt); 
            }

            delete q; 
            return working;
        }

        bool test_single_push_and_pop(const uint32_t threadAmnt) {
            auto *q = new LockFreeQueueLinkedList<uint32_t>; 

            //create threads
            vector<thread> pushThrs; 
            vector<thread> popThrs;

            for(uint32_t i = 0; i < threadAmnt; i++) 
                pushThrs.push_back(thread(&LockFreeQueueLinkedList<uint32_t>::push, q, i));

            //1:1 push and pop
            vector<uint32_t> popVals(threadAmnt); 
            for(uint32_t i = 0; i < threadAmnt; i++) 
                popThrs.push_back(thread(&LockFreeQueueLinkedList<uint32_t>::pop, q, ref(popVals[i])));
            
            //join
            for(uint32_t i = 0; i < threadAmnt; i++) {
                pushThrs[i].join();
                popThrs[i].join(); 
            }
            
            //every push is followed by a pop, so size should be '0'
            bool working = (q->size() - q->get_return_pops() == 0); //why 'q->get_return_pops()'?
                                                                    //threads are async, sometimes the pop thread is faster than the push thread => 
                                                                    //pop gets returned because the queue is empty at that point in time
                                                                    //but this results in 2 push's in a row with only 1 pop, so the queueSize wont be 0 anymore
                                                                    //even though everything worked correctly as it should
            if(!working) {
                const bool failedAtFirstCheck = false;
                state.safe_single_push_and_pop_error(popVals.size(), q->size(), q->get_return_pops(), threadAmnt); 
            }

            delete q;
            return working;
        }


        //main 
        void tests_loop(uint32_t threadAmnt) {
            state.start_clock();

            while(true) {
                threadAmnt = random_thread_count(5000, 500'000);

                cout << state.default_print(threadAmnt) << endl; 

                state.increase_count(test_push(threadAmnt));
                state.increase_count(test_pop(threadAmnt));
                state.increase_count(test_empty_pop(threadAmnt));
                state.increase_count(test_push_and_pop(threadAmnt));
                state.increase_count(test_single_push_and_pop(threadAmnt));
            }
        }

        void wait_for_exit() {
            cin.get(); //input => exit
            
            const string printTo = ""; //add txt file to print 'tests_finished_print()' to
            
            cout << string(100, '\n') << state.tests_finished_print() << endl << endl << "print to: " << printTo;
            
            if(printTo.length() > 0) {
                ofstream file(printTo);
                file << state.tests_finished_print();
                file.close();
            }
                
            exit(0);
        }

    public: 
        void run_tests(const uint32_t threadAmnt = DEFAULT_THREAD_AMNT) {
            cout << "push: " << (test_push(threadAmnt) ? "working" : "error") << endl;
            cout << "pop: " << (test_pop(threadAmnt) ? "working" : "error") << endl; 
            cout << "pop on empty queue: " << (test_empty_pop(threadAmnt) ? "working" : "error") << endl;
            cout << "concurrent push & pop: " << (test_push_and_pop(threadAmnt) ? "working" : "error") << endl;
            cout << "single element concurrent push & pop: " << (test_single_push_and_pop(threadAmnt) ? "working" : "error") << endl;
        }

        void run_tests_loop() {
            thread testThread(&LockFreeQueueLinkedListTest::tests_loop, this, DEFAULT_THREAD_AMNT);
            thread exitThread(&LockFreeQueueLinkedListTest::wait_for_exit, this);
            
            testThread.join();
            exitThread.join(); 
        }
}; 















///very messy code ahead, soldier - 

///Dont really see a reason, to clean this code too.
///I think that I cleaned up the test code for the LinkedList queue, is enough proof that im able to do it. 
///I would just waste 1 more hour, to clean up code, that nobody ever is going to look at again... so I simply dont.

class LockFreeQueueArrayTest { 
    private:

        bool debug = false;
        
        size_t threadAmnt = 10000; 
        size_t workingTests = 0, 
               errorTests = 0; 

        enum Tests { PUSH = 0, POP, PUSH_AND_POP, EMPTY_POP, SINGLE_PUSH_AND_POP }; 
        vector<vector<string>> errorSafe; 

        void push_threads(LockFreeQueueArray<int> *q) {
            vector<thread> v; 
            for(int i = 0; i < threadAmnt; i++) {
                v.push_back(thread(&LockFreeQueueArray<int>::push, q, i));
            }

            for(auto &t : v) {
                t.join(); 
            }
        } 

        vector<int> pop_threads(LockFreeQueueArray<int> *q) {
            vector<thread> v;
            vector<int> vals(threadAmnt); 

            for(int i = 0; i < threadAmnt; i++) {
                v.push_back(thread(&LockFreeQueueArray<int>::pop, q, ref(vals[i])));
            }

            for(auto &t : v) {
                t.join();
            }

            return vals;
        }

        bool test_push() {
            auto *q = new LockFreeQueueArray<int>;
            push_threads(q);

            //check 
            size_t num = 0; //number we are currently looking for
            bool foundNum = false; //number found this round?
            
            const int *currArray = q->get_array(); 
            size_t currHead = q->get_head(); 
            size_t currTail = q->get_tail(); 

            for(int i = 0; i < threadAmnt; i++) {
                foundNum = false;

                for(int i = currHead; i < currTail; i++) { //go through whole list (threads are async and push's are not sorted)
                    if(currArray[i] == num) {
                        foundNum = true;
                        num++; //increase number for next round
                        break;
                    }
                }

                if(!foundNum || debug) {
                    currHead = q->get_head(); 
                    currTail = q->get_tail(); 
                    const string s = "failed at num: " + to_string(num) + 
                    "\nqueue size: " + to_string(currTail - currHead) +  
                    "\nqueue resizes: " + to_string(q->get_resizes()) + 
                    "\nqueue capacity: " + to_string(q->get_capacity()) + 
                    "\ncurrThreads: " + to_string(threadAmnt) + 
                    "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                    "\nworking: " + to_string(workingTests) + 
                    "\nerrors: " + to_string(errorTests) + "\n";
                    
                    errorSafe[PUSH].push_back(s);
                    delete q; 
                    return false; //num not found => error in push
                }
            }

            delete q; 
            return true; //no error => working
        }

        bool test_pop() {
            auto *q = new LockFreeQueueArray<int>;

            push_threads(q);
            vector<int> v = pop_threads(q); // v = values of all popped threads

            sort(v.begin(), v.end()); 
            
            int num = 0; 
            for(int val : v) {
                if(val != num || debug) {
                    
                    size_t currHead = q->get_head(); 
                    size_t currTail = q->get_tail(); 
                    const string s = "v size: " + to_string(v.size()) + 
                    "\nqueue size: " + to_string(currTail - currHead) +  
                    "\nqueue resizes: " + to_string(q->get_resizes()) + 
                    "\nqueue capacity: " + to_string(q->get_capacity()) + 
                    "\ncurrThreads: " + to_string(threadAmnt) + 
                    "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                    "\nworking: " + to_string(workingTests) + 
                    "\nerrors: " + to_string(errorTests) + "\n";
                    
                    errorSafe[POP].push_back(s);
                    delete q; 
                    return false;
                }

                num++; //increase number for next round
            }
            
            delete q; 
            return true; 
        }

        bool test_push_and_pop() {
            auto *q = new LockFreeQueueArray<int>; 

            //create threads
            vector<thread> pushThrs; 
            vector<thread> popThrs;
            
            for(int i = 0; i < threadAmnt; i++) {
                pushThrs.push_back(thread(&LockFreeQueueArray<int>::push, q, i));
            }

            //only halve as many pops as pushes
            vector<int> popVals(threadAmnt); 
            for(int i = 0; i <= threadAmnt / 2; i++) {
                popThrs.push_back(thread(&LockFreeQueueArray<int>::pop, q, ref(popVals[i])));
            }

            //join
            for(int i = 0; i < threadAmnt; i++) {
                pushThrs[i].join(); 
                
                if(i <= threadAmnt / 2) {
                    popThrs[i].join(); 
                }
            }

            size_t num = 0;
            bool foundNum = false;
            
            const int *currArray = q->get_array(); 
            size_t currHead = q->get_head(); 
            size_t currTail = q->get_tail(); 
            
            for(int i = 0; i < threadAmnt; i++) {
               
                foundNum = false;

                for(int i = currHead; i < currTail; i++) { //go through whole list (threads are async and push's are not sorted)
                    if(currArray[i]== num) {
                        foundNum = true;
                        num++; //increase number for next round
                        break;
                    }
                }

                //50% of the values got poped, this means 50% of the pushed values are in popVals
                //so that we didnt found num in the queue, we check popVals if it got popped (and catched correctly)
                if(!foundNum) {
                    for(int i = 0; i < popVals.size(); i++) {
                        if(popVals[i] == num) {
                            foundNum = true;
                            num++;
                            break; 
                        }
                    }

                    if(!foundNum || debug) { //also not in popVals => error
                        currHead = q->get_head();
                        currTail = q->get_tail(); 
            
                        const string s = "popVals size: " + to_string(popVals.size()) + 
                        "\nqueue size: " + to_string(currTail - currHead) +  
                        "\nqueue resizes: " + to_string(q->get_resizes()) + 
                        "\nqueue capacity: " + to_string(q->get_capacity()) + 
                        "\ncurrThreads: " + to_string(threadAmnt) + 
                        "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                        "\nworking: " + to_string(workingTests) + 
                        "\nerrors: " + to_string(errorTests) + "\n";
                        
                        errorSafe[PUSH_AND_POP].push_back(s);
                        
                        delete q; 
                        return false;
                    }
                }
            }

            delete q; 
            return true; 
        }


        bool test_empty_pop() {
            auto *q = new LockFreeQueueArray<int>; 

            vector<int> v = pop_threads(q);

            for(auto &val : v) {
                if(val != 0) { //vector<int> in pop_threads get auto initialized => full of int '0'
                    
                    size_t currHead = q->get_head(); 
                    size_t currTail = q->get_tail(); 
                    const string s = "failed at: 1\nv size: " + to_string(v.size()) + 
                    "\nqueueSize: " + to_string(currTail - currHead) +  
                    "\nqueue resizes: " + to_string(q->get_resizes()) + 
                    "\nqueue capacity: " + to_string(q->get_capacity()) + 
                    "\ncurrThreads: " + to_string(threadAmnt) + 
                    "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                    "\nworking: " + to_string(workingTests) + 
                    "\nerrors: " + to_string(errorTests) + "\n";
                    
                    errorSafe[EMPTY_POP].push_back(s);
                    delete q; 
                    return false;
                }
            }
            
            size_t currHead = q->get_head(); 
            size_t currTail = q->get_tail(); 

            const size_t queueSize = currTail - currHead; 
            bool suc = (queueSize == 0 ? true : false); 
            
            if(!suc || debug) {
                const string s = "failed at: 2\nv size: " + to_string(v.size()) + 
                "\nqueue size: " + to_string(queueSize) + 
                "\nqueue resizes: " + to_string(q->get_resizes()) + 
                "\nqueue capacity: " + to_string(q->get_capacity()) + 
                "\ncurrThreads: " + to_string(threadAmnt) + 
                "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                "\nworking: " + to_string(workingTests) + 
                "\nerrors: " + to_string(errorTests) + "\n";
                
                errorSafe[EMPTY_POP].push_back(s);
            }
            
            delete q; 
            return suc; //check if queue size is also 0, if yes => working
        }


        bool test_single_push_and_pop() {
            auto *q = new LockFreeQueueArray<int>; 

            //create threads
            vector<thread> pushThrs; 
            vector<thread> popThrs;

            for(int i = 0; i < threadAmnt; i++) {
                pushThrs.push_back(thread(&LockFreeQueueArray<int>::push, q, i));
            }

            //1:1 push and pop
            vector<int> popVals(threadAmnt); 
            for(int i = 0; i < threadAmnt; i++) {
                popThrs.push_back(thread(&LockFreeQueueArray<int>::pop, q, ref(popVals[i])));
            }

            //join
            for(int i = 0; i < threadAmnt; i++) {
                pushThrs[i].join();
                popThrs[i].join(); 
            }

            size_t currHead = q->get_head(), 
                   currTail = q->get_tail(); 

            const size_t queueSize = currTail - currHead, 
                         falseReturnPops = q->get_return_pops(); //threads are async, sometimes the pop thread is faster than the push thread => 
                                                                 //pop gets returned because the queue is empty at that point in time
                                                                 //but this results in 2 push's in a row with only 1 pop, so the queueSize wont be 0
                                                                 //even though everything worked correctly

            bool suc = (queueSize - falseReturnPops) == 0;
            if(!suc|| debug) {
                const string s = "queueSize: " + to_string(queueSize) +  
                "\nqueue resizes: " + to_string(q->get_resizes()) + 
                "\nqueue capacity: " + to_string(q->get_capacity()) + 
                "\nfalseReturnPops: " + to_string(falseReturnPops) + 
                "\ncurrThreads: " + to_string(threadAmnt) + 
                "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                "\nworking: " + to_string(workingTests) + 
                "\nerrors: " + to_string(errorTests) + "\n";

                errorSafe[SINGLE_PUSH_AND_POP].push_back(s);
            }

            delete q; 
            return suc; //every push is followed by a pop, so size should be 0
        }

    
        void increase_count(bool working) {
            if(working) 
                workingTests++; 
            else 
                errorTests++; 
        }
        
        int random_int(int min, int max) {
            static random_device rd;
            static mt19937 gen(rd());
            uniform_int_distribution<> dist(min, max);
            return dist(gen);
        }

        int currTime; 
        void run_silent_tests() {
            auto start = high_resolution_clock::now();
            string clear(100, '\n');

            for(int i = 0; i < 5; i++) {
                errorSafe.push_back({});
            }

            while(true) {
                threadAmnt = random_int(100, 500000); 
                
                auto curr = high_resolution_clock::now();
                auto m = duration_cast<minutes>(curr - start).count();

                currTime = m; 

                cout << clear 
                     << "(run after print)" << endl
                     << "curr threads: " << threadAmnt << endl << endl 
                     << "working: " << workingTests << endl 
                     << "error: " << errorTests << endl << endl 

                     << "run: " << (workingTests + errorTests) / 5 << endl
                     << "running for: " << m << "m (" << ((double)m / 60) << "h)";

                increase_count(test_push());
                increase_count(test_pop());
                increase_count(test_empty_pop());
                increase_count(test_push_and_pop());
                increase_count(test_single_push_and_pop());
            }
        }

        void wait_for_exit() {
            string asdbajsdbsjadb; 
            cin >> asdbajsdbsjadb; 

            cout << string(100, '\n'); 

            //print data
            string s = "working: " + to_string(workingTests) + 
                       "\nerrors: "+ to_string(errorTests)+  
                        "\nruns: " + to_string((workingTests + errorTests) / 5) +
                        "\nran for: " + to_string(currTime) + "m (" + to_string(((double)currTime / 60)) + "h)" +
                        
                        "\n\n\n\nPUSH ERRORS:\n";

            for(int i = 0; i < errorSafe[PUSH].size(); i++) {
                s += errorSafe[PUSH][i] + "\n"; 
            }

            s += "\n\nPOP ERRORS:\n";
            for(int i = 0; i < errorSafe[POP].size(); i++) {
                s += errorSafe[POP][i] + "\n"; 
            }
                        
            s += "\n\nPUSH_AND_POP ERRORS:\n";
            for(int i = 0; i < errorSafe[PUSH_AND_POP].size(); i++) {
                s += errorSafe[PUSH_AND_POP][i] + "\n"; 
            }
                        
            s += "\n\nEMPTY_POP ERRORS:\n";
            for(int i = 0; i < errorSafe[EMPTY_POP].size(); i++) {
                s += errorSafe[EMPTY_POP][i] + "\n"; 
            }
                        
            s += "\n\nSINGLE_PUSH_AND_POP ERRORS:\n";
            for(int i = 0; i < errorSafe[SINGLE_PUSH_AND_POP].size(); i++) {
                s += errorSafe[SINGLE_PUSH_AND_POP][i] + "\n"; 
            }

            const string printTo = ""; //add txt file to print to
            cout << s << endl << "\n\nprinted to: " << printTo; 

            if(printTo.length() > 0) {
                ofstream file(printTo);
                file << s; 
                file.close(); 
            }

            exit(0);
        }

    public: 
        void run_tests() {
            cout << "push: " << (test_push() ? "working" : "error") << endl;
            cout << "pop: " << (test_pop() ? "working" : "error") << endl; 
            cout << "pop on empty queue: " << (test_empty_pop() ? "working" : "error") << endl;
            cout << "concurrent push & pop: " << (test_push_and_pop() ? "working" : "error") << endl;
            cout << "single element concurrent push & pop: " << (test_single_push_and_pop() ? "working" : "error") << endl;
        }

        void run_tests_loop() {
            thread testThread(&LockFreeQueueArrayTest::run_silent_tests, this);
            thread exitThread(&LockFreeQueueArrayTest::wait_for_exit, this);
            
            testThread.join(); 
            exitThread.join();
        }
}; 