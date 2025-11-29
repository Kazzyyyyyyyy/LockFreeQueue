#include <iostream>
#include <thread>
#include <atomic> 
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include <fstream>

#include "lockFreeQueues.h"

using namespace std; 
using namespace chrono; 

constexpr size_t        DEFAULT_THREAD_AMNT         = 10000;

class TestState {
    private: 
        enum Tests { PUSH = 0, POP, PUSH_AND_POP, EMPTY_POP, SINGLE_PUSH_AND_POP }; 
        
        size_t      workingTests,
                    errorTests; 

        time_point<high_resolution_clock> startTime;
        vector<vector<string>> errorSafe; 

    public: 

        //data
        TestState(const size_t &wt = 0, const size_t &et = 0) {
            workingTests    = wt; 
            errorTests      = et;
            
            //init errorSafe
            for(int i = 0; i < 5/*Tests.size()*/; i++)
                errorSafe.push_back({});
        }

        void reset(const size_t &wt = 0, const size_t &et = 0) {
            workingTests    = wt;
            errorTests      = et;

            errorSafe.clear();
        }

        const size_t get_working_tests() const {
            return workingTests; 
        }
        
        const size_t get_error_tests() const {
            return workingTests; 
        }
    
        void increase_count(bool working) {
            if(working) 
                workingTests++; 
            else 
                errorTests++; 
        }


        //time 
        inline void start_clock() {
            startTime = high_resolution_clock::now();
        }

        const inline int get_current_clock_time() const {
            return duration_cast<minutes>(high_resolution_clock::now() - startTime).count();
        }

        //idk 
        const string clear_console() const {
              return string(100, '\n');
        }
 
        const string default_print(const size_t currThreadAmnt) const {
            return clear_console() +
                    "(this run after print)\n" + 
                    "curr threads: " + to_string(currThreadAmnt) + "\n\n" + 

                    "working: " + to_string(workingTests) + "\n" + 
                    "error: " + to_string(errorTests)  + "\n\n" + 

                    "run: " + to_string((workingTests + errorTests) / 5) + "\n" +
                    "running for: " + to_string(get_current_clock_time()) + "m (" + to_string(((double)get_current_clock_time() / 60)) + 
                    "h)";
        }


        //error handling
        void push_print(const size_t num, const size_t queueSize, const size_t currThreadAmnt) {
            const string s =    "failed at num: " + to_string(num) + 
                                "\nqueue size: " + to_string(queueSize) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\n";

            errorSafe[PUSH].push_back(s);
        }

        void pop_print(const size_t size, const size_t queueSize, const size_t currThreadAmnt) {
            const string s =    "v size: " + to_string(size) + 
                                "\nqueue size: " + to_string(queueSize) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\n";
                    
            errorSafe[POP].push_back(s);
        }


        void push_and_pop_print(const size_t size, const size_t queueSize, const size_t currThreadAmnt) {
            const string s =    "popVals size: " + to_string(size) + 
                                "\nqueue size: " + to_string(queueSize) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\n";
                                
            errorSafe[PUSH_AND_POP].push_back(s);
        }

        void empty_pop_print(const bool &failedAtFirstCheck, const size_t size, const size_t queueSize, const size_t currThreadAmnt) {
            string s = (failedAtFirstCheck ? "failed at: 1" : "failed at: 2"); 
            
            s +=                "\nv size: " + to_string(size) + 
                                "\nqueueSize: " + to_string(queueSize) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\n";   

            errorSafe[EMPTY_POP].push_back(s);
        }

        void single_push_and_pop_print(const size_t queueSize, const size_t falseReturnPops, const size_t currThreadAmnt) {
            const string s =    "queueSize: " + to_string(queueSize) + 
                                "\nfalseReturnPops: " + to_string(falseReturnPops) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nrun: " + to_string((workingTests + errorTests) / 5) +    
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\n";

            errorSafe[SINGLE_PUSH_AND_POP].push_back(s);
        }


        const string test_finished_print() {
            //print data
            string s =  "working: " + to_string(workingTests) + 
                        "\nerrors: "+ to_string(errorTests)+  
                        "\nruns: " + to_string((workingTests + errorTests) / 5) +
                        "\nran for: " + to_string(get_current_clock_time()) + "m (" + to_string(((double)get_current_clock_time() / 60)) + "h)" +
                        "\n"; 
            
                    
            s += "\n\nPUSH ERRORS:\n";
            for(int i = 0; i < errorSafe[PUSH].size(); i++) 
                s += errorSafe[PUSH][i] + "\n"; 
            

            s += "\n\nPOP ERRORS:\n";
            for(int i = 0; i < errorSafe[POP].size(); i++) 
                s += errorSafe[POP][i] + "\n"; 
            

            s += "\n\nPUSH_AND_POP ERRORS:\n";
            for(int i = 0; i < errorSafe[PUSH_AND_POP].size(); i++) 
                s += errorSafe[PUSH_AND_POP][i] + "\n"; 
            

            s += "\n\nEMPTY_POP ERRORS:\n";
            for(int i = 0; i < errorSafe[EMPTY_POP].size(); i++) 
                s += errorSafe[EMPTY_POP][i] + "\n"; 
            

            s += "\n\nSINGLE_PUSH_AND_POP ERRORS:\n";
            for(int i = 0; i < errorSafe[SINGLE_PUSH_AND_POP].size(); i++) 
                s += errorSafe[SINGLE_PUSH_AND_POP][i] + "\n"; 


            return s; 
        }
}; 





class LockFreeQueueLinkedListTest { 
    private:
        struct Node {
            const int data;
            atomic<Node*> next; 

            Node(const int &val) : data(val), next(nullptr) {}
        }; 

        TestState state; 

        void push_threads(LockFreeQueueLinkedList<int> *q, const size_t threadAmnt) {
            vector<thread> v; 
            for(int i = 0; i < threadAmnt; i++)
                v.push_back(thread(&LockFreeQueueLinkedList<int>::push, q, i));
            
            for(auto &t : v)  
                t.join();
        } 

        vector<int> pop_threads(LockFreeQueueLinkedList<int> *q, const size_t threadAmnt) {
            vector<thread> v;
            vector<int> vals(threadAmnt);

            for(int i = 0; i < threadAmnt; i++) 
                v.push_back(thread(&LockFreeQueueLinkedList<int>::pop, q, ref(vals[i])));
            
            for(auto &t : v) 
                t.join();

            return vals; //returns, popped values as vector<int>
        }

        bool test_push(const size_t threadAmnt) {
            auto *q = new LockFreeQueueLinkedList<int>;
            push_threads(q, threadAmnt);

            //check if pushed correctly
            bool foundNum; 
            Node *tmp;
            for(int num = 0; num < threadAmnt; num++) {
                tmp = (Node*)q->get_head(); 
                foundNum = false;

                while(tmp != nullptr) { //go through whole list (threads are async and push's are not sorted)
                    if(tmp->data == num) {
                        foundNum = true;
                        break;
                    }

                    tmp = tmp->next; //advance through queue
                }

                if(foundNum)
                    continue;

                //num not found => error in push
                state.push_print(num, q->size(), threadAmnt);
                delete q; 
                return false;
            }

            //no error => working
            delete q; 
            return true;
        }

        bool test_pop(const size_t threadAmnt) {
            auto *q = new LockFreeQueueLinkedList<int>;

            push_threads(q, threadAmnt);
            vector<int> v = pop_threads(q, threadAmnt);

            sort(v.begin(), v.end()); 

            for(int num = 0; num < v.size(); num++) {
                if(v[num] == num)
                    continue;
                
                //not all values got popped 
                state.pop_print(num, v.size(), threadAmnt); 
                delete q; 
                return false; 
            }
            
            //all values correct
            delete q; 
            return true; 
        }

        bool test_push_and_pop(const size_t threadAmnt) {
            auto *q = new LockFreeQueueLinkedList<int>; 

            //create threads
            vector<thread> pushThrs; 
            vector<thread> popThrs;
            
            for(int i = 0; i < threadAmnt; i++) 
                pushThrs.push_back(thread(&LockFreeQueueLinkedList<int>::push, q, i));

            //only halve as many pops as pushes
            vector<int> popVals(threadAmnt); 
            for(int i = 0; i <= threadAmnt / 2; i++) 
                popThrs.push_back(thread(&LockFreeQueueLinkedList<int>::pop, q, ref(popVals[i])));

            //join
            for(int i = 0; i < threadAmnt; i++) {
                pushThrs[i].join(); 
                
                if(i <= threadAmnt / 2) {
                    popThrs[i].join(); 
                }
            }

            bool foundNum;
            Node *tmp;
            for(int num = 0; num < threadAmnt; num++) {
                tmp = (Node*)q->get_head();
                foundNum = false; 

                while(tmp != nullptr) { //go through whole list (threads are async and push's are not sorted)
                    if(tmp->data == num) {
                        foundNum = true; 
                        break;
                    }

                    tmp = tmp->next; //advance queue
                }

                if(foundNum)
                    continue;

                //50% of the values got popped, this means 50% of the pushed values are in popVals
                //so that we didnt found num in the queue, we check popVals if it got popped (and catched correctly)
                for(int i = 0; i < popVals.size(); i++) {
                    if(popVals[i] == num) {
                        foundNum = true; 
                        break;
                    }
                }
                        
                if(foundNum)
                    continue;

                //still not found => error
                state.push_and_pop_print(num, q->size(), threadAmnt); 
                delete q; 
                return false;
            }

            delete q; 
            return true; 
        }


        bool test_empty_pop(const size_t threadAmnt) {
            auto *q = new LockFreeQueueLinkedList<int>; 

            vector<int> v = pop_threads(q, threadAmnt);

            for(int num = 0; num < v.size(); num++) {
                if(v[num] != 0) { //v gets initialized with '0's 
                    const bool failedAtFirstCheck = true;
                    state.empty_pop_print(failedAtFirstCheck, v.size(), q->size(), threadAmnt);
                    delete q;
                    return false;
                }
            }

            const bool working = (q->size() == 0);  

            if(!working) {
                const bool failedAtFirstCheck = false;
                state.empty_pop_print(failedAtFirstCheck, v.size(), q->size(), threadAmnt); 
            }

            delete q; 
            return working; //check if queue size is also 0, if yes => working
        }


        bool test_single_push_and_pop(const size_t threadAmnt) {
            auto *q = new LockFreeQueueLinkedList<int>; 

            //create threads
            vector<thread> pushThrs; 
            vector<thread> popThrs;

            for(int i = 0; i < threadAmnt; i++) 
                pushThrs.push_back(thread(&LockFreeQueueLinkedList<int>::push, q, i));

            //1:1 push and pop
            vector<int> popVals(threadAmnt); 
            for(int i = 0; i < threadAmnt; i++) 
                popThrs.push_back(thread(&LockFreeQueueLinkedList<int>::pop, q, ref(popVals[i])));
            
            //join
            for(int i = 0; i < threadAmnt; i++) {
                pushThrs[i].join();
                popThrs[i].join(); 
            }

            bool suc = (q->size() - q->get_return_pops() == 0); //threads are async, sometimes the pop thread is faster than the push thread => 
                                                                 //pop gets returned because the queue is empty at that point in time
                                                                 //but this results in 2 push's in a row with only 1 pop, so the queueSize wont be 0
                                                                 //even though everything worked correctly
            if(!suc) {
                state.single_push_and_pop_print(q->size(), q->get_return_pops(), threadAmnt);
            }

            delete q; 
            return suc; //every push is followed by a pop, so size should be 0
        }
        
        const int random_int(const int min, const int max) const {
            static random_device rd;
            static mt19937 gen(rd());
            uniform_int_distribution<> dist(min, max);
            return dist(gen);
        }

        void run_silent_tests(size_t threadAmnt = DEFAULT_THREAD_AMNT) {
            while(true) {
                threadAmnt = random_int(5000, 500000);

                cout << state.default_print(threadAmnt) << endl; 

                state.increase_count(test_push(threadAmnt));
                state.increase_count(test_pop(threadAmnt));
                state.increase_count(test_empty_pop(threadAmnt));
                state.increase_count(test_push_and_pop(threadAmnt));
                state.increase_count(test_single_push_and_pop(threadAmnt));
            }
        }

        void wait_for_exit() {
            // string asdbajsdbsjadb; 
            // cin >> asdbajsdbsjadb; 

            // cout << string(100, '\n'); 

            // ////////jhasbDJHASbdojashbda uhsbdla jhdbasljhbdasjlh bdasjlhbdj habsjhb
            // const string printTo = "C:\\Users\\staer\\Desktop\\LockFreeQueueReports\\linkedlistreport.txt";
            // ////////jhasbDJHASbdojashbda uhsbdla jhdbasljhbdasjlh bdasjlhbdj habsjhb
          
            // cout << s << endl << endl << endl << "printed to: " << printTo; 
            // ofstream file(printTo);
            // file << s; 
            // file.close(); 

            // exit(1);
        }

    public: 
        void run_tests(const size_t threadAmnt = DEFAULT_THREAD_AMNT) {
            cout << "push: " << (test_push(threadAmnt) ? "working" : "error") << endl;
            cout << "pop: " << (test_pop(threadAmnt) ? "working" : "error") << endl; 
            cout << "pop on empty queue: " << (test_empty_pop(threadAmnt) ? "working" : "error") << endl;
            cout << "concurrent push & pop: " << (test_push_and_pop(threadAmnt) ? "working" : "error") << endl;
            cout << "single element concurrent push & pop: " << (test_single_push_and_pop(threadAmnt) ? "working" : "error") << endl;
        }

        void run_tests_loop() {
            thread testThread(&LockFreeQueueLinkedListTest::run_silent_tests, this, DEFAULT_THREAD_AMNT);
            // thread exitThread(&LockFreeQueueLinkedListTest::wait_for_exit, this);
            
            testThread.join(); 
            // exitThread.join();
        }
}; 















































































































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

            const string printTo = "C:\\Users\\staer\\Desktop\\LockFreeQueueReports\\arrayreport.txt";
            cout << s << endl << "\n\nprinted to: " << printTo; 

            ofstream file(printTo);
            file << s; 
            file.close(); 

            exit(1);
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