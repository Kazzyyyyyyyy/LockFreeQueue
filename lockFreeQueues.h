#include <iostream>
#include <memory>
#include <atomic>

using namespace std; 
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
        LockFreeQueue() {
            dummy = new Node(T{}); 
            tail.store(dummy); // so that head & tail != NULL
            head.store(dummy); 
        }

        ~LockFreeQueue() {
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


template<typename T> 
class LockFreeQueueArray {
    private: 
        T *arr = new T[100];
        atomic<size_t> head, tail;

    public: 
        LockFreeQueue() {
            head.store(0); 
            tail.store(0); 
        }

        ~LockFreeQueue() {
            delete[] arr;
        }
 
        void push(const T &val) {
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

        void test() {
            cout << "head: " << head.load() << "\ntail: " << tail.load() << endl;
            for(int i = head.load(); i < tail.load(); i++) 
                cout << arr[i] << endl;

            cout << endl; 
        }
}; 