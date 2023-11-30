//
// Created by Andrew Bowie on 3/29/23.
//

#include "hash_map.h"
#include "memory.h"
#include "string.h"

///A node representing a tombstone.
static const hash_map_node_t TOMBSTONE_NODE = {0};
///The default size of the hash map.
static const int DEFAULT_CAPACITY = 16;

/**
 * @brief Double hashes the given key.
 * @param key the key to hash.
 * @return the double hash.
 */
int double_hash(hash_map_t *map, void *key)
{
    int hash = map->hash_func(key);
    return hash ^ ((hash >> 16) & 0xFFFF);
}

/**
 * Gets the correct index for the given hash.
 *
 * @param map the map.
 * @param hash the hash.
 * @return the proper index.
 */
int get_map_index(hash_map_t *map, int hash)
{
    int mod = hash % map->capacity;
    if((hash ^ map->capacity) < 0 && mod != 0)
        return mod + map->capacity;
    return mod;
}

/**
 * @brief Resizes the given map to the new size. Note that the map's new size MUCH be larger than its old.
 *
 * @param map the map.
 * @param new_size the new size of the map.
 */
void resize_map(hash_map_t *map, int new_size)
{
    hash_map_node_t **old_items = map->values;
    int old_capacity = map->capacity;

    size_t total_size = sizeof (hash_map_node_t *) * new_size;
    hash_map_node_t **new_items = sys_alloc_mem(total_size);
    memset(new_items, 0, total_size);

    int old_size = map->size;
    map->values = new_items;
    map->size = map->contamination = 0;
    map->capacity = new_size;
    if(old_size > 0)
    {
        //Put all the old values into the map.
        for (int i = 0; i < old_capacity; ++i)
        {
            hash_map_node_t *node = old_items[i];
            if(node == NULL || node == &TOMBSTONE_NODE)
                continue;

            put(map, node->key, node->value);
        }
    }
}

hash_map_t *new_map(bool (*equality_func)(void *value1, void *value2), int (*hash_func)(void *value))
{
    hash_map_t *allocated = sys_alloc_mem(sizeof (hash_map_t));
    if(allocated == NULL)
    {
        return NULL;
    }

    memset(allocated, 0, sizeof (hash_map_t));
    allocated->equality_func = equality_func;
    allocated->hash_func = hash_func;
    resize_map(allocated, DEFAULT_CAPACITY);
    return allocated;
}

void *put(hash_map_t *map, void *key, void *value)
{
    int hash_code = double_hash(map, key);
    int index = get_map_index(map, hash_code);

    //Try to find any collisions.
    int first_tombstone_index = -1;
    for (int i = 0; i < map->capacity; ++i)
    {
        //Get the real index via ASQP
        int real_index = get_map_index(map, index + (i * i) * (i % 2 == 0 ? -1 : 1));
        hash_map_node_t *node = map->values[real_index];

        //Check for an easy replacement.
        if(node == NULL)
        {
            //In this case, we need to create a new node.
            hash_map_node_t *new_node = sys_alloc_mem(sizeof (hash_map_node_t));
            memset(new_node, 0, sizeof (hash_map_node_t));
            new_node->hash_code = hash_code;
            new_node->key = key;
            new_node->value = value;
            map->values[real_index] = new_node;
            map->size++;
            map->contamination++;

            //Check if we should resize.
            if((float) map->contamination / (float) map->capacity > 0.75F)
            {
                int new_capacity = map->capacity * 2;
                resize_map(map, new_capacity);
            }
            return NULL;
        }

        if(node == &TOMBSTONE_NODE)
        {
            if(first_tombstone_index == -1)
                first_tombstone_index = real_index;
            continue;
        }

        //Check for a replacement.
        if(node->hash_code == hash_code && map->equality_func(node->key, key))
        {
            //We need to replace the first tombstone in this case.
            void *old_value = node->value;
            node->key = key;
            node->value = value;
            node->hash_code = hash_code;
            if(first_tombstone_index >= 0)
            {
                map->values[real_index] = (hash_map_node_t *) &TOMBSTONE_NODE;
                map->values[first_tombstone_index] = node;
            }
            return old_value;
        }
    }

    //This should never happen, in theory.
    return NULL;
}

void *get(hash_map_t *map, void *key)
{
    int hash_code = double_hash(map, key);
    int index = get_map_index(map, hash_code);

    //Loop through the map, trying to find the item.
    for (int i = 0; i < map->capacity; ++i)
    {
        //Get the index.
        int real_index = get_map_index(map, index + (i * i) * (i % 2 == 0 ? -1 : 1));
        hash_map_node_t *node = map->values[real_index];

        if(node == NULL)
            return NULL;

        if(node == &TOMBSTONE_NODE)
            continue;

        if(node->hash_code == hash_code && map->equality_func(node->key, key))
            return node->value;
    }
    return NULL;
}

bool contains_key(hash_map_t *map, void *key)
{
    return get(map, key) != NULL;
}

void clear(hash_map_t *map)
{
    clear_free(map, false, false);
}

void clear_free(hash_map_t *map, bool free_keys, bool free_values)
{
    for (int i = 0; i < map->capacity; ++i)
    {
        //Get the node and check if we should free it.
        hash_map_node_t *node = map->values[i];
        if(node == NULL || node == &TOMBSTONE_NODE)
            continue;

        if(free_keys)
            sys_free_mem(node->key);
        if(free_values)
            sys_free_mem(node->value);

        sys_free_mem(node);
    }

    map->size = 0;
    map->contamination = 0;
}