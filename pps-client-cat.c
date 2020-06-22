/**
 * @file pps-client-cat.c
 * @brief concatenate two values
 *
 * @date 02.05.2018
 */

#include <stdio.h>

#include "node_list.h"
#include "network.h"
#include "node.h"
#include "hashtable.h"
#include "client.h"

#include "limits.h"

// =====================================================================

int main(int argc, char *argv[])
{
    //initialise client
    client_t client;
    client_init_args_t client_args = {&client, SIZE_MAX, TOTAL_SERVERS | GET_NEEDED | PUT_NEEDED, (size_t) argc, &argv};

    if (client_init(&client_args) != ERR_NONE || client_args.nbArg <= 1) {
        printf("FAIL\n");
        fprintf(stderr, "Erreur : client_init error\n");
        return EXIT_FAILURE;
    }


    kv_list_t *listToConcatenate = kv_list_init();

    //Once all key are fetched, get the associated value except the last one
    for(size_t i = 0; i < argv_size(*client_args.argv) - 1; i++) {
        pps_value_t *value = calloc(1, sizeof(char*));

        if(value == NULL) {
            printf("FAIL\n");
            fprintf(stderr, "Could not allocate memory for value\n");
            free(value);
            free_const_ptr(*value);
            client_end(&client);
            kv_list_free(listToConcatenate);
            return EXIT_FAILURE;
        }

        pps_key_t key = (*client_args.argv)[i];

        //get the value with the network
        if(network_get(client, key, value) != ERR_NONE) {
            printf("FAIL\n");
            kv_list_free(listToConcatenate);
            free_const_ptr(*value);
            free(value);
            client_end(&client);
            return EXIT_FAILURE;
        } else {
            kv_pair_t p = {strdup(key), strdup(*value)};
            if(kv_pair_add(listToConcatenate, &p) != ERR_NONE) {
                printf("FAIL\n");
                kv_list_free(listToConcatenate);
                free_const_ptr(*value);
                free(value);
                client_end(&client);
                return EXIT_FAILURE;
            }
        }
        free_const_ptr(*value);
        free(value);
    }

    char* newValue = calloc(MAX_MSG_ELEM_SIZE, sizeof(char));

    if(newValue == NULL) {
        printf("FAIL\n");
        fprintf(stderr, "Could not allocate memory for newValue\n");
        kv_list_free(listToConcatenate);
        client_end(&client);
        return EXIT_FAILURE;
    }

    for(size_t i = 0; i < listToConcatenate -> size; i++) {
        if(strlen(newValue) + strlen(listToConcatenate -> pairs[i].value) > MAX_MSG_ELEM_SIZE) {
            printf("FAIL\n");
            fprintf(stderr, "New value too long in pps-client-cat");
            kv_list_free(listToConcatenate);
            client_end(&client);
            return EXIT_FAILURE;
        }
        strncat(newValue, listToConcatenate -> pairs[i].value, strlen(listToConcatenate -> pairs[i].value));
    }	

	
    if(network_put(client, (*client_args.argv)[argv_size(*client_args.argv) - 1], newValue) != ERR_NONE) {
        printf("FAIL\n");
    } else {
        printf("OK\n");
    }

    client_end(&client);
    free(newValue);
    return EXIT_SUCCESS;
}
