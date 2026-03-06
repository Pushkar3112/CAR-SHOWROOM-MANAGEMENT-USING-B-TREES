/*
 * hash_analytics.c
 *
 * Implements a memory-based String Hash Map to aggregate car sales data.
 */

#include "hash_analytics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Hash function computing DJB2 hash algorithm */
static unsigned int hash_string(const char *str)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash % HASH_TABLE_SIZE;
}

HashTable *ht_create(void)
{
    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
    if (!ht) return NULL;
    
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        ht->buckets[i] = NULL;
    }
    ht->total_entries = 0;
    return ht;
}

void ht_increment_car(HashTable *ht, const char *car_name)
{
    if (!ht || !car_name) return;

    unsigned int idx = hash_string(car_name);
    HashNode *current = ht->buckets[idx];

    /* Search for existing car entry */
    while (current != NULL) {
        if (strcmp(current->car_name, car_name) == 0) {
            current->sales_count++;
            return;
        }
        current = current->next;
    }

    /* Not found, add new entry at the head of the bucket */
    HashNode *new_node = (HashNode *)malloc(sizeof(HashNode));
    if (!new_node) return;

    strncpy(new_node->car_name, car_name, 49);
    new_node->car_name[49] = '\0';
    new_node->sales_count = 1;
    new_node->next = ht->buckets[idx];

    ht->buckets[idx] = new_node;
    ht->total_entries++;
}

int ht_get_count(HashTable *ht, const char *car_name)
{
    if (!ht || !car_name) return 0;

    unsigned int idx = hash_string(car_name);
    HashNode *current = ht->buckets[idx];

    while (current != NULL) {
        if (strcmp(current->car_name, car_name) == 0) {
            return current->sales_count;
        }
        current = current->next;
    }
    return 0;
}

int ht_get_most_popular(HashTable *ht, char *popular_name_out)
{
    if (!ht || !popular_name_out) return 0;

    int max_count = 0;
    popular_name_out[0] = '\0';

    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode *current = ht->buckets[i];
        while (current != NULL) {
            if (current->sales_count > max_count) {
                max_count = current->sales_count;
                strcpy(popular_name_out, current->car_name);
            }
            current = current->next;
        }
    }

    return max_count;
}

void ht_destroy(HashTable *ht)
{
    if (!ht) return;

    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode *current = ht->buckets[i];
        while (current != NULL) {
            HashNode *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht);
}
