Implement lock-free SPSC (Single Producer Single Consumer) Queue.

For more implemented details explanation, refer to https://www.cnblogs.com/sinkinben/p/17949761/spsc-queue

**Usage**

Define a `spsc_queue`:
```cpp
#include "spsc_queue.hpp"
spsc_queue<int> sq;
```

In consumer thread:
```cpp
int data = -1;
while (!sq.dequeue(data))
    ;
std::cout << data << '\n';
```

In producer thread:
```cpp
int data = 233;
while (!sq.enqueue(data))
    ;
// now 'data' has been sent to consumer
```


**Tests And Example**

See `benchmark.cpp`, and just type `make all` to compile and run it