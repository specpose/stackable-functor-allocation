# Stackable Functor Allocation
Is a refactoring pattern. It aims to keep the Functor operator() allocation free and the Functor body free of references. The strict memory is adressed entirely as a *stack*, as heap (size known at compile time), but not as free-store.
## Algorithm LifeCycle
1. C++20/STL
   - Effectively is re-introducing a storage-like class into algorithms which have been originally designed to abstract away the storage class. Keep in mind that a GPU is always a rigid hardware implementation and the scope of the API is not a response to future challenges.
   - The sized_sentinel_for concept of the ranges API presents another approach to address the same problem. The ranges call syntax is similar to Java and is a radical change in programming paradigm.
2. C++17/SFA
   - SFA is a refactoring pattern which also has a Java-like call syntax. Applying stackable functor allocation may lead to code which is compatible to ranges, but ranges has a few abstractions which make it difficult to write low-level code.
   - It is object-oriented. It helps converting STL programs into something that ressembles a C program.
