/**
 * @file pps-dump-node.c
 * @brief show all content of a node
 *
 * @date 26.03.2018
 */
#include <stdio.h>

#include "client.h"
#include "node.h"
#include "hashtable.h"
#include "network.h"
#include "util.h"

#define UDP_SIZE 65507

int main(int argc, char *argv[]) {
	
     //initialise client
    client_t client;
    client_init_args_t client_args = {&client, 2, 0, (size_t) argc, &argv};

    if (client_init(&client_args) != ERR_NONE) {
		printf("FAIL\n");
        fprintf(stderr, "Erreur : client_init error\n");
        return EXIT_FAILURE;
    }

    client.parsedOpt->N = 1;
    client.parsedOpt->R = 1;
    client.parsedOpt->W = 1;

    //initialize the node corresponding to the server
    node_t serverNode;

    int port = checkPort((*client_args.argv)[1]);

    if(port == -1){
        fprintf(stderr, "Could not parse a valid port\n");
        printf("FAIL\n");
        client_end(&client);
        return EXIT_FAILURE;
    }

    if(checkIP((*client_args.argv)[0], strlen((*client_args.argv)[0])) == 0){
        fprintf(stderr, "Could not parse a valid IP\n");
        client_end(&client);
        printf("FAIL\n");
        return EXIT_FAILURE;
    }

    if(node_init(&serverNode, strdup((*client_args.argv)[0]), (uint16_t)port ,0) != ERR_NONE){
        fprintf(stderr, "Could initilaize a new node for the dump\n");
        client_end(&client);
        printf("FAIL\n");
        return EXIT_FAILURE;
    }

    client.node = node_list_new();

    if (client.node == NULL) {
        client_end(&client);
        node_end(&serverNode);
        fprintf(stderr, "Could not create a new node list for the client in pps-dump-node\n");
        printf("FAIL\n");
        return EXIT_FAILURE;
    }

    if (node_list_add(client.node, serverNode) != ERR_NONE) {
        client_end(&client);
        node_end(&serverNode);
        fprintf(stderr, "Could not add the server's node to the node_list in pps-dump-node\n");
        printf("FAIL\n");
        return EXIT_FAILURE;
    }

    //send message \0 to get all pairs key/value of the server's hashtable
    pps_key_t key = calloc(1, sizeof(char));
    pps_value_t* result = calloc(1, sizeof(char*));

    if(key == NULL || result == NULL){
        client_end(&client);
        node_end(&serverNode);
        fprintf(stderr, "Memory error when allocating memory for the key and the result\n");
        printf("FAIL\n");
        return EXIT_FAILURE;
    }

    if(network_get(client, key, result) != ERR_NONE){
        free_const_ptr(key);
        free_const_ptr(*result);
        free(result);
        client_end(&client);
        fprintf(stderr, "Could not get result from network\n");
        printf("FAIL\n");
        return EXIT_FAILURE;
    }


    //compute the number of pairs with the 4 first bytes of result
    size_t nbr_pairs_exp = 0;
    memcpy(&nbr_pairs_exp, *result, sizeof(int));
    nbr_pairs_exp = ntohl(nbr_pairs_exp);

    size_t nbr_pairs_act = 0;

    int isKey = 1;

    pps_value_t ptr = *result;

    *result += sizeof(int);
    size_t read_bytes = 0;

    //if the hashtable was empty
    if(nbr_pairs_exp == 0){
        free_const_ptr(ptr);
        free(result);
        client_end(&client);
        return EXIT_SUCCESS;
    }

    //print all pairs
    while(nbr_pairs_act < nbr_pairs_exp){

        if(isKey == 1){
            nbr_pairs_act++;
            printf("%s = ", *result+read_bytes);

        }else{
            printf("%s\n", *result+read_bytes);
        }

        while((*result)[read_bytes] != '\0'){
            read_bytes++;
        }

        read_bytes++;

        isKey = !isKey;

    }
    printf("%s\n", *result+read_bytes);

    free_const_ptr(key);
    free_const_ptr(ptr);
    free(result);
    client_end(&client);

    return EXIT_SUCCESS;
}
