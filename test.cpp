#include <iostream> 
#include "lockFreeQueues.h"
#include <vector>
#include <thread>
#include <atomic>
#include <algorithm>
#include <chrono>
#include <string>
#include <random>
#include <fstream>

using namespace std; 
using namespace chrono; 

class LockFreeQueueLinkedListTest { 
    private:
        size_t threadAmnt = 10000; 
        size_t workingTests = 0, 
               errorTests = 0; 

        enum Tests { PUSH = 0, POP, PUSH_AND_POP, EMPTY_POP, SINGLE_PUSH_AND_POP }; 
        vector<vector<string>> errorSafe; 

        struct Node {
            const int data;
            atomic<Node*> next; 

            Node(const int& val) : data(val), next(nullptr) {}
        }; 

        void push_threads(LockFreeQueueLinkedList<int> *q) {
            vector<thread> v; 
            for(int i = 0; i < threadAmnt; i++) {
                v.push_back(thread(&LockFreeQueueLinkedList<int>::push, q, i));
            }

            for(auto &t : v) {
                t.join(); 
            }
        } 

        vector<int> pop_threads(LockFreeQueueLinkedList<int> *q) {
            vector<thread> v;
            vector<int> vals(threadAmnt); 

            for(int i = 0; i < threadAmnt; i++) {
                v.push_back(thread(&LockFreeQueueLinkedList<int>::pop, q, ref(vals[i])));
            }

            for(auto &t : v) {
                t.join();
            }

            return vals;
        }


        bool test_push() {
            auto *q = new LockFreeQueueLinkedList<int>;
            push_threads(q);

            //check 
            size_t num = 0; //number we are currently looking for
            bool foundNum = false; //number found this round?
            Node *tmp;

            for(int i = 0; i < threadAmnt; i++) {
                tmp = (Node*)q->get_head(); 
                foundNum = false;

                while(tmp != nullptr) { //go through whole list (threads are async and push's are not sorted)
                    if(tmp->data == num) {
                        foundNum = true;
                        num++; //increase number for next round
                        break;
                    }

                    tmp = tmp->next; //advance through queue
                }

                if(!foundNum) {
                    const string s = "failed at num: " + to_string(num) + 
                    "\nqueue size: " + to_string(q->size()) + 
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
            auto *q = new LockFreeQueueLinkedList<int>;

            push_threads(q);
            vector<int> v = pop_threads(q); // v = values of all popped threads

            sort(v.begin(), v.end()); 

            int num = 0; 
            for(int val : v) {
                if(val != num) {
                    
                    const string s = "v size: " + to_string(v.size()) + 
                    "\nqueue size: " + to_string(q->size()) + 
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
            auto *q = new LockFreeQueueLinkedList<int>; 

            //create threads
            vector<thread> pushThrs; 
            vector<thread> popThrs;
            
            for(int i = 0; i < threadAmnt; i++) {
                pushThrs.push_back(thread(&LockFreeQueueLinkedList<int>::push, q, i));
            }

            //only halve as many pops as pushes
            vector<int> popVals(threadAmnt); 
            for(int i = 0; i <= threadAmnt / 2; i++) {
                popThrs.push_back(thread(&LockFreeQueueLinkedList<int>::pop, q, ref(popVals[i])));
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
            Node *tmp;

            for(int i = 0; i < threadAmnt; i++) {
                tmp = (Node*)q->get_head();
                foundNum = false;

                while(tmp != nullptr) { //go through whole list (threads are async and push's are not sorted)
                    if(tmp->data == num) {
                        foundNum = true;
                        num++; //increase number for next round
                        break;
                    }

                    tmp = tmp->next; //advance queue
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

                    if(!foundNum) { //also not in popVals => error
                        
                        const string s = "popVals size: " + to_string(popVals.size()) + 
                        "\nqueue size: " + to_string(q->size()) + 
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
            auto *q = new LockFreeQueueLinkedList<int>; 

            vector<int> v = pop_threads(q);

            for(auto &val : v) {
                if(val != 0) { //vector<int> in pop_threads get auto initialized => full of int '0'
                    
                    const string s = "failed at: 1\nv size: " + to_string(v.size()) + 
                    "\nqueueSize: " + to_string(q->size()) + 
                    "\ncurrThreads: " + to_string(threadAmnt) + 
                    "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                    "\nworking: " + to_string(workingTests) + 
                    "\nerrors: " + to_string(errorTests) + "\n";
                    
                    errorSafe[EMPTY_POP].push_back(s);
                    delete q; 
                    return false;
                }
            }

            const size_t queueSize = q->size(); 
            delete q; 
            bool suc = (queueSize == 0 ? true : false); 
           
            if(!suc) {
                const string s = "failed at: 2\nv size: " + to_string(v.size()) + 
                "\nqueue size: " + to_string(queueSize) + 
                "\ncurrThreads: " + to_string(threadAmnt) + 
                "\nrun: " + to_string((workingTests + errorTests) / 5) + 
                "\nworking: " + to_string(workingTests) + 
                "\nerrors: " + to_string(errorTests) + "\n";
                
                errorSafe[EMPTY_POP].push_back(s);
            }

            return suc; //check if queue size is also 0, if yes => working
        }


        bool test_single_push_and_pop() {
            auto *q = new LockFreeQueueLinkedList<int>; 

            //create threads
            vector<thread> pushThrs; 
            vector<thread> popThrs;

            for(int i = 0; i < threadAmnt; i++) {
                pushThrs.push_back(thread(&LockFreeQueueLinkedList<int>::push, q, i));
            }

            //1:1 push and pop
            vector<int> popVals(threadAmnt); 
            for(int i = 0; i < threadAmnt; i++) {
                popThrs.push_back(thread(&LockFreeQueueLinkedList<int>::pop, q, ref(popVals[i])));
            }

            //join
            for(int i = 0; i < threadAmnt; i++) {
                pushThrs[i].join();
                popThrs[i].join(); 
            }

            const size_t queueSize = q->size(), 
                         falseReturnPops = q->get_return_pops(); //threads are async, sometimes the pop thread is faster than the push thread => 
                                                                 //pop gets returned because the queue is empty at that point in time
                                                                 //but this results in 2 push's in a row with only 1 pop, so the queueSize wont be 0
                                                                 //even though everything worked correctly

            bool suc = (queueSize - falseReturnPops) == 0;
            if(!suc) {
                const string s = "queueSize: " + to_string(queueSize) + "\nfalseReturnPops: " + to_string(falseReturnPops) + "\ncurrThreads: " + to_string(threadAmnt) + 
                                 "\nrun: " + to_string((workingTests + errorTests) / 5) + "\nworking: " + to_string(workingTests) + "\nerrors: " + to_string(errorTests) + "\n";
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

        int currTime; 
        void run_silent_tests() {
            auto start = high_resolution_clock::now();
            string clear(100, '\n');
            srand(time(0));

            for(int i = 0; i < 5; i++) {
                errorSafe.push_back({});
            }

            while(true) {
                ///change, rand() is bad 
                threadAmnt = rand() % 500000; 
                
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

            const string printTo = "C:\\Users\\staer\\Desktop\\LockFreeQueueReports\\linkedlistreport.txt";
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
            thread testThread(&LockFreeQueueLinkedListTest::run_silent_tests, this);
            thread exitThread(&LockFreeQueueLinkedListTest::wait_for_exit, this);
            
            testThread.join(); 
            exitThread.join();
        }
}; 