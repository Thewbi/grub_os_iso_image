#include "linked_list.h"

int add_to_list(linked_list_item_t **linked_list, void *payload) {

  // try to allocate memory
  void *ptr = malloc(sizeof(linked_list_item_t));
  if (ptr == NULL) {
    return -1;
  }

  k_printf("Inserting 0x%x\n", ptr);

  // if the list is empty, add a single element
  if (*linked_list == NULL) {

    *linked_list = ptr;

    // link the element to itself
    (*linked_list)->next = *linked_list;
    (*linked_list)->prev = *linked_list;

    (*linked_list)->payload = payload;

    return 0;
  }

  // insert the payload into an element at the end of the list
  linked_list_item_t *new = ptr;

  new->next = (*linked_list);
  new->prev = (*linked_list)->prev;

  (*linked_list)->prev->next = new;
  (*linked_list)->prev = new;

  new->payload = payload;

  return 0;
}

void delete_list(linked_list_item_t **linked_list) {

  if (*linked_list == NULL) {
    return;
  }

  // if the list has a single element only, delete it and return
  if ((*linked_list)->next == (*linked_list)) {

    k_printf("Deleting 0x%x\n", *linked_list);

    free(*linked_list);
    (*linked_list) = NULL;

    return;
  }

  // iterate and delete
  linked_list_item_t *ptr = (*linked_list);
  ptr->prev->next = NULL;
  ptr->prev = NULL;

  while (ptr->next != NULL) {

    ptr = ptr->next;

    k_printf("Deleting 0x%x\n", ptr->prev);
    free(ptr->prev);
    ptr->prev = NULL;
  }

  k_printf("Deleting 0x%x\n", ptr);
  free(ptr);
  ptr = NULL;

  (*linked_list) = NULL;
}

int length(linked_list_item_t *linked_list) {

  if (linked_list == NULL) {
    return 0;
  }

  // go to the second list element and set the list length to 1
  linked_list_item_t *ptr = (linked_list)->next;
  int length = 1;

  // while not back at the first element, iterate and increment the length
  while (ptr != linked_list) {

    length++;
    ptr = ptr->next;
  }

  return length;
}

void iterate_over_list(linked_list_item_t *linked_list,
                       void (*functionPtr)(linked_list_item_t *)) {

  if (linked_list == NULL) {
    return;
  }

  // set the pointer to the second element
  linked_list_item_t *ptr = linked_list->next;

  // call the function for the first element
  functionPtr(linked_list);

  // while not back at the first element, iterate and call the function
  while (ptr != linked_list) {

    functionPtr(ptr);
    ptr = ptr->next;
  }
}