/**
 * @file node.c
 * @brief represents the nodes of the ring
 *
 * @date 21.03.2018
 */

#include <netinet/in.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <openssl/sha.h>

#include "node.h"
#include "error.h"
#include "util.h" // for _unused macro
#include "config.h" // for macros (default IP etc)
#include "system.h"

#define MAX_IP_SIZE 15
#define MAX_PORT_LENGTH 5
#define ASCII_0 48
#define ASCII_9 57
#define IP_FORMAT 4
#define MAX_PORT 65535
#define MAX_SIZE_LINE 43  // MAX_IP_SIZE + 1 + MAX_PORT_LENGTH +1 +MAX_SIZET_STR_SIZE +1 (\n)
#define MAX_SIZET_STR_SIZE 20
#define MAX_UINT16_STR_SIZE 5

/**
 *
 * @param ip
 * @param port
 * @param node_id
 * @return the SHA-1 corresponding to "<IP>_<PORT>_<NODE-ID>" (where _ is a space) or NULL if there is an error
 */
unsigned char* compute_sha(const char *ip, uint16_t port, size_t _unused node_id);

//======================================================================

void node_end(node_t *node) {
    if(node != NULL){
        free_const_ptr(node->server_SHA);
        free_const_ptr(node->SHA);
        free_const_ptr(node->ip);
    }
}

//======================================================================
error_code node_init(node_t *node, const char *ip, uint16_t port, size_t _unused node_id)
{

	//get the address of the server
    struct sockaddr_in srv_addr;
    error_code err = get_server_addr(ip, port, &srv_addr);

    if(err != ERR_NONE) {
        return err;
    }

	//initialize the node
    node -> ip = ip;
    node -> port = port;
    node -> id = node_id;
    node -> srv_addr = srv_addr;
    node ->SHA = compute_sha(ip,port, node_id);
    node ->server_SHA = compute_sha(ip, port, 0);

    return ERR_NONE;
}

// =====================================================================

int node_cmp_sha(const node_t *first, const node_t *second){

    return strncmp(first->SHA, second->SHA, strlen(first->SHA));
}

// =====================================================================

int checkIP(char *IP, size_t size) {
    if(IP == NULL){
        fprintf(stderr, "IP was NULL\n");
        return 0;
    }
    if (size <= 0 || size > MAX_IP_SIZE) return 0;

    int count = 1;
    int nbr_points = 0;
    int blocks = 1;

    for (size_t i = 0; i < size; i++) {
        if (IP[i] == '.' && i != 0) {
            count = 1;
            nbr_points++;
            blocks++;
        } else if (count <= 3 && IP[i] >= ASCII_0 && IP[i] <= ASCII_9) {
            count++;
        } else {
            return 0;
        }
    }

    if (nbr_points != IP_FORMAT - 1 || blocks != IP_FORMAT || IP[size - 1] == '.') return 0;

    return 1;
}

// =====================================================================

int checkPort(char* p){

    if(p == NULL){
        fprintf(stderr, "Port was NULL\n");
        return 0;
    }

    int port = 0;
    int err = sscanf(p,"%d", &port);

    if (err != 1 && (port <= 0 || port > MAX_PORT)) {
        fprintf(stderr, "Wrong format for port: %d, in get_node_from_file\n", port);
        return -1;
    }

    return port;
}

// =====================================================================

unsigned char* compute_sha(const char *ip, uint16_t port, size_t _unused node_id){


    //convert ID and port to a string
    char port_str[MAX_UINT16_STR_SIZE + 1];
    memset(port_str, 0, (MAX_UINT16_STR_SIZE+1)*sizeof(char));

    if(sprintf(port_str, "%hu", port) < 0){
        fprintf(stderr, "Error: could not convert the port into a string\n");
        return NULL;
    }

    char id_str[MAX_SIZET_STR_SIZE + 1];
    memset(id_str, 0, (MAX_SIZET_STR_SIZE+1)*sizeof(char));

    if(sprintf(id_str, "%zu", node_id) < 0){
        fprintf(stderr, "Error: could not convert the node id into a string\n");
        return NULL;
    }

    size_t size = strlen(ip) + strlen(id_str) + strlen(port_str) + 2;

    char SHA1_input[size+1];
    memset(SHA1_input, 0, (size+1)*sizeof(char));

    //create the string "<IP> <PORT> <ID>"
    strncpy(SHA1_input, ip, strlen(ip));
    strncat(SHA1_input, " ", 1);
    strncat(SHA1_input, port_str, strlen(port_str));
    strncat(SHA1_input, " ", 1);
    strncat(SHA1_input, id_str, strlen(id_str));

    unsigned char* SHA1_output = calloc(SHA_DIGEST_LENGTH, sizeof(unsigned char));

    if(SHA1_output == NULL){
        fprintf(stderr, "Memory error in compute_sha\n");
        return NULL;
    }

    SHA1((unsigned char*)SHA1_input, size, SHA1_output);

    return SHA1_output;

}

// =====================================================================

int node_cmp_server_addr(const node_t *first, const node_t *second) {
	/*if(first == NULL || second == NULL){
		return ???;
	}*/
	
	int cmpIP = strncmp(first -> ip, second -> ip, MAX_IP_SIZE);
	if(cmpIP != 0) {
		return cmpIP;
	}
	
	if(first -> port > second -> port) {
		return 1;
	} else if (first -> port < second -> port){
		return -1;
	}
	
	return node_cmp_sha(first, second);
	
}
