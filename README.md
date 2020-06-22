# Distributed-Hashtable

We develop a simplified version of DHT inspired by Dynamo from Amazon. Such a system can be seen as a version of Java Maps which would be made robust by its redundant distribution on several computers (instead of staying locally in the memory of just one). Such a system offers, of course, a simple user interface, very similar to an associative table, but behind it distributes the actual data storage on multiple servers. On the one hand, this increases the availability of data by the causes multiple machines to have copies of these (in case one or the other machine would be unavailable), and on the other hand the total storage capacity (in adding the capacities of all the machines involved).

## Usage

### Compile & run multiple servers

Run ``` make ``` to compile using the Makefile provided.

Servers list:
- Open the text file named ```servers.txt``` that will contains all IP/PORT of the servers
- Fill the text file using the following syntax: ```IP PORT #OfNodes```

To start _multiple servers_, do the following steps, run ```./pps-launch-server < server.txt```

To start a _single server_, run ```./pps-launch-server``` in a terminal (you can add a ``` & ``` at the end of the command in detach mode and use other commands in the same terminal). Then insert your IP and the port you want your server to run at (following ```IP PORT```).

 If you want to run the server localy, use ```127.0.0.1``` as IP.

### Commands

We use the notation as follows:
- [ X ] implies X is optional
- \<Y\> implies Y is compulsory
- ... implies a variable number of arguments
- \[--\] is used to differentiate the optional arguments and the compulsory ones (as "-n" could be a key in our hash table)

You can run the following commands in a terminal to interact with the Hash Table:

- Put an element (key, value) in the table :  
 ```./pps-client-put [-n N] [-w W] [--] <key> <value>```
- Get a value from the table :  
 ```./pps-client-get [-n N] [-r R] [--] <key>```
- List all the nodes and know which one is up and which one is down :  
 ```./pps-list-nodes```
- Dump the content of a given node :   
```./pps-dump-node <IP> <Port>```
- Concatenate values from multiple keys and store the result with a new key :  
 ```./pps-client-cat [-n N] [-w W] [-r R] [--] <input-key1> <input-key2> ... <output-key>```
- Get a substring of a value and store it with a new key :   
 ```./pps-client-substr [-n N] [-w W] [-r R] [--] <input-key> <position> <length> <output-key>```
- Find the index of a matching substring in another key-value pair :  
 ```./pps-client-find [-n N] [-w W] [-r R] [--] <key-to-search> <key-to-search-for>```

Note : for the system to work correctly, you might need to adjust the N, R, W and S values:
- N: maximum number of servers that store a particular key; this is also the maximum number of reads / writes performed for a value given (see R and W).
- R: ("read") number of functional servers required to retrieve a value from the network; read operation attempts to read on N servers and succeeds if (at least) R of these reads have succeeded and coincide.
- S: (“server”) number of servers in the network; We must have M ≥ S ≥ N. It corresponds to the number of lines in the ```servers.txt``` files
- W: (“write”) number of functional servers required to store a value in the network; so this is the minimum number of replications of each value in the network; a write operation is trying to write on N servers and succeeds if (at least) W of these writes succeeded.

### Clean

Once you are done, run ```make clean``` to remove executables and ```.o``` files.

## Authors

 - [Ricardo Ribeiro](https://github.com/somecookie)
 - [Anton Ragot](https://github.com/AntonRagot)

