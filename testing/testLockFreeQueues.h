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
    
        ///testing 
        size_t returnPops = 0; 

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
            Node *newNode = new Node(val), 
                 *currTail; 

            while(true) {
                currTail = tail.load(); 
                Node *nextTail = currTail->next; //compare_exchange_weak doesnt like L values

                if(currTail != tail.load() || nextTail != nullptr)
                    continue; 

                if(currTail->next.compare_exchange_weak(nextTail, newNode)) { //set currTail->next = newNode
                    tail.compare_exchange_weak(currTail, newNode); //advance tail to newNode
                    return; 
                }
            }
        }

        bool pop(T &val) {
            Node *currHead,
                 *nextHead;

            while(true) {
                currHead = head.load(); 
                nextHead = currHead->next; 
                
                if(currHead == tail.load() || nextHead == nullptr) { //queue empty 
                    returnPops++; 
                    return false; //for test single bla bla, this can cause an "error" even though its normal behaviour and absolutely ok
                }

                if(currHead == head.load()) {
                    if(head.compare_exchange_weak(currHead, nextHead)) {
                        val = nextHead->data; //currHead is the dummy => holds value T{}
                        return true; 
                    }
                }
            }
        }

        ///testing 
        size_t size() {
            Node *currHead = head.load(); 

            size_t num = 0; 
            while(currHead->next != nullptr) {
                currHead = currHead->next; 
                num++; 
            }

            return num; 
        }

        size_t get_return_pops() {
            return returnPops;
        }

        Node *get_head() {
            return head.load(); 
        }

        Node *get_tail() {
            return tail.load(); 
        }
};


template<typename T, size_t startCapacity = 10000>
class LockFreeQueueArray {
    private:
        T *arr = new T[startCapacity];
        size_t capacity = startCapacity; 
        atomic<size_t> head, tail;
        bool resizing = false; 

        ///testing 
        uint8_t resizeCount = 0; 
        size_t returnPops = 0; 

        //create new array with new size, copy old array, delete old array, set old = new
        void resize_arr(const size_t newSize) {
            capacity = newSize; 
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
                resizing = true; //stop other threads from changing array while rearraynging (good one huh?)
                resize_arr(capacity * 2);
                resizing = false;
        
                ///testing 
                resizeCount++;
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
                if(currHead == tail.load()) {
                    returnPops++; 
                    return false; 
                }

                //pop if not currently resizing array, set &val = value and finish (return)
                if(!resizing && head.compare_exchange_weak(currHead, nextHead)) {
                    val = arr[currHead];
                    return true;
                }
            }
        }


        ///testing 
        int *get_array() {
            return arr; 
        }

        size_t get_head() {
            return head.load(); 
        }
        
        size_t get_tail() {
            return tail.load(); 
        }

        size_t get_capacity() {
            return capacity; 
        }
        
        uint8_t get_resizes() {
            return resizeCount; 
        }

        size_t get_return_pops() {
            return returnPops;
        }
}; 