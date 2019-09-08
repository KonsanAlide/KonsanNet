# KonsanNet communication framework
* A free network communication framework, written in c++.
* Contain the server and the client.
* Support Linux, Windows platform.
* Support TCP/IP high concurrency communication.
* Using iocp/epoll in the communication kenel module of the server.
* Using a high performance cache module in the server to process data. 
* Support large file block transmission.
* Support RPC object.
* Support multichannel sessions.
* Support data encryption and data compression.


# Version 0.7:
## Description:
   this version had been used in a middle project in my work,I will update it regularly in the future.

## New features:
*  1.Add the sending list.
*  2.Add the compression.
*  3.Add the encryption.
*  4.Add RPC model.
*  5.Add the statistical function on processed time
*  6.Record the message to track and position the questions in running time
   
## Remaining issues:
*    1.add a message forwarding logic in the server to let us to forward some data to third party
*    2.add the timeout process logic in the message processing
   
## Modified stuff: 
### this version modify too many things, show as:  
*  1.change the CXCommunicationClient to a library project
*  2.modify the unpacking logic in the CXConnectionObject
*  3.fix some bugs in the session manager class
*  4.fix some bugs in the connection manager class
*  5.fix the mistake in the CXConnectionObject::SendData, continue it when meet the EAGAIN error .
*  6.fix some bugs in the memory cache manager, and add a elastic class to make the cache scalable.
*  7.fix some bugs in the CXFile64 class.
*  8.modify the CXLog : flush data by the configurable interval time
*  9.fix a bug in the CXThread::Wait() in linux version.
*  10.add RPC model, change the session login process to a rpc object
*  11.change the file operation class to a rpc object
*  12.add a journal log handle to save io records and comsuming times
*  13.add a packet uuid in the packet structure to use to track positioning errors
*  14.fix the grave bug that the order of messages is not maintained in processing.

# Version 0.8:

## New features:
*  1.Add task pool to run the time-consuming task, like: compression, encryption, connect to remote used tcp stream, and so on .
*  2.Add RPC client in the server, and use it to access the the RPC Object in the third-party program
*  3.Add the function used threads to compress and encrypt data, and asynchronously send it.
   
## Remaining issues:
*  1.add the timeout process logic in the message processing
*  2.add the hot outer setting, and dynamically adjust the running parameter of the server
   
## Modified stuff: 
### this version modify too many things, show as:  
*  1.fix the bug that mistakenly use the condition in the CXEvent,
*  2.add a thread cache to reduce the locks

## Notice: 
*  1.after compile this project, you need to unzip the cryptlib.zip in path:'third_libs/i386/windows/debug', and unzip the libcryptopp.zip in the path:'third_libs/x64/linux' 

