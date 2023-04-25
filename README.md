# CTNR
Compile Time Name Reflector

A single header-source file for typename reflection at compile time for C++11.

## Supported compilers
- g++
- clang++
- Microsoft Visual Studio

## Notes
Compiling under MSVC results in different behaviour compared to the others.
```c++
CTNR::GetName<classy>();
CTNR::GetName<structy<classy>>();
```
will evaluate to the following</br>
g++ & clang++
```
"classy"
"structy<classy>"
```
MSVC
```
"class classy"
"struct structy<class classy>"
```
