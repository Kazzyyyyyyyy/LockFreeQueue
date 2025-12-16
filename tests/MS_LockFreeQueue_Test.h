#include <cstdint>
#include <chrono> 
#include <vector>
#include <algorithm>
#include <random>
#include <string>
#include <iostream> 
#include <thread> 
#include <fstream>

#include "../MS_LockFreeQueue.h"

using namespace std; 
using namespace chrono; 

class TestState {
    private: 
        enum Tests { st_PUSH = 0, st_POP, st_EMPTY_POP, mt_PUSH, mt_POP, mt_PUSH_AND_POP, mt_EMPTY_POP, mt_SINGLE_PUSH_AND_POP };
        const uint8_t TESTS_SIZE = 8; 

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
            for(uint8_t i = 0; i < TESTS_SIZE; i++)
                errorSafe.push_back({});
        }

        const inline uint16_t get_working_tests() const {
            return workingTests; 
        }
        
        const inline uint16_t get_error_tests() const {
            return workingTests; 
        }
    
        inline void increase_count(const bool working) {
            if(working) workingTests++;
            else        errorTests++;
        }
        
        const inline string get_total_tests() const {
            return to_string((workingTests + errorTests) / TESTS_SIZE); 
        }


        //timer
        inline void start_clock() {
            startTime = high_resolution_clock::now();
        }

        const inline string get_current_clock_time() const {
            return to_string(duration_cast<minutes>(high_resolution_clock::now() - startTime).count()); 
        }

        const inline string get_current_hour_clock_time() const {
            return to_string(((double)duration_cast<minutes>(high_resolution_clock::now() - startTime).count() / 60));
        }
 

        //prints
        const string default_print(const uint32_t currThreadAmnt) const {
            return  string(100, '\n') + 
                    "(this run after print)\n" + 
                    "curr threads: " + to_string(currThreadAmnt) + "\n\n" + 

                    "working: " + to_string(workingTests) + "\n" + 
                    "error: " + to_string(errorTests)  + "\n\n" + 

                    "run: " + get_total_tests() + "\n" +
                    "running for: " + get_current_clock_time() + "m (" + get_current_hour_clock_time() + 
                    "h)";
        }

        const string tests_finished_print() {
            string s =  "working: " + to_string(workingTests) + 
                        "\nerrors: "+ to_string(errorTests)+  
                        "\nruns: " + get_total_tests() +
                        "\nran for: " + get_current_clock_time() + "m (" + get_current_hour_clock_time() + 
                        "h)\n";
                     
            s += "\n\nst_PUSH ERRORS:\n";
            for(uint16_t i = 0; i < errorSafe[st_PUSH].size(); i++) {
                s += errorSafe[st_PUSH][i] + "\n"; 
            }
            
            s += "\n\nst_POP ERRORS:\n";
            for(uint16_t i = 0; i < errorSafe[st_POP].size(); i++) {
                s += errorSafe[st_POP][i] + "\n"; 
            }           

            s += "\n\nst_EMPTY_POP ERRORS:\n";
            for(uint16_t i = 0; i < errorSafe[st_EMPTY_POP].size(); i++) {
                s += errorSafe[st_EMPTY_POP][i] + "\n"; 
            }

            s += "\n\nmt_PUSH ERRORS:\n";
            for(uint16_t i = 0; i < errorSafe[mt_PUSH].size(); i++) {
                s += errorSafe[mt_PUSH][i] + "\n"; 
            }

            s += "\n\nmt_POP ERRORS:\n";
            for(uint16_t i = 0; i < errorSafe[mt_POP].size(); i++) {
                s += errorSafe[mt_POP][i] + "\n"; 
            }
            

            s += "\n\nmt_PUSH_AND_POP ERRORS:\n";
            for(uint16_t i = 0; i < errorSafe[mt_PUSH_AND_POP].size(); i++) {
                s += errorSafe[mt_PUSH_AND_POP][i] + "\n"; 
            }
            

            s += "\n\nmt_EMPTY_POP ERRORS:\n";
            for(uint16_t i = 0; i < errorSafe[mt_EMPTY_POP].size(); i++) {
                s += errorSafe[mt_EMPTY_POP][i] + "\n"; 
            }
            

            s += "\n\nmt_SINGLE_PUSH_AND_POP ERRORS:\n";
            for(uint16_t i = 0; i < errorSafe[mt_SINGLE_PUSH_AND_POP].size(); i++) {
                s += errorSafe[mt_SINGLE_PUSH_AND_POP][i] + "\n"; 
            }
                

            return s; 
        }


        //errors 
        void safe_st_push_error(const uint32_t failedAt, const uint32_t queueSize) {
            const string s =    "failed at num: " + to_string(failedAt) + 
                                "\nqueue size: " + to_string(queueSize) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) +
                                "\nrun: " + get_total_tests() +
                                "\n";

            errorSafe[st_PUSH].push_back(s);
        }

        void safe_st_pop_error(const uint32_t failedAt, const uint32_t popValsSize, const uint32_t queueSize) {
            const string s =    "failed at num: " + to_string(failedAt) + 
                                "\npopVals size: " + to_string(popValsSize) +
                                "\nqueue size: " + to_string(queueSize) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\nrun: " + get_total_tests() + 
                                "\n";
                    
            errorSafe[st_POP].push_back(s);
        } 
        
        void safe_st_empty_pop_error(const bool failedAtFirstCheck, const uint32_t popValsSize, const uint32_t queueSize) {
            const string s =    "failed at: " + to_string(failedAtFirstCheck) +  
                                "\npopVals size: " + to_string(popValsSize) + 
                                "\nqueueSize: " + to_string(queueSize) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\nrun: " + get_total_tests() + 
                                "\n";   
            
            errorSafe[st_EMPTY_POP].push_back(s);
        }


        void safe_mt_push_error(const uint32_t failedAt, const uint32_t queueSize, const uint32_t currThreadAmnt) {
            const string s =    "failed at num: " + to_string(failedAt) + 
                                "\nqueue size: " + to_string(queueSize) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) +
                                "\nrun: " + get_total_tests() +
                                "\n";

            errorSafe[mt_PUSH].push_back(s);
        }

        void safe_mt_pop_error(const uint32_t failedAt, const uint32_t popValsSize, const uint32_t queueSize, const uint32_t currThreadAmnt) {
            const string s =    "failed at num: " + to_string(failedAt) + 
                                "\npopVals size: " + to_string(popValsSize) +
                                "\nqueue size: " + to_string(queueSize) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\nrun: " + get_total_tests() + 
                                "\n";
                    
            errorSafe[mt_POP].push_back(s);
        }

        void safe_mt_push_and_pop_error(const uint32_t failedAt, const uint32_t popValsSize, const uint32_t queueSize, const uint32_t currThreadAmnt) {
            const string s =    "failed at num: " + to_string(failedAt) + 
                                "\npopVals size: " + to_string(popValsSize) + 
                                "\nqueue size: " + to_string(queueSize) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\nrun: " + get_total_tests() + 
                                "\n";
                                
            errorSafe[mt_PUSH_AND_POP].push_back(s);
        }

        void safe_mt_empty_pop_error(const bool failedAtFirstCheck, const uint32_t popValsSize, const uint32_t queueSize, const uint32_t currThreadAmnt) {
            const string s =    "failed at: " + to_string(failedAtFirstCheck) +  
                                "\npopVals size: " + to_string(popValsSize) + 
                                "\nqueueSize: " + to_string(queueSize) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\nrun: " + get_total_tests() +
                                "\n";   

            errorSafe[mt_EMPTY_POP].push_back(s);
        }

        void safe_mt_single_push_and_pop_error(const uint32_t popValsSize, const uint32_t queueSize, const uint32_t currThreadAmnt) {
            const string s =    "popVals size: " + to_string(popValsSize) +
                                "\nqueueSize: " + to_string(queueSize) + 
                                "\ncurrThreads: " + to_string(currThreadAmnt) + 
                                "\nworking: " + to_string(workingTests) + 
                                "\nerrors: " + to_string(errorTests) + 
                                "\nrun: " + get_total_tests() +
                                "\n"; 

            errorSafe[mt_SINGLE_PUSH_AND_POP].push_back(s);
        }
}; 



template<const uint32_t DEFAULT_THREAD_AMNT = 10'000>
class LockFreeQueueTests { 
    private:
        TestState state;
        
        //utils
        const inline uint32_t random_thread_count(const uint32_t min, const uint32_t max) const {
            static random_device rd;
            static mt19937 gen(rd());
            uniform_int_distribution<> dist(min, max);
            return dist(gen);
        }

        inline void push_threads(LockFreeQueue<uint32_t> *q, const uint32_t threadAmnt) {
            vector<thread> v; 

            for(uint32_t i = 0; i < threadAmnt; i++)
                v.push_back(thread(&LockFreeQueue<uint32_t>::push, q, i)); //create push threads with value 'i'
            
            for(auto &t : v) 
                t.join();
        } 

        const inline vector<uint32_t> pop_threads(LockFreeQueue<uint32_t> *q, const uint32_t threadAmnt) {
            vector<thread> v;
            vector<uint32_t> vals(threadAmnt);

            for(uint32_t i = 0; i < threadAmnt; i++) 
                v.push_back(thread(&LockFreeQueue<uint32_t>::pop, q, ref(vals[i]))); //create pop threads that return value to 'vals[i]'
            
            for(auto &t : v) 
                t.join();

            return vals; //returns popped values as vector<int>
        }
 

        //singlethread tests 
        const bool st_test_push(const uint32_t threadAmnt) {
            unique_ptr<LockFreeQueue<uint32_t>> q(new LockFreeQueue<uint32_t>); 
            for(int i = 0; i < threadAmnt; i++) {
                q->push(i); 
            }

            //check if pushed correctly
            bool foundNum;
            Pointer<uint32_t> tmp;
            for(uint32_t num = 0; num < threadAmnt; num++) {
                tmp = q->get_head(); 
                foundNum = false;

                while(tmp.ptr != nullptr) { //go through whole list (threads are async and push's are not sorted)
                    if(tmp.ptr->val == num) {
                        foundNum = true;
                        break;
                    }

                    tmp = tmp.ptr->next; //advance through queue
                }

                if(!foundNum) { //num not found => error in push
                    state.safe_st_push_error(num, q->size());
                    return false;
                }
            }

            //no error => working
            return true;
        }

        const bool st_test_pop(const uint32_t threadAmnt) {
            unique_ptr<LockFreeQueue<uint32_t>> q(new LockFreeQueue<uint32_t>); 
            
            for(int i = 0; i < threadAmnt; i++) {
                q->push(i); 
            }

            vector<uint32_t> v(threadAmnt);
            for(int i = 0; i < threadAmnt; i++) {
                q->pop(v[i]); 
            }

            sort(v.begin(), v.end()); //sort values, so check for value is O(1)

            for(uint32_t num = 0; num < v.size(); num++) {
                if(v[num] == num)
                    continue;
                
                //not all values got popped 
                state.safe_st_pop_error(num, v.size(), q->size()); 
                return false;
            }
            
            //all values correct
            return true; 
        }
        
        const bool st_test_empty_pop(const uint32_t threadAmnt) {
            unique_ptr<LockFreeQueue<uint32_t>> q(new LockFreeQueue<uint32_t>); 

            vector<uint32_t> v(threadAmnt);
            for(int i = 0; i < threadAmnt; i++) {
                q->pop(v[i]); 
            }

            for(uint32_t num = 0; num < v.size(); num++) {
                if(v[num] != 0) { //'v' gets initialized with all '0's and because there are no values in queue to pop, 
                                  //all index's of 'v' should still be value '0'

                    const bool failedAtFirstCheck = true;
                    state.safe_st_empty_pop_error(failedAtFirstCheck, v.size(), q->size());
                    return false;
                }
            }

            const bool working = (q->size() == 0); //check if queue size is '0', if yes => working

            if(!working) {
                const bool failedAtFirstCheck = false;
                state.safe_st_empty_pop_error(failedAtFirstCheck, v.size(), q->size()); 
            }

            return working;
        }


        //multithread tests
        const bool mt_test_push(const uint32_t threadAmnt) {
            unique_ptr<LockFreeQueue<uint32_t>> q(new LockFreeQueue<uint32_t>); 
            push_threads(q.get(), threadAmnt);

            //check if pushed correctly
            bool foundNum;
            Pointer<uint32_t> tmp;
            for(uint32_t num = 0; num < threadAmnt; num++) {
                tmp = q->get_head(); 
                foundNum = false;

                while(tmp.ptr != nullptr) { //go through whole list (threads are async and push's are not sorted)
                    if(tmp.ptr->val == num) {
                        foundNum = true;
                        break;
                    }

                    tmp = tmp.ptr->next; //advance through queue
                }

                if(!foundNum) { //num not found => error in push
                    state.safe_mt_push_error(num, q->size(), threadAmnt);
                    return false;
                }
            }

            //no error => working
            return true;
        }

        const bool mt_test_pop(const uint32_t threadAmnt) {        
            unique_ptr<LockFreeQueue<uint32_t>> q(new LockFreeQueue<uint32_t>); 

            push_threads(q.get(), threadAmnt);
            vector<uint32_t> v = pop_threads(q.get(), threadAmnt);

            sort(v.begin(), v.end()); //sort values, so check for value is O(1)

            for(uint32_t num = 0; num < v.size(); num++) {
                if(v[num] == num)
                    continue;
                
                //not all values got popped 
                state.safe_mt_pop_error(num, v.size(), q->size(), threadAmnt); 
                return false; 
            }
            
            //all values correct
            return true; 
        }

        const bool mt_test_push_and_pop(const uint32_t threadAmnt) {
            unique_ptr<LockFreeQueue<uint32_t>> q(new LockFreeQueue<uint32_t>); 

            //create threads
            vector<thread> pushThrs; 
            vector<thread> popThrs;
            
            for(uint32_t i = 0; i < threadAmnt; i++) 
                pushThrs.push_back(thread(&LockFreeQueue<uint32_t>::push, q.get(), i));

            //only half as many pops as pushes
            vector<uint32_t> popVals(threadAmnt); 
            for(uint32_t i = 0; i <= threadAmnt / 2; i++) 
                popThrs.push_back(thread(&LockFreeQueue<uint32_t>::pop, q.get(), ref(popVals[i])));

            //join
            for(uint32_t i = 0; i < threadAmnt; i++) {
                pushThrs[i].join(); 
                
                if(i <= threadAmnt / 2) {
                    popThrs[i].join(); 
                }
            }

            bool foundNum;
            Pointer<uint32_t> tmp;
            for(uint32_t num = 0; num < threadAmnt; num++) {
                tmp = q->get_head();
                foundNum = false; 

                while(tmp.ptr != nullptr) { //go through whole list (threads are async and push's are not sorted)
                    if(tmp.ptr->val == num) {
                        foundNum = true; 
                        break;
                    }

                    tmp = tmp.ptr->next; //advance queue
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
                        state.safe_mt_push_and_pop_error(num, popVals.size(), q->size(), threadAmnt); 
                        return false;
                    }
                }
            }

            return true; 
        }

        const bool mt_test_empty_pop(const uint32_t threadAmnt) {
            unique_ptr<LockFreeQueue<uint32_t>> q(new LockFreeQueue<uint32_t>); 

            vector<uint32_t> v = pop_threads(q.get(), threadAmnt); //pop without pushing

            for(uint32_t num = 0; num < v.size(); num++) {
                if(v[num] != 0) { //'v' gets initialized with all '0's, and because there are not values in queue to pop, 
                                  //all index's of 'v' should still be value '0'

                    const bool failedAtFirstCheck = true;
                    state.safe_mt_empty_pop_error(failedAtFirstCheck, v.size(), q->size(), threadAmnt);
                    return false;
                }
            }

            const bool working = (q->size() == 0); //check if queue size is '0', if yes => working

            if(!working) {
                const bool failedAtFirstCheck = false;
                state.safe_mt_empty_pop_error(failedAtFirstCheck, v.size(), q->size(), threadAmnt); 
            }

            return working;
        }

        const bool mt_test_single_push_and_pop(const uint32_t threadAmnt) {
            unique_ptr<LockFreeQueue<uint32_t>> q(new LockFreeQueue<uint32_t>); 

            //create threads
            vector<thread> pushThrs; 
            vector<thread> popThrs;

            for(uint32_t i = 0; i < threadAmnt; i++) 
                pushThrs.push_back(thread(&LockFreeQueue<uint32_t>::push, q.get(), i));

            //1:1 push and pop
            vector<uint32_t> popVals(threadAmnt); 
            for(uint32_t i = 0; i < threadAmnt; i++) 
                popThrs.push_back(thread(&LockFreeQueue<uint32_t>::pop, q.get(), ref(popVals[i])));
            
            //join
            for(uint32_t i = 0; i < threadAmnt; i++) {
                pushThrs[i].join();
                popThrs[i].join(); 
            }
            
            //every push is followed by a pop, so size should be '0'
            bool working = (q->size() == 0); 
            if(!working) {
                const bool failedAtFirstCheck = false;
                state.safe_mt_single_push_and_pop_error(popVals.size(), q->size(), threadAmnt); 
            }

            return working;
        }


        //main 
        void tests_loop(uint32_t threadAmnt) {
            state.start_clock();

            while(true) {
                threadAmnt = random_thread_count(5'000, 300'000); //random test-load

                cout << state.default_print(threadAmnt) << endl; 

                //----singlethread----
                state.increase_count(st_test_push(threadAmnt));
                state.increase_count(st_test_pop(threadAmnt));
                state.increase_count(st_test_empty_pop(threadAmnt));
                
                //----multithread----
                state.increase_count(mt_test_push(threadAmnt));
                state.increase_count(mt_test_pop(threadAmnt));
                state.increase_count(mt_test_empty_pop(threadAmnt));
                state.increase_count(mt_test_push_and_pop(threadAmnt));
                state.increase_count(mt_test_single_push_and_pop(threadAmnt));
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
            cout << "----singlethread----" << endl; 
            cout << "push: " << (st_test_push(threadAmnt) ? "working" : "error") << endl;
            cout << "pop: " << (st_test_pop(threadAmnt) ? "working" : "error") << endl;
            cout << "pop on empty queue: " << (st_test_empty_pop(threadAmnt) ? "working" : "error") << endl;

            cout << endl << "----multithread----" << endl; 
            cout << "push: " << (mt_test_push(threadAmnt) ? "working" : "error") << endl;
            cout << "pop: " << (mt_test_pop(threadAmnt) ? "working" : "error") << endl; 
            cout << "pop on empty queue: " << (mt_test_empty_pop(threadAmnt) ? "working" : "error") << endl;
            cout << "concurrent push & pop: " << (mt_test_push_and_pop(threadAmnt) ? "working" : "error") << endl;
            cout << "trade push & pop: " << (mt_test_single_push_and_pop(threadAmnt) ? "working" : "error") << endl;
        }

        void run_tests_loop() {
            thread exitThread(&LockFreeQueueTests::wait_for_exit, this); // reserve 1 thread to guarantee clean exit
            thread testThread(&LockFreeQueueTests::tests_loop, this, DEFAULT_THREAD_AMNT);
            
            testThread.join();
            exitThread.join(); 
        }
};




