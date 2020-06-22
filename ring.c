/**
 * @file ring.c
 * @brief Ring parsing and handling. This is required only from week 11.
 *
 */

#include <stddef.h>
#include <stdint.h>

#include "ring.h"
#include "error.h"
#include "hashtable.h"
#include "node_list.h"

#define MAX_IP_SIZE 15
#define MAX_PORT_LENGTH 5
#define MAX_SIZE_IP_PORT 20

ring_t *ring_alloc(){
    ring_t* ring = node_list_new();

    if(ring == NULL){
        fprintf(stderr, "Memory error while allocating memory for the new ring\n");
        return NULL;
    }

    return ring;

}


error_code ring_init(ring_t *ring){
    ring_t* newRing = get_nodes();

    if(newRing == NULL){
        fprintf(stderr, "Could not get the nodes from the file while initializing the ring\n");
        return ERR_NOMEM;
    }

    node_list_sort(newRing, node_cmp_sha);

    *ring = *newRing;
    free(newRing);

    return ERR_NONE;
}


void ring_free(ring_t *ring){
    node_list_free(ring);
}


node_list_t *ring_get_nodes_for_key(const ring_t *ring, size_t wanted_list_size, pps_key_t key){

    node_list_t* list = node_list_new();

    if(list == NULL){
        fprintf(stderr, "Error: Could not create a new node_list at line %d of %s\n", __LINE__, __FILE__);
        return NULL;
    }

    //compute the SHA-1 of the key
    char SHA_key[SHA_DIGEST_LENGTH];
    memset(SHA_key, 0, SHA_DIGEST_LENGTH*sizeof(char));
    SHA1(key, strlen(key), SHA_key);

    //construct an hashtable that stores the strings <IP><port> with a value "seen"
    const char seen[] = "seen";
    Htable_t ip_port_table = construct_Htable(wanted_list_size);
    if(ip_port_table == NULL){
        node_list_free(list);
        fprintf(stderr, "Could not construct a hashtable in ring_get_nodes_for_key\n");
        return NULL;
    }

    //skip the nodes s.t SHA_node < SHA_key
    size_t i = 0;

    while(i < ring->size && strcmp(ring->nodes[i].SHA, SHA_key) < 0){
        i++;
    }

    //find the N wand_list_size servers
    size_t nbr_servers = 0;
    while(nbr_servers < wanted_list_size){

        //build the key of the hashtable
        char ip_port[MAX_SIZE_IP_PORT+1];
        memset(ip_port, 0, (MAX_SIZE_IP_PORT+1)*sizeof(char));
        strncpy(ip_port, ring->nodes[i].ip, MAX_IP_SIZE);
        char port_str[MAX_PORT_LENGTH + 1];
        memset(port_str, 0, (MAX_PORT_LENGTH+1)*sizeof(char));

        if(sprintf(port_str, "%hu",ring->nodes[i].port ) < 0){
            fprintf(stderr, "Error: could not convert the port into a string\n");
            delete_Htable_and_content(&ip_port_table);
            node_list_free(list);
            return NULL;
        }
        strncat(ip_port, port_str, MAX_PORT_LENGTH);

        //the server has not been seen yet
        if(get_Htable_value(ip_port_table, ip_port) == NULL){
            nbr_servers++;
            if(node_list_add(list, ring->nodes[i]) != ERR_NONE){
                fprintf(stderr, "Error: could add a new node to the list in ring_get_nodes_for_key\n");
                delete_Htable_and_content(&ip_port_table);
                node_list_free(list);
                return NULL;
            }

            if(add_Htable_value(ip_port_table,ip_port, seen) != ERR_NONE){
                    fprintf(stderr, "Error: could add a new pair IP/port to the hashtable in ring_get_nodes_for_key\n");
                    delete_Htable_and_content(&ip_port_table);
                    node_list_free(list);
                    return NULL;
            }

        }

        i = (i+1)%ring->size;
    }

    delete_Htable_and_content(&ip_port_table);

    return list;

}
