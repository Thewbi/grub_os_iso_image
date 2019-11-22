#ifndef LINKED_LIST
#define LINKED_LIST

#include "../heap.h"
#include "../types.h"

typedef struct linked_list_item_t {
  struct linked_list_item_t *prev;
  struct linked_list_item_t *next;
  void * payload;
} linked_list_item_t;

/*
USAGE:

// define a pointer for the list
linked_list_item_t *linked_list = NULL;

// add an entry to the end of the list
if (add_to_list(&linked_list, <payload>) < 0) {
  k_printf("add failed!\n");
}

// iterate over the list and specify a function pointer
iterate_over_list(linked_list, &output_list_item);

// delete the list
delete_list(&linked_list);
*/

/**
 * @brief Adds a new entry to the end of the list. 
 * 
 * The list does not own 
 * the pointer to the payload! When deleting the list the payload pointer and the 
 * data it points to are left unchanged! You have to manage the memory yourself!
 * It the list is the sole pointer to the payload, you will create a memory leak
 * by deleting the list without deleting the payload!
 * 
 * @param linked_list 
 * @param payload 
 * @return int 
 */
int add_to_list(linked_list_item_t ** linked_list, void * payload);

/**
 * @brief Deletes a list by deleting all the entries on the list.
 * 
 * The data behind the payload pointers is NOT deleted because the list does not own them!
 * 
 * @param linked_list 
 */
void delete_list(linked_list_item_t ** linked_list);

/**
 * @brief Outputs the length of the list which is the amount of items in the list.
 * 
 * @param linked_list 
 * @return int 
 */
int length(linked_list_item_t * linked_list);

/**
 * @brief Iterates over all the items on the list by putting them into the function pointer.
 * 
 * @param linked_list 
 * @param functionPtr 
 */
void iterate_over_list(linked_list_item_t * linked_list, void (*functionPtr)(linked_list_item_t *));

#endif