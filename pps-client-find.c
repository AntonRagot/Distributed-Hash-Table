/**
 * @file pps-client-find.c
 * @brief find a given substring
 *
 * @date 08.05.2018
 */

#include <stdio.h>

#include "node.h"
#include "client.h"
#include "network.h"
#include "hashtable.h"

int main(int argc, char *argv[])
{

    //initialise client
    client_t client;
    client_init_args_t client_args = {&client, 2, TOTAL_SERVERS | GET_NEEDED | PUT_NEEDED, (size_t) argc, &argv};

    if (client_init(&client_args) != ERR_NONE) {
		printf("FAIL\n");
        fprintf(stderr, "Erreur : client_init error\n");
        return EXIT_FAILURE;
    }

    pps_value_t *value1 = calloc(1, sizeof(char*));
    pps_value_t *value2 = calloc(1, sizeof(char*));
    
    //If error whilst allocating the key
    if(value1 == NULL || value2 == NULL){
        fprintf(stderr, "Null keys\n");
		printf("FAIL\n");
		free(value1);
        free(value2);
        client_end(&client);
        return EXIT_FAILURE;
	}

    if(network_get(client, (*client_args.argv)[0], value1) != ERR_NONE || network_get(client, (*client_args.argv)[1], value2) != ERR_NONE) {
        fprintf(stderr, "Get failed\n");
        printf("FAIL\n");
        free(value1);
        free(value2);
        client_end(&client);
        return EXIT_FAILURE;
    }

    char* ptr = strstr(*value1, *value2);
    ptrdiff_t index = -1;
    if(ptr != NULL) {
        index = ptr - *value1;
    }

    printf("OK %d\n", (int)index);

    free_const_ptr(*value1);
    free_const_ptr(*value2);

    free(value1);
    free(value2);

    client_end(&client);

    return EXIT_SUCCESS;
}
