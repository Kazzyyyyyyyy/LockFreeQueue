#include <iostream>
#include <memory>
#include <atomic>

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
                
                //head found, pop and set data
                val = nextHead->data;
                head.compare_exchange_weak(currHead, nextHead);
                return true;
            }
        }

        void test(int size) {
            Node *tmp; 
            int f = 0;
            bool found = false;  
            while(true) {        
                tmp = head.load(); 
                while(tmp != nullptr) {
                    if(tmp->data == f) {
                        f++; 
                        cout << "found: " << f << endl; 
                        found = true; 
                        break; 
                    }

                    tmp = tmp->next; 
                }

                if(f == size - 1) {
                    cout << "found all" << endl;
                    break; 
                }
                else if(!found) {
                    tmp = head.load(); 
                    cout << endl << "error run: " << endl; 

                    while(tmp != nullptr) {
                        if(tmp->data == f + 1) {
                            f++; 
                            cout << "f + 1 found: " << f + 1 << endl; 
                            found = true; 
                            return; 
                        }

                        tmp = tmp->next; 
                        //3885, 8165, 8107 => resize bug
                    }

                    cout << "f + 1 not found :(" << endl; 
                    return;
                }

                found = false;
            }
        }
};


template<typename T, size_t startCapacity = 1000> 
class LockFreeQueueArray {
    private:
        T *arr = new T[startCapacity];
        size_t capacity = startCapacity; 
        atomic<size_t> head, tail;
        bool resizing = false; 

        //create new array with new size, copy old array, delete old array, set old = new
        void resize_arr(const size_t size) {
            capacity = size; 
            T *newArr = new T[capacity];

            for(int i = head; i < tail; i++) 
                newArr[i - head] = arr[i];

            delete[] arr; 

            tail.store(tail - head); 
            head.store(0);

            arr = newArr;
        }

    public: 
        LockFreeQueueArray() {
            head.store(0); 
            tail.store(0); 
        }

        ~LockFreeQueueArray() {
            delete[] arr;
        }
 
        void push(const T &val) {
            //to be honest, im not sure if this is thread safe, because, if multiple threads are exactly head to head
            //i think all could go through the if, before resizing = false... => will test it (somehow -_-)
            if(tail.load() >= capacity && !resizing) { //!resizing gets checked so only one thread at a time does the resizing
                resizing = true; //stop other threads from changing array while rearraynging (good one huh?) it
                resize_arr(capacity * 2);
                resizing = false;
            }

            size_t currTail, nextTail;

            while(true) {
                currTail = tail.load();
                nextTail = currTail + 1;
 
                //push val if not currently resizing array
                if(!resizing && tail.compare_exchange_weak(currTail, nextTail)) {
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

                //queue empty, nothing to pop
                if(currHead == tail.load())
                    return false; 

                //pop if not currently resizing array, set &val = value and finish (return)
                if(!resizing && head.compare_exchange_weak(currHead, nextHead)) {
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