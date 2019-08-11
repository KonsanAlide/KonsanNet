# CXCommunicationFramework
A free network communication framework, written in c++.
Contain the server and the client.
Support Linux, Windows platform.
Support TCP/IP high concurrency communication.
Using iocp/epoll in the communication kenel module of the server.
Using a high performance cache module in the server to process data. 
Support large file block transmission.
Support RPC object.
Support multichannel sessions.
Support data encryption and data compression.


version 0.7:
description:
   this version had been used in a middle project in my work,I will update it regularly in the future.
Remaining issues:
   1.add a send list to the server
   2.add a message forwarding logic in the server to let us to forward some data to third party
   3.add the timeout process logic in the message processing
   4.count the processing time of a message packet, record the slow operations
   
modified stuff:
this version modify too many things, show as:
1.change the CXCommunicationClient to a library project
2.modify the unpacking logic in the CXConnectionObject
3.fix some bugs in the session manager class
4.fix some bugs in the connection manager class
5.fix the mistake in the CXConnectionObject::SendData, continue it when meet the EAGAIN error .
6.fix some bugs in the memory cache manager, and add a elastic class to make the cache scalable.
7.fix some bugs in the CXFile64 class.
8.modify the CXLog : flush data by the configurable interval time
9.fix a bug in the CXThread::Wait() in linux version.
10.add RPC model, change the session login process to a rpc object
11.change the file operation class to a rpc object
12.add a journal log handle to save io records and comsuming times
13.add a packet uuid in the packet structure to use to track positioning errors

