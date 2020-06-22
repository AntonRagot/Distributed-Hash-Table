/**
 * @file client.c
 * @brief client
 *
 * @date 21.03.2018
 */
#include <stddef.h> // for size_t
#include <stdio.h>

#include "client.h"
#include "error.h"     // for error_code type
#include "node.h"      // in week 5
#include "node_list.h" // weeks 6 to 10
#include "args.h"      // weeks 10 and after
#include "ring.h"      // weeks 11 and after
#include "node.h"      //in order to close the client
#include "system.h"      //IP/Socket
#include "ring.h"
#include "hashtable.h"


#define BUFFER_SIZE 1024
#define MAX_REQ_SIZE 4 //biggest required size (the one of substring)

/**
 * @brief get the number of line of servers.txt
 * @return the number of line
 */
static size_t get_S(void);

/**
 * @brief checks that N, R and W of a client are valid
 * @param client we want to check
 * @return error_code
 */
static error_code check_args(const client_t* client);

// =====================================================================

void client_end(client_t *client) {
    if (client != NULL) {
        ring_free(client->node);
        client-> node = NULL;
        free(client->parsedOpt);
        client->parsedOpt = NULL;
        delete_Htable_and_content(&client -> nodes_status);
    }
}

// =====================================================================

error_code client_init(client_init_args_t *client_args) {

    M_EXIT_IF_NULL(client_args->client, sizeof(client_args->client), "client.c/client_init");
    
    client_args->client -> name = *(client_args->argv)[0];

	//Removing the first args(name) before computing
    *(client_args->argv) += 1;
    (client_args->argc)--;

    char **argv_1 = *client_args->argv;

    if (client_args->nbArg <= MAX_REQ_SIZE && client_args->argc < client_args->nbArg) {
        fprintf(stderr, "Wrong number of expected argument\n");
        return ERR_BAD_PARAMETER;
    }
    
    
    //Computing optional arguments
    args_t *args = parse_opt_args(client_args->argOpt, client_args->argv);
    if (args == NULL) {
        fprintf(stderr, "parse NULL\n");
        return ERR_BAD_PARAMETER;
    }
    

	//Computing the number of remaining args
    ptrdiff_t diffPtr = *client_args->argv - argv_1;
    client_args->argc -= diffPtr;

    if (client_args->nbArg <= MAX_REQ_SIZE && client_args->argc != client_args->nbArg) {
        free(args);
        args = NULL;
        fprintf(stderr, "Wrong number of argument\n");
        return ERR_BAD_PARAMETER;
    }

    client_args->client->parsedOpt = args;

    ring_t* ring = ring_alloc();

    if(ring == NULL){
        free(args);
        args = NULL;
        fprintf(stderr, "Could not allocate memory for the ring\n");
        return ERR_NOMEM;
    }
    
    //initialize the ring
    if(ring_init(ring) != ERR_NONE){
        free(args);
        args = NULL;
        ring_free(ring);
        fprintf(stderr, "Could not initialize the ring\n");
        return ERR_NOMEM;
    }

    client_args->client->node = ring;

    if (client_args->client->node == NULL) {
        free(args);
        args = NULL;
        ring_free(ring);
        fprintf(stderr, "Can't get the nodes from the file\n");
        return ERR_IO;
    }
    
    //Adapt W and R if N = 1
    if(client_args -> client -> parsedOpt -> N == 1){
		client_args -> client -> parsedOpt -> R = 1;
		client_args -> client -> parsedOpt -> W = 1;
	}

	client_args -> client -> nodes_status = NULL;
	
    check_args(client_args -> client);

    return ERR_NONE;
}

// =====================================================================

static error_code check_args(const client_t* client) {

    //checking that the value of N, W and R are valid
    size_t S = get_S();

    if (S < client->parsedOpt->N || client->parsedOpt->W > client->parsedOpt->N ||
        client->parsedOpt->R > client->parsedOpt->N) {
        fprintf(stderr, "Arguments S: %zu N: %zu, R: %zu and W: %zu are not valid\n", S, client->parsedOpt->N,
                client->parsedOpt->R, client->parsedOpt->W);
        return ERR_BAD_PARAMETER;
    }

    return ERR_NONE;

}

// =====================================================================

static size_t get_S(void) {

    FILE *server_file = fopen(PPS_SERVERS_LIST_FILENAME, "r");

    if (server_file == NULL) {
        fprintf(stderr, "Error: Could not open %s at line %d of %s\n", PPS_SERVERS_LIST_FILENAME, __LINE__, __FILE__);
        fclose(server_file);
        return 0;
    }

    size_t lines = 0;
    char buffer[BUFFER_SIZE + 1];

	//Counting the number of line
    while (fgets(buffer, BUFFER_SIZE + 1, server_file) != NULL && !feof(server_file)) {
        lines++;
    }

    fclose(server_file);
    return lines;

}
