/**
 * @file pps-launch-server.c
 * @brief starts a server on localhost
 *
 * @date 21.03.2018
 */

#include <stdio.h>
#include <stdlib.h>        //for exit
#include <errno.h>            //for errno
#include <stdint.h>            //for int32_t
#include <ctype.h>       //isspace
#include <string.h>            //strtok

#include <sys/socket.h>    //for revfrom

#include "system.h"
#include "config.h"
#include "hashtable.h"
#include "node.h"

#define SIZE_PAIR_OCTET 5
#define SIZE_KEY_OCTET 1

#define MAX_IP_SIZE 15
#define MAX_PORT_LENGTH 5
#define MAX_SIZE_LINE 22
#define MAX_PORT_LENGTH 5
#define UDP_SIZE 65507

// =====================================================================

int main(void) {

    Htable_t table = construct_Htable(HTABLE_SIZE);

    //We bind the socket to s (without timeout)
    int s = get_socket(0);
    if (s == -1) {
        fprintf(stderr, "Error : get_socket in pps-launch-server");
        return EXIT_FAILURE;
    }

    printf("IP port? ");

    char line[MAX_SIZE_LINE+1];
    memset(line, 0, (MAX_SIZE_LINE+1)* sizeof(char));

    if(fgets(line, MAX_SIZE_LINE+1, stdin) == NULL){
        fprintf(stderr, "Error: could not get the input (IP/Port) from stdin\n");
        return EXIT_FAILURE;
    }

    char IP[MAX_IP_SIZE+1];
    memset(IP, 0, (MAX_IP_SIZE+1)* sizeof(char));
    uint16_t port = 0;
    int args = sscanf(line,"%s %hu", IP, &port);

    if(args != 2){
        fprintf(stderr,"Error: sscanf failed\n");
        return EXIT_FAILURE;
    }

    //Bind the socket to PPS_DEFAULT_IP and PPS_DEFAULT_PORT
    if (bind_server(s, IP, port) != 0) {
        fprintf(stderr, "Error : bind_server in pps-launch-server");
        return EXIT_FAILURE;
    }


    while (1) {

        struct sockaddr_in cli_addr;
        socklen_t addr_len = sizeof(cli_addr);
        memset(&cli_addr, 0, addr_len);

        //Allocate memory for incoming message
        char *in_msg = calloc(MAX_MSG_SIZE+1, sizeof(char));
        if (in_msg == NULL) {
            fprintf(stderr, "Could not allocate in_msg");
            return EXIT_FAILURE;
        }

        ssize_t sizeMsg = recvfrom(s, in_msg, MAX_MSG_SIZE, 0, (struct sockaddr *) &cli_addr, &addr_len);

        ssize_t sendto_err = -1;
        char *get0;

        //Get a pointeur to the position of the first '\0' if any
        if (sizeMsg < 0) {
            fprintf(stderr, "recvfrom didn't work");
            continue;
        } else {
            get0 = memchr(in_msg, '\0', sizeMsg);
        }


        if (sizeMsg == 0) {
            sendto_err = sendto(s, NULL, 0, 0, (struct sockaddr *) &cli_addr, addr_len);
        } else {
            //handle client get
            if (get0 == NULL && sizeMsg >= 0) {
                pps_value_t datagram = get_Htable_value(table, in_msg);

                //key is not in the hashtable
                if (datagram == NULL) {
                    pps_value_t d = calloc(1, sizeof(char));
                    sendto_err = sendto(s, d, 1, 0, (struct sockaddr *) &cli_addr, addr_len);
                    free_const_ptr(d);
                    //the value that is associated with the key is empty
                } else {
                    sendto_err = sendto(s, datagram, strlen(datagram), 0, (struct sockaddr *) &cli_addr, addr_len);
                }
                free_const_ptr(datagram);

            } else if (get0 != NULL && sizeMsg == 1 && in_msg[0] == '\0') {
                //get content from table
                kv_list_t *content = get_Htable_content(table);





                char *toSend = calloc(UDP_SIZE, sizeof(char));

                if (toSend == NULL) {
                    fprintf(stderr, "Could not allocate memory for the message in server\n");
                    return EXIT_FAILURE;
                }

                size_t used = sizeof(int);

                size_t content_size = htonl(content->size);
                memcpy(toSend,&content_size, sizeof(int));


                if (content->size == 0){
                    sendto(s, toSend, used, 0, (struct sockaddr *) &cli_addr, addr_len);
                    free(toSend);
                    toSend = NULL;
                }else{
                    for (size_t i = 0; i < content->size; ++i) {
                        //compute size of <key>\0<value>
                        size_t new_bytes = strlen(content->pairs[i].key) + strlen(content->pairs[i].value) + 2;

                        //if we add the new pair to the string, check if the length exceeds UDP_SIZE (and send message if it's the case)
                        if (used + new_bytes > UDP_SIZE) {
                            if (i > 0) {
                                used--;
                            }
                            //send the the message without the new pair and prepare new message
                            sendto_err = sendto(s, toSend, used, 0, (struct sockaddr *) &cli_addr, addr_len);
                            used = 0;
                            memset(toSend, 0, UDP_SIZE * sizeof(char));
                        }

                        //copy <key>\0<value>\0 at the end of toSend
                        strncpy(toSend + used, content->pairs[i].key, strlen(content->pairs[i].key));
                        used = used + strlen(content->pairs[i].key) + 1; //for \0
                        strncpy(toSend + used, content->pairs[i].value, strlen(content->pairs[i].value));
                        used = used + strlen(content->pairs[i].value) + 1;

                    }

                    //if the last chunk of key/value was not sent
                    if (used != 0) {
                        sendto_err = sendto(s, toSend, used - 1, 0, (struct sockaddr *) &cli_addr, addr_len);
                    }

                    free(toSend);
                    toSend = NULL;}

                //handle client put
            } else if (get0 != NULL && sizeMsg >= 0) {
                char *key = calloc(MAX_MSG_SIZE, sizeof(char));
                if (key == NULL) {
                    fprintf(stderr, "Could not alloc key");
                    free(key);
                    return EXIT_FAILURE;
                }

                //Adding the key with a \0 at the end
                strncpy(key, in_msg, strlen(in_msg) + 1);

                char *value = calloc(MAX_MSG_SIZE, sizeof(char));
                if (value == NULL) {
                    fprintf(stderr, "Could not alloc value");
                    free(key);
                    free(value);
                    return EXIT_FAILURE;
                }

                strncpy(value, (get0 + 1), sizeMsg - strlen(key));

                //Send '\0' if there was a problem in adding the value to the HTable, send NULL otherwise
                if (add_Htable_value(table, key, value) != ERR_NONE) {
                    char *datagram = calloc(1, sizeof(char));
                    datagram[0] = '\0';
                    sendto_err = sendto(s, datagram, 1, 0, (struct sockaddr *) &cli_addr, addr_len);
                    free(datagram);
                } else {
                    sendto_err = sendto(s, NULL, 0, 0, (struct sockaddr *) &cli_addr, addr_len);
                }
                free(in_msg);
                free(key);
                free(value);

            }
        }

    }

    delete_Htable_and_content(&table);

    return EXIT_SUCCESS;
}
