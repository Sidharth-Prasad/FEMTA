#ifndef HEADER_GAURD_FOR_COMPILER_UNIT_GRAPHICS
#define HEADER_GAURD_FOR_COMPILER_UNIT_GRAPHICS

#include <curses.h>
#include <menu.h>

typedef struct View {

  WINDOW * window;

  unsigned char inner_width;
  unsigned char inner_height;

  unsigned char outer_width;
  unsigned char outer_height;
  
} View;

typedef struct print_view {

  View * view;

  unsigned char number_lines_printed;
  unsigned char current_view_line;
  unsigned char number_of_lines;
  
} print_view;

typedef struct graph_view {

  View * view;
  
} graph_view;

typedef struct setup_view {

  View * view;
  
} setup_view;

void initialize_graphics();
void terminate_graphics();
void print(unsigned char window_number, char * string, unsigned char color);

#endif
