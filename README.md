# repeat
Simple command-line utility that repeats the given string a set amount of times. Compiles for Linux and Windows.

# How To Build

## Prerequisites
```
git (not just for cloning, makefile uses git to clean the repo as well)
make (only on Linux)
bash (only on Linux)
cmd/powershell (only on Windows)
clang++-11 or newer (only on Linux)
MSVC 19.29 or newer (only on Windows)
```

## Guide For Linux
After cloning the repo (we don't have any submodules, you don't have to recursively clone if you don't want to), go into it and run ```make```.
By default, this will try to use clang++-15, but if you have another version that supports C++20, you can do the following to use that version:
```
make CLANG_PROGRAM_NAME:=x
# Where x is another version of clang++.
```
There are also some other variables in the makefile that you can play around with, just open it up and read them, the file isn't complicated at all.
Also, by default, make will build an optimized build. If you want an unoptimized build (you can also do this by setting the corresponding variable by hand),
simply run ```make unoptimized```.
You can also clean the repo with ```make clean```, which will remove all files that aren't tracked by git, EXCEPT editor swap files, in case you've got an open vim session or something.
If you want to clean everything, INCLUDING the swap files, just run ```make clean_include_swaps```.

After building, the finished executable will be located in the bin folder, which will be created if there isn't one.

## Guide For Windows
After cloning the repo (we don't have any submodules, you don't have to recursively clone if you don't want to), go into it and run ```./build_on_windows.bat```.
This will build the program with MSVC with optimizations and dump the finished executable into the current folder (which should be the top-level folder in the repo).
It DOES NOT put the finished executable in the bin folder.
To be able to run the batch script, you have to run it from within an instance of a Visual Studio developer command prompt, in order to have access to cl.exe.
If you can get that program into your PATH and have it somehow work, then you can use the batch script from anywhere, but the dev cmd prompt is the easiest way.

# How To Install
There is nothing to install. The finished binary has no dependencies, just copy it to where ever you want it and start using it.

# How To Use
Very very simple, just type ```repeat --help``` and you'll get an explanation.
