# LockFreeQueue
Michael/Scott LockFreeQueue for x86 TSO

## What Ive learned 
 - atomic / CAS operations and differences between them
 - structure and principle of LockFree data-structures
 - the ABA problem and how to solve it via tagged pointers (counter)
 - memory order (basics)

## State of the project 
The project is done (for now: 16.12.25). The current Version of the queue, is made only for x86 Architecture (because it needs TSO) and will NOT run on ARM. 
Later I will probably add a Version that is also viable for ARM, but idk when Im gonna do this.

## Notes 
The selfwritten Tests, test basic functions and simple edge cases with randomized load and duration.
 
**compilation:** 
```
# normal
g++ MS_LockFreeQueue.h node.h pointer.h -o XX -latomic

# tests (add '#define TESTING' in 'MS_LockFreeQueue.h')
g++ MS_LockFreeQueue.h tests/MS_LockFreeQueue_Test.h node.h pointer.h -o XX -latomic
```