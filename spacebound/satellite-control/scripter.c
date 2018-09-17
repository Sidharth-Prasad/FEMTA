
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hashmap.h"
#include "scripter.h"
#include "error.h"
#include "colors.h"



void initialize_scripter() {

  action_table = create_hashmap(8);
  
  
}

void define_script_action(char * symbol, lambda action) {
  
  action_table -> add(action_table, symbol, (void *) action);
  
}

void execute_script(char * filename) {
  FILE * file = fopen(filename, "r");
  
  if (!file) exit_printing("Could not read file\n", ERROR_OS_FAILURE);
  
  char * line = NULL;
  size_t length = 0;
  
  while (getline(&line, &length, file) != -1) {

    char instruction[64];
    
    char * seeker = line;
    char * command = instruction;
    
    for (; *seeker != ' '; seeker++) *command++ = *seeker;
    for (; *seeker == ' '; seeker++);
    *command = '\0';

    //printf("%c\n", *seeker);
    //printf("rest: %s\n", seeker);
    
    if (!strcmp("nothing\n", seeker)) {
      
      //printf("command: %s\n", instruction);
      
      ((lambda) action_table -> get(action_table, instruction))(NULL);
      //printf("\n");
    }
    else {
      //printf("command: %s\n", instruction);
      //printf("argument: %s", seeker);
      //((void (*)(void *))((lambda) action_table -> get(action_table, instruction)))((void *) seeker);
      (((lambda) action_table -> get(action_table, instruction)))((void *) seeker);
      //printf("\n");
    }
  }

  if (line) free(line);
  fclose(file);
}

