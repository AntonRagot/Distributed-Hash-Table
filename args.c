/**
 * @file args.c
 * @brief Parsing argv options
 *
 * @date 09.05.2018
 */

#include <stdio.h>
#include <stddef.h> // for size_t
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "args.h"


#define ASCII_0 48
#define ASCII_9 57

//======================================================================


/**
 * @brief check if a given char* is a number or not
 * @param a string char* to check
 * @return 1 if it is a number, 0 otherwise
 */
static int isNumber(char *str);

/**
 * @brief initialise args_t to N = 3, R = 2, W = 2
 * @return a pointer to the newly initialised args_t
 */
static args_t *args_t_init();


//======================================================================

args_t *parse_opt_args(size_t supported_args, char ***rem_argv) {

    if (rem_argv == NULL) {
        fprintf(stderr, "rem _argv was NULL in parse_opt_args\n");
        return NULL;
    }

    args_t *parsedArgs = args_t_init();

    if (parsedArgs == NULL) {
        fprintf(stderr, "args_t_init gives NULL in parse_opt_args\n");
        return NULL;
    }

    size_t oneN = 0;
    size_t oneW = 0;
    size_t oneR = 0;

    if (supported_args != 0) {

        //while we have arguments
        while (strncmp((*rem_argv)[0], "--", 2) != 0) {
            //Check for -n
            if (strncmp((*rem_argv)[0], "-n", 2) == 0) {
                //Check for the flag and if we already have a -n
                if (supported_args & TOTAL_SERVERS && oneN == 0) {
                    ++(*rem_argv);

                    //check if the following arg is a number
                    if (isNumber((*rem_argv)[0]) == 1) {
                        oneN++;
                        sscanf((*rem_argv)[0], "%zu", &(parsedArgs->N));
                        ++(*rem_argv);
                    } else {
                        fprintf(stderr, "rem _argv was not a number after -n in parse_opt_args\n");
                        free(parsedArgs);
                        parsedArgs = NULL;
                        return NULL;
                    }
                } else {
                    fprintf(stderr, "unexpected -n in parse_opt_args\n");
                    free(parsedArgs);
                    parsedArgs = NULL;
                    return NULL;
                }

            }
                //Check for -r
            else if (strncmp((*rem_argv)[0], "-r", 2) == 0) {
                //Check for the flag and if we already have a -r
                if (supported_args & GET_NEEDED && oneR == 0) {
                    ++(*rem_argv);

                    //check if the following arg is a number
                    if (isNumber((*rem_argv)[0]) == 1) {
                        oneR++;
                        sscanf((*rem_argv)[0], "%zu", &(parsedArgs->R));;
                        ++(*rem_argv);
                    } else {
                        fprintf(stderr, "rem _argv was not a number after -r in parse_opt_args\n");
                        free(parsedArgs);
                        parsedArgs = NULL;
                        return NULL;
                    }
                } else {
                    fprintf(stderr, "unexpected -r in parse_opt_args\n");
                    free(parsedArgs);
                    parsedArgs = NULL;
                    return NULL;
                }

            }
                //Check for -w
            else if (strncmp((*rem_argv)[0], "-w", 2) == 0) {
                //Check for the flag and if we already have a -w
                if (supported_args & PUT_NEEDED && oneW == 0) {
                    ++(*rem_argv);

                    //check if the following arg is a number
                    if (isNumber((*rem_argv)[0]) == 1) {
                        oneW++;
                        sscanf((*rem_argv)[0], "%zu", &(parsedArgs->W));
                        ++(*rem_argv);
                    } else {
                        fprintf(stderr, "rem _argv was not a number after -w in parse_opt_args\n");
                        free(parsedArgs);
                        parsedArgs = NULL;
                        return NULL;
                    }
                } else {
                    fprintf(stderr, "unexpected arg in parse_opt_args\n");
                    free(parsedArgs);
                    parsedArgs = NULL;
                    return NULL;
                }

            } else {
                //arg is not an optional and there is no [--]
                return parsedArgs;
            }

        }
        //Found a [--]
        ++(*rem_argv);
    }
    return parsedArgs;

}

//======================================================================


static args_t *args_t_init() {
    //args_t *parsedArgs = calloc(3, sizeof(size_t));
    args_t *parsedArgs = malloc(sizeof(args_t));

    if (parsedArgs == NULL) {
        fprintf(stderr, "Could not allocate memory in args_t_init\n");
        return NULL;
    }

    parsedArgs->N = 3;
    parsedArgs->R = 2;
    parsedArgs->W = 2;

    return parsedArgs;
}

//======================================================================


static int isNumber(char *str) {
    if (str == NULL) {
        fprintf(stderr, "str was NULL in isNumber\n");
        return 0;
    }

    for (size_t i = 0; i < strlen(str); ++i) {
        if ((int) str[i] < ASCII_0 || (int) str[i] > ASCII_9) {
            return 0;
        }
    }

    return 1;
}
