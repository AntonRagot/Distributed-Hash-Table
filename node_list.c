/**
 * @file node_list.c
 * @brief represents a list of nodes
 *
 * @date 28.03.2018
 */

#include <netinet/in.h>
#include <stddef.h>
#include <stdlib.h> //malloc etc..
#include <string.h> //memset
#include <ctype.h> //isspace
#include <stdio.h> //FILLE

#include "node_list.h"
#include "node.h"
#include "config.h"
#include "util.h"

#define MAX_SIZE_LINE 43  // MAX_IP_SIZE + 1 + MAX_PORT_LENGTH +1 +MAX_SIZET_STR_SIZE +1 (\n)
#define MAX_IP_SIZE 15

// =====================================================================

node_list_t *node_list_new() {
    //initialialize new pointers for the node_list_t and the list of nodes
    node_list_t *list = calloc(1, sizeof(node_list_t));

    if (list == NULL) {
        fprintf(stderr, "No memory error when allocating memory for a new node_list_t\n");
        return NULL;
    }

    list->size = 0;
    list->nodes = NULL;
    return list;
}

// =====================================================================

node_list_t *get_nodes() {

    FILE *input_server = NULL;
    input_server = fopen(PPS_SERVERS_LIST_FILENAME, "r");

    if (input_server != NULL) {
        node_list_t *list = node_list_new();

        if (list == NULL) {
            fprintf(stderr, "New node_list was null");
            fclose(input_server);
            return NULL;
        }

        char line[MAX_SIZE_LINE+1];
        memset(line, 0, (MAX_SIZE_LINE+1)*sizeof(char));

        //for each line of servers.txt get the IP, the port and  the number of nodes and then create the nodes
        while (!feof(input_server) && !ferror(input_server) && fgets(line, MAX_SIZE_LINE+1, input_server) != NULL){
            //get the IP, port and number of nodes from the file
            char IP[MAX_IP_SIZE +1];
            memset(IP, 0, (MAX_IP_SIZE+1)* sizeof(char));
            uint16_t port = 0;
            size_t nbr_nodes = 0;

            if(sscanf(line, "%s %hu %zu", IP, &port, &nbr_nodes)!= 3){
                fprintf(stderr, "Error while reading servers.txt\n");
                fclose(input_server);
                node_list_free(list);
                return NULL;
            }

            //create the new nodes
            for(size_t id = 1; id <= nbr_nodes; ++id){
                node_t newNode;

                if(node_init(&newNode, strdup(IP), port, id) != ERR_NONE){
                    fprintf(stderr, "Error while initializing a new node\n");
                    fclose(input_server);
                    node_list_free(list);
                    return NULL;
                }
                //add the node to the list being built
                if(node_list_add(list, newNode) != ERR_NONE){
                    fprintf(stderr, "Error while adding an element to the list\n");
                    fclose(input_server);
                    node_list_free(list);
                    return NULL;
                }
            }


        }

        if (ferror(input_server)){
            fprintf(stderr, "Error in input_server\n");
            fclose(input_server);
            return NULL;
        }

        fclose(input_server);
        
        return list;
    }

    fclose(input_server);
    fprintf(stderr, "Given file was NULL in get_nodes()\n");
    return NULL;

}

// =====================================================================

void node_list_free(node_list_t *list) {
    if (list != NULL && list->nodes != NULL) {

        //stops each node of the list
        for (size_t i = 0; i < list->size; i++) {
            node_end(&list->nodes[i]);
        }

        //free the space that the list occupied
        free(list->nodes);
        list->nodes = NULL;

        list->size = 0;

        //free the space of the pointer
        free(list);
        list = NULL;
    }

}

// =====================================================================

error_code node_list_add(node_list_t *list, node_t node) {

    if (list == NULL) {
        return ERR_BAD_PARAMETER;
    }

    list->nodes = realloc(list->nodes, (list->size + 1) * sizeof(node_t));

    M_EXIT_IF_NULL(list->nodes, sizeof(list->nodes), "Could not realloc memory in node_list_add.");

    (list->size)++;

    list->nodes[list->size - 1] = node;

    return ERR_NONE;

}

// =====================================================================

void node_list_sort(node_list_t *list, int (*comparator)(const node_t *, const node_t *)){
    qsort(list->nodes, list->size, sizeof(node_t), comparator);
}
