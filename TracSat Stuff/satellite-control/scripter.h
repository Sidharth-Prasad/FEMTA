#ifndef COMPILER_GAURD_SCRIPTER
#define COMPILER_GAURD_SCRIPTER

#include "hashmap.h"
#include "types.h"

void initialize_scripter();
void define_script_action(char * symbol, lambda action);
void execute_script(char * filename);

Hashmap * action_table;

#endif
