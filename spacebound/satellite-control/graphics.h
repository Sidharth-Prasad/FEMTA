#ifndef HEADER_GAURD_GRAPHICS
#define HEADER_GAURD_GRAPHICS

#include <stdbool.h>
#include <curses.h>
#include <menu.h>

#include "linked-list.h"

#define PRESENT_NORMAL 0
#define PRESENT_GRAPH  1

#define GENERAL_WINDOW 0
#define CONTROL_WINDOW 1
#define OPERATE_WINDOW 2

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

typedef struct View {

  WINDOW * window;

  uchar inner_width;
  uchar inner_height;

  uchar outer_width;
  uchar outer_height;
  
} View;

typedef struct Plot {

  char * name;
  
  List ** lists;
  uchar number_of_lists;
  float min_value;
  float max_value;
  bool has_data;
  
} Plot;

typedef struct print_view {

  View * view;

  List * lines;
  List * colors;

  uchar number_lines_printed;
  uchar current_view_line;
  uchar number_of_lines;
  
} print_view;

typedef struct graph_view {

  View * view;
  uchar vertical_tick_marks;
  uchar horizontal_tick_marks;
  
} graph_view;

typedef struct setup_view {

  View * view;
  
} setup_view;

// Views shown to the user
print_view ** print_views;
graph_view ** graph_views;
setup_view ** setup_views;

void initialize_graphics();
void terminate_graphics();
void print(uchar window_number, char * string, unsigned int color);
void stomp_printer(uchar window_number, char * string, unsigned int color);
void clear_print_window(uchar window_number);
void erase_print_window(uchar window_number);
void update_state_graphic(uchar line, bool state);

void graph_plot(Plot * plot);
void plot_add_value(Plot * plot, List * list, Node * node);

void switch_to_full_graph(void * graph);    // For selector
void switch_to_normal(void * nil);          // ------------
void clear_and_redraw(void * nil);          // ------------


Plot * create_plot(char * name, uchar number_of_lists);

uchar presentation_mode;    // What are we showing

uchar number_of_data_points_plottable;  // = 0

Plot *  graph_owner;              // The stream in control of the plot area
Plot ** all_possible_owners;      // All possible plots that could be in control
List *  owner_index_list;         // DLL of owners for the plot area
Node *  graph_owner_index_node;   // Used for indexing owners array

#endif
