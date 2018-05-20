Rock-Paper-Scissors-Spock-Lizard
================================
![Travi-CI](https://travis-ci.com/necktwi/LiSpScPaRo.svg?branch=master)

LICENSE
-------
The MIT License (MIT)

Copyright (c) 2016 Gowtham Kudupudi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

Installation
------------
### Install cmake
#### macOS
brew install cmake
or
sudo port install cmake
#### Ubuntu/Debian|Linux
sudo apt install cmake
#### Windows
1. Download https://cmake.org/files/v3.7/cmake-3.7.2-win32-x86.msi
2. install it with option add to the system path checked
3. Download and install https://www.microsoft.com/en-in/download/confirmation.aspx?id=5555
### Installing boost
#### macOS
brew install boost
or
sudo port install boost
#### Ubuntu/Debian|Linux
sudo apt install libboost-all-dev
#### Windows
1. download relevent binary from https://sourceforge.net/projects/boost/files/boost-binaries/1.63.0/
2. for visual studio 2015 download boost_1_63_0-msvc-14.0-32.exe
3. run the binary as administrator
4. install it to **C:\Program Files (x86)\boost** - It is very important that it is the exact path; boost folder name should not contain any version number.

Build LiSpScPaRo
----------------
1. extract the LiSpScPaRo.7z to a folder called workspace
2. cd to workspace
3. make directory buildLiSpScPaRo
4. cd to buildLiSpScPaRo
### macOS
5. cmake ../LiSpScPaRo -G Xcode
6. xcodebuild
7. cd Debug
8. ./LiSpScPaRo
### Linux
5. cmake ../LiSpScPaRo
6. make
8. ./LiSpScPaRo
### Windows
5. cmake ..\LiSpScPaRo -G "Visual Studio 14 2015"
6. add "C:\Program Files (x86)\MSBuild\14.0\Bin" to the path
7. msbuild LiSpScPaRo.sln
8. cd Debug
9. LiSpScPaRo.exe
