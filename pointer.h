#include <cstdint>

using namespace std; 

template<typename T> 
class Node; 

template<typename T> 
class Pointer {
    public: 
        Node<T> *ptr;
        uint64_t count; 

        Pointer() = default; // use default constructor

        Pointer(Node<T> *newPtr, const uint64_t newCount) {
            ptr = newPtr; 
            count = newCount; 
        }

        bool operator==(const Pointer<T> &other) {
            return ptr == other.ptr && count == other.count; 
        }

}; 