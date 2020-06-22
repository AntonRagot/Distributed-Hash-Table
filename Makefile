CFLAGS+=-W -std=c11 -lcrypto -Wall -Wextra -pedantic -g

DEPENDANCIES = network.o client.o node.o hashtable.o system.o node_list.o util.o error.o args.o ring.o



all: clean pps-launch-server pps-client-get pps-client-put pps-list-nodes pps-client-cat pps-dump-node pps-client-substr pps-client-find

#-----------
# All .o
#-----------

pps-launch-server.o: 

args.o :

system.o : 

week04.o :

hashtable.o :

error.o :

util.o :

pps-client-put.o : 

network.o : 

client.o : 

node.o : 

node_list.o :

pps-dump-node.o :

pps-list-nodes.o : 

pps-client.get.o :

pps-client-cat.o :

pps-client-substr.o: 

pps-client-find.o :

ring.o:



#-----------
# week04
#-----------

week04 : week04.o hashtable.o error.o
	gcc $(CFLAGS) week04.o hashtable.o error.o -lcheck -lm -lsubunit -lrt -pthread -o test-hashtable


#----------
# pps-launch-server
#----------

pps-launch-server : pps-launch-server.o $(DEPENDANCIES)
	gcc $(CFLAGS) pps-launch-server.o $(DEPENDANCIES) -o pps-launch-server -lcrypto 


#----------
# pps-client-get
#----------

pps-client-get : pps-client-get.o $(DEPENDANCIES)
	gcc $(CFLAGS) pps-client-get.o $(DEPENDANCIES) -o pps-client-get -lcrypto 


#----------
# pps-client-put
#----------

pps-client-put : pps-client-put.o $(DEPENDANCIES)
	gcc $(CFLAGS) pps-client-put.o $(DEPENDANCIES) -o pps-client-put -lcrypto 


#----------
# pps-list-nodes
#----------

pps-list-nodes : pps-list-nodes.o $(DEPENDANCIES)
	gcc $(CFLAGS) pps-list-nodes.o $(DEPENDANCIES) -o pps-list-nodes -lcrypto 

#----------
# pps-dump-node
#----------

pps-dump-node : pps-dump-node.o $(DEPENDANCIES)
	gcc $(CFLAGS) pps-dump-node.o $(DEPENDANCIES) -o pps-dump-node -lcrypto 


#----------
# pps-client-cat
#----------

pps-client-cat : pps-client-cat.o $(DEPENDANCIES)
	gcc $(CFLAGS) pps-client-cat.o $(DEPENDANCIES) -o pps-client-cat -lcrypto 

#----------
# pps-client-substr
#----------

pps-client-substr : pps-client-substr.o $(DEPENDANCIES)
	gcc $(CFLAGS) pps-client-substr.o $(DEPENDANCIES) -o pps-client-substr -lcrypto 


#----------
# pps-client-find
#----------

pps-client-find : pps-client-find.o $(DEPENDANCIES)
	gcc $(CFLAGS) pps-client-find.o $(DEPENDANCIES) -o pps-client-find -lcrypto 

#----------
# clean
#----------

clean: 
	rm -f *.o pps-launch-server pps-client-get pps-client-put pps-list-nodes pps-client-cat pps-dump-node pps-client-substr pps-client-find teston



