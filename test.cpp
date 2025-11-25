#include <iostream> 
#include "lockFreeQueues.h"
#include <vector>
#include <thread>
#include <atomic>
#include <algorithm>

using namespace std; 

template<const size_t threadAmnt = 10000>
class LockFreeQueueLinkedListTest { 
    private:
        struct Node {
            const int data;
            atomic<Node*> next; 

            Node(const int& val) : data(val), next(nullptr) {}
        }; 

        void push_threads(LockFreeQueueLinkedList<int> *q, const size_t size) {
            vector<thread> v; 

            //push
            for(int i = 0; i < size; i++) {
                v.push_back(thread(&LockFreeQueueLinkedList<int>::push, q, i));
            }

            for(auto &t : v) {
                t.join(); 
            }
        } 
        
        vector<int> pop_threads(LockFreeQueueLinkedList<int> *q, const size_t size) {
            vector<thread> v;
            vector<int> vals(size); 

            for(int i = 0; i < size; i++) {
                v.push_back(thread(&LockFreeQueueLinkedList<int>::pop, q, ref(vals[i])));
            }

            for(auto &t : v) {
                t.join();
            }

            return vals;
        }

        bool test_push() {
            const size_t size = threadAmnt; 
            auto *q = new LockFreeQueueLinkedList<int>; 
            push_threads(q, size);

            //check 
            int f = 0; //number we are currently looking for
            bool foundf = false; //number found this round?
            Node *tmp;

            for(int i = 0; i < size; i++) {
                tmp = (Node*)q->get_head();
                foundf = false;

                while(tmp != nullptr) { //go through whole list (threads are async and push's are not sorted)
                    if(tmp->data == f) {
                        foundf = true;
                        f++; //increase number for next round
                        break;
                    }

                    tmp = tmp->next; //advance queue
                }

                if(!foundf) {
                    delete q; 
                    return false; //f not found => error in push
                }
            }

            delete q; 
            return true; //no error => test success
        }

        bool test_pop() {
            const size_t size = threadAmnt; 
            auto *q = new LockFreeQueueLinkedList<int>; 

            push_threads(q, size); 
            vector<int> v = pop_threads(q, size);

            sort(v.begin(), v.end()); 

            // cout << endl << "v.begin(): " << v[0] << "  - v.end(): " << v[v.size() - 1];
            int f = 0; 
            for(int val : v) {
                if(val != f) {
                    cout << endl << "failed at: " << f << endl;
                    for(int val2 : v) {
                        if(val2 = f + 1)  {
                            cout << "found f + 1: " << f + 1 << endl;
                            break;
                        }
                    }

                    delete q; 
                    return false;
                }

                f++;
            }
            
            delete q; 
            return true; 
        }

        bool test_push_and_pop() {
            auto *q = new LockFreeQueueLinkedList<int>; 
            const size_t size = threadAmnt; 

            vector<thread> pushThrs; 
            vector<thread> popThrs;
            
            for(int i = 0; i < size; i++) {
                pushThrs.push_back(thread(&LockFreeQueueLinkedList<int>::push, q, i));
            }

            vector<int> popVals(size); 
            for(int i = 0; i <= size / 2; i++) {
                popThrs.push_back(thread(&LockFreeQueueLinkedList<int>::pop, q, ref(popVals[i])));
            }

            for(int i = 0; i < size; i++) {
                pushThrs[i].join(); 
                
                if(i <= size / 2) {
                    popThrs[i].join(); 
                }
            }

            int f = 0;
            bool foundf = false;
            Node *tmp;
            for(int i = 0; i < size; i++) {
                tmp = (Node*)q->get_head();
                foundf = false;

                while(tmp != nullptr) { //go through whole list (threads are async and push's are not sorted)
                    if(tmp->data == f) {
                        foundf = true;
                        // cout << "tmp: " << f << endl;
                        f++; //increase number for next round
                        break;
                    }

                    tmp = tmp->next; //advance queue
                }

                if(!foundf) {
                    for(int i = 0; i < popVals.size(); i++) {
                        if(popVals[i] == f) {
                            foundf = true; 
                            // cout << "popVals: " << f << endl;
                            f++; 
                            break; 
                        }
                    }

                    if(!foundf) {
                        delete q; 
                        return false; //f not found => error in push
                    }
                }
            }

            delete q; 
            return true; 
        }


    public: 
        void test() {
            cout << "push: " << (test_push() ? "working" : "error") << endl;
            cout << "pop: " << (test_pop() ? "working" : "error") << endl; 
            cout << "concurrent push & pop: " << (test_push_and_pop() ? "working" : "error") << endl;
        }

}; 