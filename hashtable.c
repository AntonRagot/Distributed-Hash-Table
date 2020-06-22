/**
 * @file hastable.c
 * @brief standard operations on hashtables
 *
 * @date 14.03.2018
 */

#include <stddef.h> // for size_t
#include <stdint.h> // for int32_t
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "error.h" // for error_code
#include "hashtable.h"
#include "util.h"

/**
 * @brief free all the allocated space of a linked list starting with head
 * @param head first element of the list
 */
static void delete_bucket_list(bucket_t **head);

/**
 * @brief get the bucket corresponding to a given key
 * @param key
 * @param table
 * @return Null if the key is not in the table or the bucket
 */
static bucket_t *get_bucket(pps_key_t key, Htable_t table);

//======================================================================

/**
 * @brief node for the hashtable
 * @var bucket::pair
 * Member 'pair' contains the pair of key and value of the node
 * @var bucket::next
 * Member 'next' contains a pointer to the successor of the node in the linked list
 */
struct bucket {
    kv_pair_t pair;
    struct bucket *next;
};

//======================================================================


error_code add_Htable_value(Htable_t table, pps_key_t key, pps_value_t value)
{

    //test that the arguments are valid
    M_REQUIRE_NON_NULL(table);
    M_REQUIRE_NON_NULL(table->table);
    M_REQUIRE_NON_NULL(key);
    M_REQUIRE_NON_NULL(value);

    //copy key and value
    pps_value_t copied_key = strdup(key);
    M_REQUIRE_NON_NULL_CUSTOM_ERR(copied_key, ERR_NOMEM);
    pps_value_t copied_value = strdup(value);
    M_REQUIRE_NON_NULL_CUSTOM_ERR(copied_value, ERR_NOMEM);

    bucket_t *b = get_bucket(key, table);

    if (b != NULL) {
        //the key is already associated with a bucket in the hashtable
        free_const_ptr(b->pair.value);
        b->pair.value = copied_value;
        return ERR_NONE;
    } else {
        //create a new bucket for the new key

        bucket_t *new_bucket = NULL;
        new_bucket = malloc(sizeof(bucket_t));

        if(new_bucket == NULL) {
            fprintf(stderr, "Error whilst adding a value to the hashtable in %s", __FILE__);
            free_const_ptr(copied_key);
            free_const_ptr(copied_value);
            return ERR_NOMEM;
        }

        //initialize new bucket
        new_bucket->pair.value = copied_value;
        new_bucket->pair.key = copied_key;
        size_t hashed_key = hash_function(copied_key, table->size);
        new_bucket->next = table->table[hashed_key];

        //modify entry of the hashtable
        ++(table->nbr_elems);
        table->table[hashed_key] = new_bucket;

        return ERR_NONE;
    }

}

//======================================================================

static bucket_t *get_bucket(pps_key_t key, Htable_t table)
{

    size_t hashed_key = hash_function(key, table->size);
    size_t n = strlen(key);

    if (table->table[hashed_key] == NULL) return NULL;

    bucket_t *curr = table->table[hashed_key];

    while (curr != NULL) {
        if (strlen(curr->pair.key) == n && strncmp(curr->pair.key, key, n) == 0) {
            return curr;
        }
        curr = curr->next;
    }

    return NULL;

}

//======================================================================

pps_value_t get_Htable_value(Htable_t table, pps_key_t key)
{

    if (table->table == NULL || key == NULL) {
        fprintf(stderr, "Null pointer in get_Htable_value\n");
        return NULL;
    } else {
        bucket_t *b = get_bucket(key, table);

        if (b == NULL) {
            return NULL;
        } else {
            return strdup(b->pair.value);
        }
    }

}

//======================================================================

size_t hash_function(pps_key_t key, size_t table_size)
{
    M_REQUIRE(table_size != 0, SIZE_MAX, "size == %d", 0);
    M_REQUIRE_NON_NULL_CUSTOM_ERR(key, SIZE_MAX);

    size_t hash = 0;
    const size_t key_len = strlen(key);
    for (size_t i = 0; i < key_len; ++i) {
        hash += (unsigned char) key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash % table_size;
}

//======================================================================

Htable_t construct_Htable(size_t size)
{
    if (size == 0) {
        fprintf(stderr, "Size is 0 in construct_Htable\n");
        return NO_HTABLE;
    }

    bucket_t **tab = calloc(size, sizeof(bucket_t *));

    if (tab == NULL) {
        fprintf(stderr, "Memory error in construct_Htable\n");
        return NO_HTABLE;
    }

    //initialize all bucket* to NULL
    for (size_t i = 0; i < size; i++) {
        tab[i] = NULL;
    }

    Htable_t new_htable = malloc(sizeof(Htable));

    if (new_htable == NULL) {
        fprintf(stderr, "Memory error when allocating memory for new_htable\n");
        return NO_HTABLE;
    }

    new_htable->size = size;
    new_htable->nbr_elems = 0;
    new_htable->table = tab;
    return new_htable;
}

//======================================================================

void delete_Htable_and_content(Htable_t *table)
{

    if (table != NULL && *table != NULL) {
        for (size_t i = 0; i < (*table)->size; i++) {
            if((*table)->table[i] != NULL){
                delete_bucket_list(&(*table)->table[i]);
                free((*table)->table[i]);
                (*table)->table[i] = NULL;
            }
        }

        free((*table)->table);
        free(*table);
        table = NULL;
    }


}

//======================================================================


static void delete_bucket_list(bucket_t **head)
{

    if (head != NULL) {

        while (*head != NULL) {
            bucket_t *next = (*head)->next;
            free_const_ptr((*head)->pair.key);
            free_const_ptr((*head)->pair.value);
            free(*head);
            *head = next;
        }
    }

}

//======================================================================

kv_list_t *get_Htable_content(Htable_t table)
{

    if (table == NULL) {
        fprintf(stderr, "Received hashtable in get_Htable_content was null\n");
        return NULL;
    }

    kv_list_t *list = malloc(sizeof(kv_list_t));
    kv_pair_t *pairs = calloc(table->nbr_elems, sizeof(kv_pair_t));

    if (list == NULL || pairs == NULL) {
        fprintf(stderr, "Could not allocate memory for the new list in get_Htable_content\n");
        free(list);
        free(pairs);
        return NULL;
    }

    list->size = table->nbr_elems;
    size_t index = 0;

    //loop through all elements of the hashtable
    for (size_t i = 0; i < table->size; ++i) {
        if (table->table[i] != NULL) {
            //go through the linked list and add the pair to the new list
            for (bucket_t *b = table->table[i]; b != NULL; b = b->next) {
                kv_pair_t newPair = {strdup(b->pair.key), strdup(b->pair.value)};
                pairs[index++] = newPair;
            }
        }
    }

    list->pairs = pairs;

    return list;

}

//======================================================================

void kv_pair_free(kv_pair_t *kv)
{
    if (kv != NULL) {
        free_const_ptr(kv->key);
        free_const_ptr(kv->value);
        kv->key = NULL;
        kv->value = NULL;
    }
}

//======================================================================

void kv_list_free(kv_list_t *list)
{
    for (size_t i = 0; i < list->size; ++i) {
        kv_pair_free(&list->pairs[i]);
    }

    free(list->pairs);
    list->pairs = NULL;
    free(list);
    list = NULL;

}

//======================================================================

kv_list_t *kv_list_init()
{
    kv_list_t *pair = calloc(1, sizeof(kv_list_t));

    if(pair != NULL) {
        pair -> size = 0;
        pair -> pairs = NULL;

        return pair;
    }

    //On error
    return NULL;
}

//======================================================================

error_code kv_pair_add (kv_list_t *list, kv_pair_t *elem)
{
    if(list == NULL || elem == NULL) {
        return ERR_BAD_PARAMETER;
    }

    list-> pairs = realloc(list-> pairs, (list-> size + 1) * sizeof(kv_pair_t));

    if(list->pairs == NULL){
        kv_list_free(list);
        fprintf(stderr, "Could not realloc memory in kv_pair_add.\n");
        return ERR_NOMEM;
    }

    (list-> size)++;

    list -> pairs[list -> size - 1].key = strdup(elem -> key);
    list -> pairs[list -> size - 1].value = strdup(elem -> value);

    return ERR_NONE;
}


