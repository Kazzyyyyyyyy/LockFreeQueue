#include "node.h"
#include <atomic>

using namespace std;

#define TESTING

template<typename T> 
class LockFreeQueue { 

    #ifdef TESTING
        template<uint32_t> friend class LockFreeQueueTests;
    #endif

    private: 
        atomic<Pointer<T>> head, tail;

    public: 
        LockFreeQueue() {
            Node<T> *dummy = new Node<T>();   // create a dummy node so that the LinkedList is not empty 
            head.store(Pointer<T>(dummy, 0));
            tail.store(Pointer<T>(dummy, 0));
        }

        ~LockFreeQueue() {
            Pointer<T> tmp = head.load(); 

            while(tmp.ptr != nullptr) {
                Pointer<T> next = tmp.ptr->next; 
                delete tmp.ptr; 
                tmp = next;
            }
        }

        #ifdef TESTING
            Pointer<T> get_head() const { return head.load(); }
            Pointer<T> get_tail() const { return tail.load(); }
            
            uint32_t size() {
                Pointer<T> currHead = tail.load().ptr->next; //start at '->next' because the dummy doesnt count
                
                uint32_t s = 0; 
                while(currHead.ptr != nullptr) {
                    s++;
                    currHead = currHead.ptr->next.load();
                }

                return s; 
            }
        #endif

        void push(const T &val) {
            Node<T> *newNode = new Node<T>(val); // node we want to push
            Pointer<T> currTail; 

            while(true) { // logic is in a loop to make retries possible, because the success of a push attempt is not guaranteed
                currTail = tail.load(); // load 'tail' at every start of a push attempt
                Pointer<T> nextTail = currTail.ptr->next.load(); // node after 'tail' (should be nullptr)

                if(currTail == tail.load()) { // 'tail' is consistent?
                    if(nextTail.ptr == nullptr) { // 'tail' is actually the last node and 'nextTail' is nullptr?

                        // we use compare_exchange_strong because in case of an unexpected failure we have to repeat the entire loop iteration
                        // therefore we accept the '_strong' overhead for fewer general retries
                        if(currTail.ptr->next.compare_exchange_strong(nextTail, Pointer<T>(newNode, nextTail.count + 1))) { // try to link 'newNode' at the end of the list
                            break; // push successful, break loop
                        }
                    }
                    else {
                        tail.compare_exchange_strong(currTail, Pointer<T>(nextTail.ptr, nextTail.count + 1)); // 'tail' hanging behind, try to advance it
                    }
                }
            }

            tail.compare_exchange_strong(currTail, Pointer<T>(newNode, currTail.count + 1)); // 'newNode' successfully added, advance 'tail' to 'newNode'
        }

        bool pop(T &val) {
            Pointer<T> currHead;

            while(true) {
                currHead = head.load(); // load 'head' at every start of a pop attempt
                Pointer<T> nextHead = currHead.ptr->next.load(); // node after 'head'
                Pointer<T> currTail = tail.load();  

                if(currHead == head.load()) { // 'head' is consistent? 
                    if(currHead.ptr == currTail.ptr) { // queue is empty or 'tail' is lagging?
                        if(nextHead.ptr == nullptr) { // queue is empty?
                            return false; // queue empty, nothing to pop
                        }
                        
                        // 'nextHead.ptr' gets used to advance 'tail' because 'tail' and 'head' are pointing to the same node
                        tail.compare_exchange_strong(currTail, Pointer<T>(nextHead.ptr, currTail.count + 1)); // queue not empty, 'tail' just hanging behind, advance it
                    }
                    else {
                        val = nextHead.ptr->val; // give data to '&val'
                        if(head.compare_exchange_strong(currHead, Pointer<T>(nextHead.ptr, currHead.count + 1))) { // advance 'head' to 'head->next'
                            break; // pop finished, break
                        }
                    }
                }
            }

            delete currHead.ptr; // safe now to free old head
            return true; // pop successful
        }
}; 
