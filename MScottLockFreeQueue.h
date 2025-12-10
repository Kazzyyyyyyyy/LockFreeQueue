#include "node.h"
#include <atomic>

using namespace std;

template<typename T> 
class LockFreeQueue {
    private: 
        atomic<Pointer<T>> head, tail;

    public: 
        // we use compare_exchange_strong because in case of an spurious failure we need to go through everything in the loop again
        // for push that would be 5-10 and for pop 10-16 lines of code, so we accept the _strong overhead for lesser generell retrys  

        LockFreeQueue() {
            Node<T> *dummy = new Node<T>();     // we create the dummy node, so that the LinkedList is not empty 
            head.store(Pointer<T>(dummy, 0));   // because that makes deletion, and addition way easier
            tail.store(Pointer<T>(dummy, 0));
        }

        void push(const T &val) {
            Node<T> *newNode = new Node<T>(val); // node we want to push
            Pointer<T> currTail; 

            while(true) { // the logic is in a loop to make retrys possible, because it is not guaranteed, that a push attempt succeeds
                currTail = tail.load(); // load 'tail' at every start of a push attempt
                Pointer<T> nextTail = currTail.ptr->next; // Node after 'tail' (should be empty:nullptr)

                if(currTail == tail.load()) { // 'tail' is consistent?
                    if(nextTail.ptr == nullptr) { // 'tail' is actually the last node and 'nextTail' is empty:nullptr?
                        if(currTail.ptr->next.compare_exchange_strong(nextTail, Pointer<T>(newNode, nextTail.count + 1))) { //try to link node at the end of the list
                            break; //push successful, break loop
                        }
                    }
                    else {
                        tail.compare_exchange_strong(currTail, Pointer<T>(nextTail.ptr, nextTail.count + 1)); //tail hanging behind, try to advance it and catch up
                    }
                }
            }

            tail.compare_exchange_strong(currTail, Pointer<T>(newNode, currTail.count + 1)); //newNode successfully added, advance tail to newNode
        }


        bool pop(T &val) {
            Pointer<T> currHead;

            while(true) {
                currHead = head.load(); // load 'head' at every start of a pop attempt
                Pointer<T> nextHead = currHead.ptr->next; // Node after head
                Pointer<T> currTail = tail.load();  

                if(currHead == head.load()) { // 'head' is consistent? 
                    if(currHead.ptr == currTail.ptr) { // queue is empty or 'tail' is lagging?
                        if(nextHead.ptr == nullptr) { // queue is empty?
                            return false; // queue empty, nothing to pop
                        }

                        tail.compare_exchange_strong(currTail, Pointer<T>(nextHead.ptr, currTail.count + 1)); // queue not empty, tail just hanging behind, advance it
                        // 'nextHead.ptr' gets used to advance 'tail' because 'tail' is hanging behind and 'tail'/'head' are pointing to the same node
                        // we could also use 'currTail.ptr->next' but why should we when 'nextHead.ptr' is already in memory
                    }
                    else {
                        val = nextHead.ptr->val; // give value to &val
                        if(head.compare_exchange_strong(currHead, Pointer<T>(nextHead.ptr, currHead.count + 1))) { // advance currHead to head->next
                            break; // pop finished, break
                        }
                    }
                }
            }

            delete currHead.ptr; //safe now to free old head
            return true; //pop successful
        }
}; 
