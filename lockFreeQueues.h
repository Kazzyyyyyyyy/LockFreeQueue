#include <iostream>
#include <memory>
#include <atomic>
#include <mutex>

using namespace std; 

template<typename T>
class LockFreeQueueLinkedList {
    private: 
        struct Node {
            const T data;
            atomic<Node*> next; 

            Node(const T& val) : data(val), next(nullptr) {} 
        }; 

        atomic<Node*> head, tail; 
        Node *dummy; 

    public: 
        LockFreeQueueLinkedList() {
            dummy = new Node(T{}); 
            tail.store(dummy); // so that head & tail != NULL
            head.store(dummy); 
        }

        ~LockFreeQueueLinkedList() {
            while(dummy != nullptr) {
                Node *next = dummy->next; 
                delete dummy;
                dummy = next;
            }
        }

        void push(const T &val) {
            Node *newNode = new Node(val), //node we want to add
                 *currTail;

            while(true) {
                currTail = tail.load(); 
                Node *currTailNext = currTail->next; //need this because compare_exchange_weak doesnt like L-Values

                if(currTailNext != nullptr) { //not actual last node
                    tail.compare_exchange_weak(currTail, currTailNext); //advance through list to get to last node (tail)
                    continue;
                }
                
                if(currTail->next.compare_exchange_weak(currTailNext, newNode)) //set currTail->next = newNode
                    break;                                                      //to make the queue a linked list
            }

            //set tail = newNode: push finished, node added
            tail.compare_exchange_weak(currTail, newNode);
        }

        bool pop(T &val) {
            Node *currHead = head.load(),
                 *nextHead = currHead->next;

            while(true) {
                if(currHead == tail.load())
                    return false; //queue empty, nothing to pop
             
                if(currHead != head.load()) { //head hanging behind/lagging, advance though list
                    head.compare_exchange_weak(currHead, nextHead);
                    continue; 
                }
                
                //head found, pop and return data
                val = nextHead->data;
                head.compare_exchange_weak(currHead, nextHead);
                break;
            }

            return true;
        }
};


template<typename T, size_t startCapacity = 1000> 
class LockFreeQueueArray {
    private:
        T *arr = new T[startCapacity];
        size_t capacity = startCapacity; 
        atomic<size_t> head, tail;
        mutex mtx; 

        ///BUG
        void resize_arr(const size_t size) {
            capacity = size; 
            T *newArr = new T[capacity];

            for(int i = head; i < tail; i++) 
                newArr[i - head] = arr[i];

            delete[] arr; 

            tail.store(tail - head); 
            head.store(0);

            // cout << "tail: " << tail.load() << endl << "head: " << head.load() << endl;
 
            arr = newArr;
        }
        ///BUG

    public: 
        LockFreeQueueArray() {
            head.store(0); 
            tail.store(0); 
        }

        ~LockFreeQueueArray() {
            delete[] arr;
        }
 
        void push(const T &val) {
            ///BUG
            if(tail.load() >= capacity) {
                mtx.lock(); 
                resize_arr(capacity * 2); 
                mtx.unlock();
            }
            ///BUG

            size_t currTail, nextTail;

            while(true) {
                currTail = tail.load();
                nextTail = currTail + 1;

                if(tail.compare_exchange_weak(currTail, nextTail)) {
                    arr[currTail] = val;
                    break;
                }
            }
        }

        bool pop(T &val) {
            size_t currHead, nextHead;
            
            while(true) {
                currHead = head.load(); 
                nextHead = currHead + 1; 

                if(currHead == tail.load())
                    return false; 

                if(head.compare_exchange_weak(currHead, nextHead)) {
                    val = arr[currHead];
                    return true;
                }
            }
        }

        void test(int size) {
            // for(int i = 0; i < size; i++) 
            //     cout << arr[i] << endl;

            int f = 0;
            bool found = false;  
            while(true) {        
                for(int i = 0; i < size; i++) {
                    if(arr[i] == f) {
                        f++; 
                        cout << "found: " << f << endl; 
                        found = true; 
                        break; 
                    }
                }

                if(f == size - 1) {
                    cout << "found all" << endl;
                    break; 
                }
                else if(!found) {
                    cout << endl << "error run: " << endl
                         << "head: " << head.load() << endl  
                         << "tail: " << tail.load() << endl 
                         << "capacity: " << capacity << endl;
                         
                    for(int i = 0; i < size; i++) {
                        if(arr[i] == f + 1) {
                            f++; 
                            cout << "f + 1 found: " << f + 1 << endl; 
                            found = true; 
                            return; 
                        }

                        //3885, 8165, 8107 => resize bug
                    }

                    cout << "f + 1 not found :(" << endl; 
                    return;
                }

                found = false;
            } 

            cout << "end" << endl;

            
            // cout << tail.load() << endl; 
        }
}; 