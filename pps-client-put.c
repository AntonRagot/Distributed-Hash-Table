/**
 * @file pps-client-put.c
 * @brief allow a client to introduce a pair key/value in the hashtable
 *
 * @date 26.03.2018
 */


#include <stdio.h>
#include <stdint.h>     //for int32_t

#include "node.h"
#include "client.h"
#include "network.h"
#include "hashtable.h"

// =====================================================================

int main(int argc, char *argv[])
{
    //initialise client
    client_t client;
    client_init_args_t client_args = {&client, 2, TOTAL_SERVERS | PUT_NEEDED, (size_t) argc, &argv};

    if (client_init(&client_args) != ERR_NONE) {
        fprintf(stderr, "Erreur : client_init error\n");
        printf("FAIL\n");
        return EXIT_FAILURE;
    }
    
    client.parsedOpt->R = 1;

    //Print FAIL or OK according to the network's response
    if(network_put(client, (*client_args.argv)[0], (*client_args.argv)[1]) != ERR_NONE) {
        printf("FAIL\n");
    } else {
        printf("OK\n");
    }


    client_end(&client);

    return EXIT_SUCCESS;
}
