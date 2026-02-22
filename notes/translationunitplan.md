```cpp
struct Instance
{
    Bytecode code;
    std::vector<Frame> callStack;
};

struct Frame
{ 
    std::vector<Value> stack; 
    std::size_t pc; 
}; 

using InstanceHandle = std::size_t;

struct Runtime {
    std::vector<std::unique_ptr<Instance>> instances;
};
```

* vector owns instances 
* unique_ptr ensures deletion when removed 

call_trans opcode

1. resolve target handle 
2. copy arguments into target instances current frame stack 
3. push return context onto target callStack 
4. set target pc to entry 
5. switch current instance (runtime keeps current index)

On return: 
1. pop frame 
2. copy return value to caller stack 
3. restore caller instance as current

call_extern opcode 
1. resolve path 
2. check cache (timestamp) 
3. load bytecode if missing or stale 
4. create or reuse instance for that module 
5. resolve function pc 
6. copy arguments 
7. push return context 
8. execute 

Cache can map path to instance handle. If timestamp changes, drop old instance and reload. Inside runtime code we can use raw pointers or references after resolving handles:

```cpp
Instance* inst = runtime.instances[handle].get();
```

Exceptions will be replaced with a new runtime system
