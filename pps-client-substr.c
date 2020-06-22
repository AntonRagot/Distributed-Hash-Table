/**
 * @file pps-client-substr.c
 * @brief allow to extract a substring of a value assoicated to a given key in order to create a new pair of key and value
 *
 * @date 06.05.2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "node.h"
#include "client.h"
#include "network.h"
#include "hashtable.h"

// =====================================================================

int main(int argc, char *argv[])
{

    //initialise client
    client_t client;
    client_init_args_t client_args = {&client, 4, TOTAL_SERVERS | GET_NEEDED | PUT_NEEDED, (size_t) argc, &argv};

    if (client_init(&client_args) != ERR_NONE) {
        printf("FAIL\n");
        fprintf(stderr, "Error : client_init error\n");
        return EXIT_FAILURE;
    }
    
    pps_value_t* value = calloc(1, sizeof(char*));

	//Getting the value of the given key
    if(network_get(client, (*client_args.argv)[0], value) != ERR_NONE) {
        printf("FAIL\n");
        free_const_ptr(*value);
        free(value);
        client_end(&client);
        return EXIT_FAILURE;
    }

    int starting_pos = 0;
    int err = sscanf((*client_args.argv)[1], "%d", &starting_pos);

    if(err != 1){
        fprintf(stderr, "Error: sscanf failed at line %d of %s\n", __LINE__, __FILE__);
        printf("FAIL\n");
        free_const_ptr(*value);
        free(value);
        client_end(&client);
        return EXIT_FAILURE;
    }

    //check if it is a valid starting position
    if (starting_pos >= 0 && (size_t)starting_pos >= strlen(*value)) {
        printf("FAIL\n");
        free_const_ptr(*value);
        free(value);
        client_end(&client);
        return EXIT_FAILURE;
    } else if (starting_pos < 0 && (size_t)abs(starting_pos) > strlen(*value)) {
        printf("FAIL\n");
        free_const_ptr(*value);
        free(value);
        client_end(&client);
        return EXIT_FAILURE;
    }

    size_t length = 0;
    err = sscanf((*client_args.argv)[2], "%zu", &length);

    if(err != 1){
        fprintf(stderr, "Error: sscanf failed at line %d of %s\n", __LINE__, __FILE__);
        printf("FAIL\n");
        free_const_ptr(*value);
        free(value);
        client_end(&client);
        return EXIT_FAILURE;
    }

    //check if it is a valid length
    if (starting_pos >= 0 && starting_pos + length > strlen(*value)) {
        printf("FAIL\n");
        free_const_ptr(*value);
        free(value);
        client_end(&client);
        return EXIT_FAILURE;
    } else if (starting_pos < 0 && length > (size_t)abs(starting_pos)) {
        printf("FAIL\n");
        free_const_ptr(*value);
        free(value);
        client_end(&client);
        return EXIT_FAILURE;
    }



    char *new_value = calloc(length + 1, sizeof(char));

    if (new_value == NULL) {
        printf("FAIL\n");
        free_const_ptr(*value);
        free(value);
        client_end(&client);
        return EXIT_FAILURE;
    }

    new_value[length] = '\0';

    //copy the new string;
    if (starting_pos >= 0) {
        strncpy(new_value, *value + starting_pos, length);
    } else {
        strncpy(new_value, *value + strlen(*value) + starting_pos, length);
    }

    if (network_put(client, (*client_args.argv)[3], new_value) == ERR_NONE) {
        printf("OK\n");

    } else {
        printf("FAIL\n");
        free(new_value);
        free_const_ptr(*value);
        free(value);
        client_end(&client);
        return EXIT_FAILURE;
    }

    free(new_value);
    free_const_ptr(*value);
    free(value);
    client_end(&client);

    return EXIT_SUCCESS;
}
