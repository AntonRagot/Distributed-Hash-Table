/**
 * @file pps-list-nodes.c
 * @brief tool to display if servers are reachable or not
 *
 * @date 25.04.2018
 */

#include <stdio.h>

#include "node_list.h"
#include "network.h"
#include "node.h"
#include "client.h"
#include "hashtable.h"

//Prototypes

/**
 * @brief print the SHA of a given node
 * @param node the node whose SHA you want to print
 */
static void printSHA(node_t node);

// =====================================================================

int main(int argc, char *argv[])
{
    //initialise client
    client_t client;
    client_init_args_t client_args = {&client, 0, 0, (size_t) argc, &argv};

    if (client_init(&client_args) != ERR_NONE) {
        fprintf(stderr, "Erreur : client_init error\n");
        return EXIT_FAILURE;
    }
    
    //Setting N, W and R to 1 as we contact servers 1 by 1
    client.parsedOpt -> N = 1;
    client.parsedOpt -> W = 1;
    client.parsedOpt -> R = 1;

    client.nodes_status = construct_Htable(client.node->size);
    if(client.nodes_status == NULL){
        fprintf(stderr, "Error: could not construct a htable for the the status of the nodes at line %d of %s", __LINE__, __FILE__);
        client_end(&client);
        return EXIT_FAILURE;
    }

    //initialise hashtable
    for(size_t i = 0; i < client.node->size; i++){
        if(add_Htable_value(client.nodes_status, client.node->nodes[i].server_SHA, "FAIL") != ERR_NONE){
            fprintf(stderr, "Error: could not construct a htable for the the status of the nodes at line %d of %s", __LINE__, __FILE__);
            client_end(&client);
            return EXIT_FAILURE;
        }
    }
    
    node_list_sort(client.node, node_cmp_server_addr);
    
    pps_value_t response;
    //send request for the list-nodes operation
    if(network_get(client, NULL, &response) == ERR_NOMEM){
        client_end(&client);
        return EXIT_FAILURE;
    }

	for(size_t i = 0; i < client.node -> size; i++){

        pps_value_t status = get_Htable_value(client.nodes_status, client.node->nodes[i].server_SHA);

        if(status == NULL){
            client_end(&client);
            return EXIT_FAILURE;
        }

		//Display results
		printf("%s %hu (", (client.node -> nodes[i]).ip, (client.node -> nodes[i]).port);
		printSHA(client.node -> nodes[i]);
		printf(") ");
        printf("%s\n", status);

	}

    client_end(&client);

    return EXIT_SUCCESS;
}

// =====================================================================

static void printSHA(node_t node){
    for(size_t i = 0; i < SHA_DIGEST_LENGTH; ++i){
        printf("%02x", node.SHA[i]);
    }
}
