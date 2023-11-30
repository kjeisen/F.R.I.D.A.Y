//
// Created by Andrew Bowie on 3/29/23.
//

#ifndef F_R_I_D_A_Y_HASH_MAP_H
#define F_R_I_D_A_Y_HASH_MAP_H

#include "stdbool.h"

/**
 * @file hash_map.h
 * @brief The header file for the hash map structure.
 */

///The structure definition for holding a node in a hash map.
typedef struct {
    ///The hash m
    void *key;
    ///The value being held in this node.
    void *value;
    ///The hash code for the value being held.
    int hash_code;
} hash_map_node_t;

///The definition for the structure holding the hash map data.
typedef struct {
    ///The size of the hash map.
    int size;
    ///The total amount of elements + tombstones in the map.
    int contamination;
    ///The capacity of the hash map.
    int capacity;
    ///The function to use for equality checking for given values.
    bool (*equality_func)(void *value1, void *value2);
    ///The hash function to use for the values in this map.
    int (*hash_func)(void *value);

    ///The values we're holding in this map.
    hash_map_node_t **values;
} hash_map_t;

/**
 * @brief Creates a new hash map with the given equality and hash functions. These CANNOT be NULL!
 *
 * @param equality_func the equality function.
 * @param hash_func the hash function.
 * @return a pointer to the new map, which was dynamically allocated, or NULL if the given values were invalid (or heap is full).
 */
hash_map_t *new_map(bool (*equality_func)(void *value1, void *value2), int (*hash_func)(void *value));

/**
 * @brief Puts the given item into the map, returning the old item if it is contained.
 *
 * @param map the map to put it into.
 * @param item the item to place into this map.
 * @return the old value or NULL.
 */
void *put(hash_map_t *map, void *key, void *value);

/**
 * @brief Gets the value out of the hash map with the given key.
 *
 * @param map the map.
 * @param key the key stored in the map.
 * @return the old value or NULL.
 */
void *get(hash_map_t *map, void *key);

/**
 * @brief Checks if the map contains the given key.
 *
 * @param map the map.
 * @param key the key.
 * @return true if the key is in the map.
 */
bool contains_key(hash_map_t *map, void *key);

/**
 * Clears the map, freeing all nodes (NOT THE ITEMS INSIDE THE NODES).
 * @param map the map to clear.
 */
void clear(hash_map_t *map);

/**
 * Clears the map, freeing all nodes and the items they're holding.
 *
 * @param map the map to clear.
 * @param free_keys if we should free the keys associated with the map.
 * @param free_values if we should free the values associated with the map.
 */
void clear_free(hash_map_t *map, bool free_keys, bool free_values);

#endif //F_R_I_D_A_Y_HASH_MAP_H
