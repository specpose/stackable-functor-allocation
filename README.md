# Stackable Functor Allocation
Is a refactoring pattern. It aims to keep the Functor operator() allocation free and the Functor body free of references. The strict memory is adressed entirely as a *stack*, as heap (size known at compile time), but not as free-store.
## [SFA](test/sfa.cpp)
The refactoring pattern.
## [MOLE](test/mole.cpp)
A related GPU refactoring pattern for invertible functions. Here, the term Stack is not referring to a memory arrangement.
## [MOLE](test/invertable.cpp)
Defunct.
### Installation
CPU Mode
```sh
cmake -B build . -DSFA_BACKEND=CPU
cmake --build build
```
Goopax Mode: Download and extract Goopax to the directory above the project directory.
```sh
cmake -B build . -DSFA_BACKEND=GOOPAX
cmake --build build
```
SyCL Mode: Set a SyCL compatible clang compiler in your path.
```sh
cmake -B build . -DCMAKE_C_COMPILER=icx -DCMAKE_CXX_COMPILER=icpx -DSFA_BACKEND=INTELSYCL -GNinja
cmake --build build
```
