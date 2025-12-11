#include <atomic>
#include "pointer.h"

template<typename T> 
class Node {
    public: 
        T val; 
        atomic<Pointer<T>> next;

        Node() {
            next.store(Pointer<T>(nullptr, 0));
        }

        Node(const T& v) {
            val = v; 
            next.store(Pointer<T>(nullptr, 0)); 
        }
}; 