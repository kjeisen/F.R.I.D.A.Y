//
// Created by Andrew Bowie on 9/18/22.
//

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "stdbool.h"

/**
 * @file linked_list.h
 * @brief This file represents the functionality and structure of a linked list.
 * Any item added to this list, MUST contain the necessary data as defined by the ll_node type.
 */

/**
 * @brief The node used for all linked lists. Note that
 */
typedef struct linked_list_node_
{
    ///The pointer to the item we're storing.
    void *_item; //8 bytes
    ///The next node in the list.
    struct linked_list_node_ *_next; //8 bytes
} ll_node;

/**
 * @brief The main linked list structure.
 */
typedef struct linked_list_ {
    ///The size of the linked list.
    int _size;
    ///The maximum size of the linked list, set to -1 for infinite.
    int _max_size;
    ///A pointer to the sorting function.
    int (*sort_func)(void*, void*);
    ///The first node in the linked list.
    ll_node *_first;
    ///The second node in the linked list.
    ll_node *_last;
} linked_list;

/**
 * @brief Creates a new unbounded linked list. You should call @code destroy_list(list, 1) when finished with it.
 * @return a pointer to the new linked list.
 */
linked_list
*nl_unbounded(void);

/**
 * @brief Creates a new bounded linked list. You should call @code destroy_list(list, 1) when finished with it.
 * @return a pointer to the new linked list.
 */
linked_list
*nl_maxsize(int max_size);

/**
 * @brief Gets the first node in the linked list.
 *
 * @param list the list.
 * @return the first node in the list, or NULL.
 */
ll_node
*get_first_node(linked_list *list);

/**
 * @brief Gets the next node after the given node.
 *
 * @param node the node.
 * @return the next node, or NULL if no such node exists.
 */
ll_node
*next_node(ll_node *node);

/**
 * @brief Gets the item out of the given node.
 *
 * @param node the node.
 * @return the item.
 */
void
*get_item_node(ll_node *node);

/**
 * @brief Gets the size of the linked list.
 * @return the size of the linked list.
 */
int
list_size(linked_list *list);

/**
 * @brief Gets the item at the given index in the list, or returns NULL
 * if the index was invalid.
 * @param list the list.
 * @param index the index.
 * @return the item, or NULL.
 */
void *
get_item(linked_list *list, int index);

/**
 * @brief Destroys the linked list by freeing any memory associated with it.
 * @param list the list to destroy.
 * @param destroy_values 1 if the values should be freed as well.
 */
void
destroy_list(linked_list *list, int destroy_values);

/**
 * @brief A function for adding an unknown type to the list.
 * @param list the list to add to.
 * @param item the item to add.
 * @return 1 if successful, 0 if not.
 */
int
add_item(linked_list *list, void *item);

/**
 * @brief A function for adding an unknown type to the list.
 * @param list the list to add to.
 * @param index the index to add to.
 * @param item the item to add.
 * @return 1 if successful, 0 if not.
 */
int
add_item_index(linked_list *list, int index, void *item);

/**
 * @brief Removes an item from the linked list at the given index.
 * @param list the list to remove from.
 * @param index the index to remove at.
 * @return the item removed, or NULL if unsuccessful.
 */
void
remove_item(linked_list *list, int index);

/**
 * @brief The pointer of the item to remove.
 *
 * @param list the list to remove from.
 * @return 0 if removed, -1 if not found.
 */
int
remove_item_ptr(linked_list *list, void *item_ptr);

/**
 * @brief Removes an item from the linked list at the given index.
 * YOU are responsible for freeing this pointer if used!
 * @param list the list to remove from.
 * @param index the index to remove at.
 * @return the item removed, or NULL if unsuccessful.
 */
void
*remove_item_unsafe(linked_list *list, int index);

/**
 * @brief Sets the sorting function for this linked list.
 * @param list the list to set the func for.
 * @param sort_func the sort func for this list.
 */
void
set_sort_func(linked_list *list, int sort_func(void *, void *));

/**
 * Applies the given function to each item within the linked list.
 * @param list the list to apply to.
 * @param call the function to apply.
 */
void
for_each_il(linked_list *list, void call(void *node));

/**
 * @brief Clears the linked list.
 *
 * @param list the list to clear.
 */
void
ll_clear(linked_list *list);

/**
 * @brief Clears the linked list, optionally freeing the items as well.
 * @param list the list.
 */
void
ll_clear_free(linked_list *list, bool free_items);

#endif //LINKEDLIST_H
