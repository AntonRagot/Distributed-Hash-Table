/**
 * @file network.c
 * @brief represents the network-client communication
 *
 * @date 21.03.2018
 */

// for sockets
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <limits.h>
#include <errno.h>

#include "network.h"
#include "error.h"
#include "util.h"

#define UDP_SIZE 65507

// =====================================================================

/**
 * @brief send a request to a server
 * @param client client to use
 * @param toSend request to send
 * @param size size of the request
 * @return an error code
 */
static error_code send_to_server(node_t node, char **toSend, size_t size, int socket);

/**
 * @brief get a response  from a server
 * @param client client to use
 * @param response value that will be modifiy
 * @param size size of the response
 * @return -1 if there is an error or the number of bytes recieves
 */
static ssize_t get_response_from_server(struct sockaddr *server_addr, char **response, size_t size, int socket);

/**
 * @param client client that asked to get the content of its node
 * @param key '\0'
 * @param result string composed of the keys and values
 * @return some error_code
 */

static error_code dump(client_t client, struct sockaddr *server_addr, pps_key_t key, char **result, int socket);

/**
 * @brief prepare the packet before it is sent to the server in put
 * @param key
 * @param value
 * @param toSend the packet that is being prepared
 * @return -1 if there is a memory error, the size of the whole message otherwise
 */
static int prepare_put_packet(pps_key_t key, pps_value_t value, char **toSend);


// =====================================================================

error_code network_get(client_t client, pps_key_t key, pps_value_t *value)
{
    size_t sizeKey = 0;
    if (key != NULL) {
        sizeKey = strlen(key);
        if(sizeKey > MAX_MSG_ELEM_SIZE) {
            fprintf(stderr, "The key is too long in %s\n", __FILE__);
            return ERR_BAD_PARAMETER;
        }
    }

    //Look for servers
    if (client.node == NULL) {
        printf("FAIL\n");
        M_EXIT(ERR_IO, "Unable to read PPS_SERVERS_LIST_FILENAME");
    } else {
        struct sockaddr server_addr_rcv_from;

        int socket = get_socket(1);
        if (socket == -1) {
            return ERR_NETWORK;
        }

        //Handle list node
        if (key == NULL) {

            char FAIL[] = "FAIL";
            char OK[] = "OK";

            for (size_t i = 0; i < client.node->size; i++) {

                //try to reach each node of the ring
                if (send_to_server(client.node->nodes[i], &key, sizeKey, socket) != ERR_NONE) {
                    if (add_Htable_value(client.nodes_status, client.node->nodes[i].SHA, FAIL) != ERR_NONE) {
                        fprintf(stderr, "Could not add a new status to the hashtable at line %d in %s\n", __LINE__,
                                __FILE__);
                        return ERR_NOMEM;
                    }
                }
            }

            int size = -1;
            size_t nbrNodes = 0;

            do {
                char *response = calloc(MAX_MSG_ELEM_SIZE + 1, sizeof(char));

                if (response == NULL) {
                    fprintf(stderr, "Could not allocate memory for response at line %d in %s\n", __LINE__, __FILE__);
                    free(response);
                    return ERR_NOMEM;
                }

                size = get_response_from_server(&server_addr_rcv_from, &response, MAX_MSG_SIZE, socket);

                for (size_t j = 0; size != -1 && j < client.node->size; j++) {
                    if (memcmp(&(client.node->nodes[j].srv_addr), &server_addr_rcv_from,
                               sizeof(server_addr_rcv_from)) == 0) {
                        if (add_Htable_value(client.nodes_status, client.node->nodes[j].server_SHA, OK) != ERR_NONE) {
                            fprintf(stderr, "Could not add a new status to the hashtable at line %d in %s\n", __LINE__,
                                    __FILE__);
                            free(response);
                            return ERR_NOMEM;
                        }
                        nbrNodes++;
                        break;
                    }
                }
                free(response);
            } while (size != -1 && nbrNodes < client.node->size);


            return ERR_NONE;
        }

        //dump the content of the hashtable
        if (key != NULL && key[0] == '\0') {
            if(dump(client, &server_addr_rcv_from, key, value, socket) != ERR_NONE) {
                fprintf(stderr, "Error when dumping content of the hashtable\n");
                return ERR_NETWORK;
            }

            return ERR_NONE;
        }

        Htable_t table = construct_Htable(client.parsedOpt->N);
        M_EXIT_IF_NULL(table, sizeof(Htable_t),
                       "Memory error in network get while constructing the internal hashtable\n");


        //get the sublist corresponding to the key
        node_list_t *sublist = ring_get_nodes_for_key(client.node, client.parsedOpt->N, key);

        if (sublist == NULL) {
            delete_Htable_and_content(&table);
            fprintf(stderr, "Could not get the N nodes in network-get\n");
            return ERR_NOMEM;
        }



        //Try to get the key in N servers
        for (size_t i = 0; i < client.parsedOpt->N; i++) {

            //send the key to the server
            send_to_server(sublist->nodes[i], &key, sizeKey, socket);
        }
        ssize_t nbr_bytes = -1;
        size_t i = 0;
        do {

            //get the response from the server
            char *response = calloc(MAX_MSG_ELEM_SIZE + 1, sizeof(char));
            nbr_bytes = get_response_from_server(&server_addr_rcv_from, &response, MAX_MSG_SIZE, socket);
            response[nbr_bytes] = '\0';
            //check that the response is valid
            if ((nbr_bytes >= 0 && nbr_bytes != 1) || (nbr_bytes == 1 && response[0] != '\0')) {

                char *count = get_Htable_value(table, response);

                //first time that a value is received
                if (count == NULL && client.parsedOpt->R > 1) {

                    char *newCount = calloc(2, sizeof(char));
                    strncpy(newCount, "\x01", 1);
                    if(add_Htable_value(table, response, newCount) != ERR_NONE) {
                        fprintf(stderr, "Memory error when adding a new value in the hashtable");
                        free(response);
                        delete_Htable_and_content(&table);
                        free(count);
                        return ERR_NETWORK;
                    }


                    free(count);
                    count = NULL;
                    //first time that a value is received and R = 1
                } else if (count == NULL && client.parsedOpt->R == 1) {
                    *value = strdup(response);

                    //frees
                    free(response);
                    response = NULL;
                    free(count);
                    count = NULL;
                    delete_Htable_and_content(&table);

                    return ERR_NONE;
                    //the value has already been received
                } else {
                    //increment the counter by one and check if the new counter equals R
                    ++count[0];
                    if (count[0] == client.parsedOpt->R) {
                        *value = strdup(response);

                        //frees
                        free(response);
                        response = NULL;
                        free(count);
                        count = NULL;
                        delete_Htable_and_content(&table);

                        return ERR_NONE;
                    }

                    //add the new value of the counter to the hashtable (to override the previous value)
                    if(add_Htable_value(table, response, count) != ERR_NONE) {
                        fprintf(stderr, "Memory error when adding a new value in the hashtable");
                        free(response);
                        response = NULL;
                        free(count);
                        count = NULL;
                        delete_Htable_and_content(&table);
                        return ERR_NETWORK;
                    }
                    free(count);
                    count = NULL;

                }
            }

            free(response);
            response = NULL;

        } while(nbr_bytes != -1 && ++i < client.parsedOpt->N);

        //frees
        delete_Htable_and_content(&table);
        return ERR_NETWORK;

    }

}

// =====================================================================

error_code network_put(client_t client, pps_key_t key, pps_value_t value)
{
    M_EXIT_IF_NULL(client.node, sizeof(client.node), "Unable to read PPS_SERVERS_LIST_FILENAME\n");
    M_REQUIRE_NON_NULL(key);
    M_REQUIRE_NON_NULL(value);

    if (strlen(key) > MAX_MSG_ELEM_SIZE || strlen(value) > MAX_MSG_ELEM_SIZE) {
        fprintf(stderr, "The key or the value is to long\n");
        return ERR_BAD_PARAMETER;
    }

    char *response = calloc(1, sizeof(char));
    M_EXIT_IF_NULL(response, sizeof(response), "network_put, response in network.c\n");

    int written = 0;

    char **toSend = malloc(sizeof(char *));
    if(toSend == NULL) {
        fprintf(stderr,"toSend in network_put\n");
        free(response);
        return ERR_NETWORK;
    }

    int sizeToSend = prepare_put_packet(key, value, toSend);

    if(sizeToSend == -1 || sizeToSend > MAX_MSG_SIZE) {
        fprintf(stderr, "Invalid size of packet %s\n", __FILE__);
        free(response);
        free(*toSend);
        free(toSend);

    }

    //get the sublist corresponding to the key
    node_list_t *sublist = ring_get_nodes_for_key(client.node, client.parsedOpt->N, key);

    if (sublist == NULL) {
        fprintf(stderr, "Could not get the N nodes in network-get\n");
        free(response);
        free(*toSend);
        free(toSend);
        return ERR_NOMEM;
    }

    int socket = get_socket(1);
    if (socket == -1) {
        free(response);
        free(*toSend);
        free(toSend);
        return ERR_NETWORK;
    }

    //Put the pair in all servers, fails if one server could not add it to its Htable
    for (size_t i = 0; i < client.parsedOpt->N; i++) {
        if (send_to_server(sublist->nodes[i], toSend, sizeToSend, socket) != ERR_NONE) {
            fprintf(stderr, "Error while sending requests in network put\n");
            free(response);
            free(*toSend);
            free(toSend);
            return ERR_NETWORK;
        }
    }

    struct sockaddr server_addr_rcv_from;
    int size = -1;
    size_t nbServsSeen = 0;
    do {

        size = get_response_from_server(&server_addr_rcv_from, &response, 1, socket);
        nbServsSeen++;

        size_t j = 0;
        int cmp = -1;
        while (cmp != 0 && j++ < client.node->size) {
            cmp = memcmp(&(client.node->nodes[j].srv_addr), &server_addr_rcv_from, sizeof(server_addr_rcv_from));
        }

        if (size == 0 && cmp == 0) {
            written++;
        }
    } while (size != -1 && nbServsSeen < client.parsedOpt->N);

    free(response);
    free(*toSend);
    free(toSend);
    M_EXIT_IF(written < client.parsedOpt->W, ERR_NETWORK, "network.c/network_put",
              "Could not put the new value on enough servers\n");
    return ERR_NONE;

}

// =====================================================================

static error_code send_to_server(node_t node, char **toSend, size_t size, int socket)
{

    //initialize parameters for the method sendto
    struct sockaddr *server_addr = (struct sockaddr *) &node.srv_addr;
    socklen_t addrlen = sizeof(*server_addr);

    ssize_t sent = 0;
    //send the request to the server
    M_EXIT_IF((sent = sendto(socket, *toSend, size, 0,
                             server_addr,
                             addrlen)) == -1 && sent != size,
              ERR_NETWORK, "newtwork.c", "Error when sending a message to the server\n");

    return ERR_NONE;
}

// =====================================================================

static ssize_t get_response_from_server(struct sockaddr *server_addr, char **response, size_t size, int socket)
{
    //initialize parameters for the method recvfrom
    socklen_t addrlen = sizeof(*server_addr);

    //get the response

    ssize_t i = recvfrom(socket, *response, size, 0,
                         server_addr, &addrlen);

    return i;
}

// =====================================================================

static error_code dump(client_t client, struct sockaddr *server_addr, pps_key_t key, char **result, int socket)
{

    //alloc resources
    char *val = calloc(UDP_SIZE, sizeof(char));
    M_EXIT_IF_NULL(val, UDP_SIZE * sizeof(char), "Could not allocate memory for val in dump of network\n");
    size_t allocs = 1;
    size_t used = 0;


    error_code err = send_to_server(client.node->nodes[0], &key, sizeof(char), socket);


    if (err == ERR_NONE) {

        //allocate a buffer to get the response of the server
        char **buffer = malloc(sizeof(char *));
        *buffer = calloc(UDP_SIZE, sizeof(char));
        if(buffer == NULL || *buffer == NULL) {
            fprintf(stderr, "Could not allocate memory in dump of network\n");
            free(val);
            return ERR_NETWORK;
        }

        //get the response of the server
        ssize_t nbr_bytes = get_response_from_server(server_addr, buffer, UDP_SIZE, socket);

        if(nbr_bytes == -1) {
            fprintf(stderr, "Could not get a response from the server in dump of network\n");
            free(val);
            free(*buffer);
            free(buffer);
            return ERR_NETWORK;
        }

        //compute number corresponding to the 4 first bytes
        size_t nbr_pairs_exp = 0;
        memcpy(&nbr_pairs_exp, *buffer, sizeof(int));
        nbr_pairs_exp = ntohl(nbr_pairs_exp);

        if(nbr_pairs_exp == 0) {

            *result = malloc(sizeof(int));

            if(*result == NULL) {
                free(val);
                free(*buffer);
                free(buffer);
                return ERR_NETWORK;
            }

            for(size_t i = 0; i < sizeof(int); i++) {
                (*result)[i] = (*buffer)[i];
            }

            free(result);
            free(*buffer);
            free(buffer);
            free(val);
            buffer = NULL;
            return ERR_NONE;
        }

        size_t nbr_pairs_act = 0;

        used += nbr_bytes;

        //copy and count the number of zeros
        size_t nbr_0 = 0;
        for (size_t i = 0; i < nbr_bytes; i++) {
            val[i] = (*buffer)[i];
            if (i > sizeof(int) && val[i] == '\0') nbr_0++;
        }
        nbr_pairs_act = (nbr_0 / 2) + 1;


        //when the server sends more than one message
        while (nbr_pairs_exp != nbr_pairs_act) {
            //reset memory
            val[used] = '\0';
            used++;
            memset(*buffer, 0, UDP_SIZE * sizeof(char));

            //get the response of the server
            nbr_bytes = get_response_from_server(server_addr, buffer, UDP_SIZE, socket);
            if(nbr_bytes == -1) {
                fprintf(stderr, "Could not get a response from the server in dump of network\n");
                free(val);
                free(*buffer);
                free(buffer);
                return ERR_NETWORK;
            }

            //check if it needs to be reallocated
            if (used % UDP_SIZE + nbr_bytes > UDP_SIZE) {
                allocs++;
                val = realloc(val, allocs * UDP_SIZE * sizeof(char));
                if(val == NULL) {
                    free(val);
                    free(*buffer);
                    free(buffer);

                    fprintf(stderr, "Could not realloc memory for val in dump of network\n");
                    return ERR_NETWORK;
                }
            }

            //count the number of new pairs
            nbr_0 = 0;
            for (size_t i = 0; i < nbr_bytes; ++i) {
                if ((*buffer)[i] == '\0') nbr_0++;
            }
            nbr_pairs_act += nbr_0 / 2 + 1;

            //copy
            for (size_t i = 0; i < nbr_bytes; i++) {
                val[used + i] = (*buffer)[i];
            }

            used += nbr_bytes;
        }

        *result = calloc(used + 1, sizeof(char));
        if(*result == NULL) {
            fprintf(stderr, "Could not reallocate result in %s\n", __FILE__);
            free(val);
            free(*buffer);
            free(buffer);
            return ERR_NETWORK;
        }

        //copy
        for (size_t i = 0; i < used; ++i) {
            (*result)[i] = val[i];
        }

        //frees
        free(*buffer);
        free(buffer);
        free(val);
        buffer = NULL;

        return ERR_NONE;
    }
    fprintf(stderr, "Could not send first message in dump\n");
    free(val);
    return ERR_NETWORK;

}

// =====================================================================

static int prepare_put_packet(pps_key_t key, pps_value_t value, char **toSend)
{

    //Compute the size of the key
    size_t sizeKey = 0;
    while (sizeKey < strlen(key) && key[sizeKey] != '\n' && key[sizeKey] != '\0') {
        sizeKey++;
    }

    char *tempKey = calloc(sizeKey + 1, sizeof(char));
    if (tempKey == NULL) {
        fprintf(stderr, "Memory error: tempKey of prepare_put_packet");
        return -1;
    }

    //Extract the wanted part of the key
    strncpy(tempKey, key, sizeKey);

    //Compute the size of the value
    size_t sizeValue = 0;
    while (sizeValue < strlen(value) && value[sizeValue] != '\n' && value[sizeValue] != '\0') {
        sizeValue++;
    }

    char *tempValue = calloc(sizeValue, sizeof(char));

    if (tempValue == NULL) {
        fprintf(stderr, "Memory error: tempKey of prepare_put_packet");
        free(tempKey);
        return -1;
    }

    //Extract the wanted part of value
    strncpy(tempValue, value, sizeValue);

    size_t sizeToSend = strlen(tempKey) + 1 + strlen(tempValue);

    *toSend = calloc(sizeToSend, sizeof(char));

    if (*toSend == NULL) {
        fprintf(stderr, "Memory error: toSend of prepare_put_packet");
        free(tempKey);
        free(tempValue);
        return -1;
    }

    //Creating message to send of the form <key\0value>
    strncpy(*toSend, tempKey, sizeKey);
    strncpy(*toSend + strlen(tempKey) + 1, tempValue, sizeValue);

    free(tempKey);
    free(tempValue);

    return sizeToSend;
}
