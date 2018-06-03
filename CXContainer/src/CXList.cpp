/****************************************************************************
Copyright (c) 2018 Charles Yang

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description��This class manage a list of some large memory blocks,
a large memory block is a continuous memory block,
the large memory block will been divided into many small structures,
the size of the structure is able to custom ,etc 4kb,8kb,1MB,and so on.
a structure will been used to save a user's buffer.
This class have a spinlock to solve the problem of thread synchronization.
*****************************************************************************/
#include "CXList.h"
#ifndef WIN32
#include <string.h>
#endif

using namespace CXCommunication;
CXList::CXList()
{
}

CXList::~CXList()
{
    
}

