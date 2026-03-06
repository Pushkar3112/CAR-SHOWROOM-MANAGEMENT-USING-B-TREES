/*
 * hash_analytics.h
 *
 * Implements a string-keyed Hash Table used specifically to compute
 * "model-wise popularity rankings" and aggregate sales metrics,
 * as required by the original project description.
 */

#ifndef HASH_ANALYTICS_H
#define HASH_ANALYTICS_H

#define HASH_TABLE_SIZE 1009 /* Prime number for good distribution */

/*
 * Hash node storing the car name (key) and the frequency (value)
 */
typedef struct HashNode {
    char car_name[50];
    int  sales_count;
    struct HashNode *next;
} HashNode;

/*
 * The Hash Table structure
 */
typedef struct {
    HashNode *buckets[HASH_TABLE_SIZE];
    int total_entries;
} HashTable;

/*
 * Create and initialize the hash table.
 */
HashTable *ht_create(void);

/*
 * Increment the sales count for a specific car model name.
 */
void ht_increment_car(HashTable *ht, const char *car_name);

/*
 * Retrieve the current sales count for a car model.
 */
int ht_get_count(HashTable *ht, const char *car_name);

/*
 * Finds the most popular car across all entries in the Hash Table.
 * Populates `popular_name_out` and returns the max sales count.
 */
int ht_get_most_popular(HashTable *ht, char *popular_name_out);

/*
 * Free all memory associated with the hash table.
 */
void ht_destroy(HashTable *ht);

#endif /* HASH_ANALYTICS_H */
