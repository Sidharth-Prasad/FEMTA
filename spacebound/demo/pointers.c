
//compile using gcc -o pointer pointers.c

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct node node;
typedef struct node {

  node * next;   // The next node in the list
  
  int value;     // The value of the node

  bool (* add_node)(node * start, int value);   // Method for adding node
  void (* print)(node * start);                 // Method for printing

} node;

// Print the list of nodes
void print_nodes(node * node) {

  printf("Nodes:\t");
  
  for (struct node * root = node; root != NULL; root = root -> next) {
    printf("%d\t", root -> value);
  }

  printf("\n");
}

// Fail to print the list
void fail_print(node * node) {
  printf("Sorry but I'm not the head\n");
}


bool append_to_end(node *, int);     // Compiler promise

node * create_node(int value) {
  
  node * new = malloc(sizeof(node));

  new -> next = NULL;
  new -> value = value;
  new -> print = fail_print;         // Set the print function pointer to failure
  new -> add_node = append_to_end;   // Set append function pointer

  return new;
}

// Adds a node to the end of the list
bool append_to_end(node * start, int value) {

  if (start == NULL) return false;   // Return false if node can't be appended

  struct node * root;                // The first node in the list

  for (root = start; root -> next != NULL; root = root -> next);   // Walk the list to the last node

  root -> next = create_node(value);   // Add a new node
  
  return true;
}

int main() {

  node * head = create_node(0);   // Create the head of the list
  head -> print = print_nodes;    // Set head function pointer to the print_nodes function

  // Add nodes into the list
  (head -> add_node)(head, 1);
  (head -> add_node)(head, 3);
  (head -> add_node)(head, 7);
  (head -> add_node)(head, 9);
  
  (head -> print)(head);         // Print the list
  
  (head -> next -> print)(head -> next);   // Fail to print
  (head -> next -> print)(head);           // Fail to print


  // Here is why we might do this /////////////////////////////////////////////////////////
  printf("\n");
  
  (head -> next -> next -> print)(head -> next -> next);
  
  head -> next -> next -> print = print_nodes;   // Allow the print function for node with value 3
  
  (head -> next -> next -> print)(head -> next -> next);   // This will print something wrong
}
