#ifndef HEADER_GAURD_GRAPHICS
#define HEADER_GAURD_GRAPHICS

#include <stdbool.h>
#include <curses.h>
#include <menu.h>

#include "linked-list.h"

typedef struct View {

  WINDOW * window;

  unsigned char inner_width;
  unsigned char inner_height;

  unsigned char outer_width;
  unsigned char outer_height;
  
} View;

typedef struct Plot {

  char * name;
  
  List ** lists;
  unsigned char number_of_lists;
  float min_value;
  float max_value;
  bool has_data;
  
} Plot;

typedef struct print_view {

  View * view;

  List * lines;
  List * colors;

  unsigned char number_lines_printed;
  unsigned char current_view_line;
  unsigned char number_of_lines;
  
} print_view;

typedef struct graph_view {

  View * view;
  unsigned char vertical_tick_marks;
  unsigned char horizontal_tick_marks;
  
} graph_view;

typedef struct setup_view {

  View * view;
  
} setup_view;

void initialize_graphics();
void terminate_graphics();
void print(unsigned char window_number, char * string, unsigned int color);
void clear_print_window(unsigned char window_number);
void erase_print_window(unsigned char window_number);
void update_state_graphic(unsigned char line, bool state);

void graph_plot(Plot * plot);
void plot_add_value(Plot * plot, List * list, Node * node);

Plot * create_plot(char * name, unsigned char number_of_lists);

unsigned char number_of_data_points_plottable;
Plot * graph_owner;

#endif
