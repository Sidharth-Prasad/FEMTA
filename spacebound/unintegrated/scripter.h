#ifndef COMPILER_GAURD_SCRIPTER
#define COMPILER_GAURD_SCRIPTER

#include "hashmap.h"

Hashmap * action_table;

void initialize_scripter();


typedef void (*lambda)(void);

void define_script_action(char * symbol, lambda action);
void execute_script(char * filename);


#endif
