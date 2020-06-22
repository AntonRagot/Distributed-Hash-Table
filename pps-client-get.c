/**
 * @file pps-client-get.c
 * @brief allow a client to retreive a value in the hashtable by giving the key
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

int main(int argc, char *argv[]) {

    //initialise client
    client_t client;
    client_init_args_t client_args = {&client, 1, TOTAL_SERVERS | GET_NEEDED, (size_t) argc, &argv};

    if (client_init(&client_args) != ERR_NONE) {
		printf("FAIL\n");
        fprintf(stderr, "Erreur : client_init error\n");
        return EXIT_FAILURE;
    }
    
    client.parsedOpt->W = 1;

    pps_value_t *value = calloc(1, sizeof(char*));

    if(value == NULL){
        printf("FAIL\n");
        client_end(&client);
        return EXIT_FAILURE;
    }


    //get the value with the network

    if (network_get(client, (*client_args.argv)[0], value) != ERR_NONE) {
        printf("FAIL\n");
    } else {
        printf("OK %s\n", *value);
    }

	free_const_ptr(*value);
    free(value);
    client_end(&client);

    return EXIT_SUCCESS;
}
