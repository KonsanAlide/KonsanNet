cd /d %~dp0build
::cmake -G "Visual Studio 15 Win64" ..

::build win32 x86 project
cmake -G "Visual Studio 15" ..
CXCommunicationFramework.sln