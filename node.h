#pragma once

/**
 * @file node.h
 * @brief Node (= server until week 11) definition and related functions
 *
 * @author Valérian Rousset
 */

#include <netinet/in.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "error.h"
#include "error.h"
#include "util.h" // for _unused macro
#include "config.h" // for macros (default IP etc)
#include "system.h"

/**
 * @brief node data structure
 */
typedef struct{
	const char* ip;
	uint16_t port;
	size_t id;
	struct sockaddr_in srv_addr;
	const unsigned char* SHA;
	const unsigned char* server_SHA; //IP, port and id = 0
} node_t;

/**
 * @brief node initialization function
 * @param node node to be initalized (modified)
 * @param ip server IP address
 * @param port server port
 * @param node_id after week 11 (included), specify the server node id in a key ring. Unused before week 11.
 * @return some error code
 */
error_code node_init(node_t *node, const char *ip, uint16_t port, size_t _unused node_id);

/**
 * @brief all what needs to be done when a node is removed.
 *        Actually useless (=empty) in the current version of the project, but remains here as a placeholder
 * @param node the node that is removed, passed by reference as it might be modified.
 */
void node_end(node_t *node);

/**
 * @brief tool function to sort nodes according to their SHA
 * @param two nodes to be compared
 * @return <0 if first node comes first, 0 if equal and >0 is second node comes first
 */
int node_cmp_sha(const node_t *first, const node_t *second);

/**
 * @brief tool function to sort node according to their SERVER address
 * @param two nodes to be compared
 * @return <0 if server of first node comes first, 0 if equal and >0 is sever of second node comes first
 */
int node_cmp_server_addr(const node_t *first, const node_t *second);

/**
 *@brief checks that the IP contained in IP is valid
 *@param IP IP to check
 *@param size size of the IP
 *@return 1 if the IP is valid or 0 otherwise
 */
int checkIP(char *IP, size_t size);

/**
 * @brief check that a given port (string) is valid
 * @param port as a string
 * @return -1 if it is not valid or the port number
 */
int checkPort(char* port);


unsigned char* compute_sha(const char *ip, uint16_t port, size_t _unused node_id);
