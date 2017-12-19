**1. Install cmake**

macos
-----
brew install cmake
or
sudo port install cmake

ubuntu/debian|linux
-------------------
sudo apt install cmake

windows
-------
1. Download https://cmake.org/files/v3.7/cmake-3.7.2-win32-x86.msi
2. install it with option add to the system path checked
3. Download and install https://www.microsoft.com/en-in/download/confirmation.aspx?id=5555


**2. Installing boost**

macos
-----
brew install boost
or
sudo port install boost

ubuntu/debian|linux
-------------------
sudo apt install libboost-all-dev

windows
-------
1. download relevent binary from https://sourceforge.net/projects/boost/files/boost-binaries/1.63.0/
2. for visual studio 2015 download boost_1_63_0-msvc-14.0-32.exe
3. run the binary as administrator
4. install it to **C:\Program Files (x86)\boost** - It is very important that it is the exact path; boost folder name should not contain any version number.


**3. Build LiSpScPaRo**

1. extract the LiSpScPaRo.7z to a folder called workspace
2. cd to workspace
3. make directory buildLiSpScPaRo
4. cd to buildLiSpScPaRo

macos
-----
5. cmake ../LiSpScPaRo -G Xcode
6. xcodebuild
7. cd Debug
8. ./LiSpScPaRo

linux
-----
5. cmake ../LiSpScPaRo
6. make
8. ./LiSpScPaRo

windows
-------
5. cmake ..\LiSpScPaRo -G "Visual Studio 14 2015"
6. add "C:\Program Files (x86)\MSBuild\14.0\Bin" to the path
7. msbuild LiSpScPaRo.sln
8. cd Debug
9. LiSpScPaRo.exe
