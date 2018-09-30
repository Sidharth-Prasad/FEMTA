

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pigpio.h>
#include <curses.h>
#include <menu.h>

#include "graphics.h"
#include "femta.h"

// Unified Controller Macros
#define NUMBER_OF_MODULES 4
#define I2C_STATE 2
#define UART_STATE 3

// Graphical Macros
#define NUMBER_OF_PRINT_VIEWS 3
#define NUMBER_OF_GRAPH_VIEWS 1
#define NUMBER_OF_SETUP_VIEWS 1

bool ready_to_graph = false;

void print_window_title();

void initialize_graphics() {

  // Setup ncurses
  initscr();
  use_default_colors();
  start_color();
  curs_set(0);
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  // Pair ncurses colors
  init_pair(1, -1, -1);
  init_pair(2, COLOR_GREEN,   -1);
  init_pair(3, COLOR_MAGENTA, -1);
  init_pair(4, COLOR_RED,     -1);
  init_pair(5, COLOR_YELLOW,  -1);
  init_pair(6, COLOR_BLUE,  -1);
  init_pair(7, COLOR_CYAN,  -1);
  init_pair(8, COLOR_BLACK, COLOR_YELLOW);

  // General memory allocation
  print_views = malloc(NUMBER_OF_PRINT_VIEWS * sizeof(print_view *));
  graph_views = malloc(NUMBER_OF_GRAPH_VIEWS * sizeof(graph_view *));
  setup_views = malloc(NUMBER_OF_SETUP_VIEWS * sizeof(setup_view *));
  
  for (unsigned char p = 0; p < NUMBER_OF_PRINT_VIEWS; p++) print_views[p] = malloc(sizeof(print_view));
  for (unsigned char g = 0; g < NUMBER_OF_GRAPH_VIEWS; g++) graph_views[g] = malloc(sizeof(graph_view));
  for (unsigned char s = 0; s < NUMBER_OF_SETUP_VIEWS; s++) setup_views[s] = malloc(sizeof(setup_view));

  for (unsigned char p = 0; p < NUMBER_OF_PRINT_VIEWS; p++) print_views[p] -> view = malloc(sizeof(View));
  for (unsigned char g = 0; g < NUMBER_OF_GRAPH_VIEWS; g++) graph_views[g] -> view = malloc(sizeof(View));
  for (unsigned char s = 0; s < NUMBER_OF_SETUP_VIEWS; s++) setup_views[s] -> view = malloc(sizeof(View));

  
  // Draw the System Congifuration window
  View * view = setup_views[0] -> view;
  view -> inner_width  = 40;
  view -> inner_height = 20;
  view -> outer_width  = view -> inner_width + 2;
  view -> outer_height = view -> inner_height + 4;
  
  view -> window = newwin(view -> outer_height, view -> outer_width, 0, COLS - view -> outer_width);
  box(view -> window, 0, 0);
  
  //keypad(window, TRUE);

  print_window_title(view -> window, 1, 0, view -> outer_width, "System Configuration", COLOR_PAIR(5));
  //mvwaddch(view -> window, 2, 0, ACS_PLUS);
  mvwhline(view -> window, 2, 1, ACS_HLINE, view -> outer_width - 2);
  mvwaddch(view -> window, 2, view -> outer_width - 1, ACS_RTEE);

  unsigned char line = 4;
  unsigned char offset = 10;

  wattron(view -> window, COLOR_PAIR(5));
  mvwprintw(view -> window, line++, offset, "logical   physical   state");
  wattroff(view -> window, COLOR_PAIR(5));
  
  for (char m = 0; m < NUMBER_OF_MODULES; m++) {
    if (! modules[m] -> enabled) wattron(view -> window, A_DIM);
    wattron(view -> window, COLOR_PAIR(5));
    mvwprintw(view -> window, line++, 2, modules[m] -> identifier);
    wattroff(view -> window, COLOR_PAIR(5));
    for (char p = 0; p < modules[m] -> n_pins; p++) {
      offset = 7;
      if (modules[m] -> pins[p].logical < 10) offset += 1;
      offset += 8;
      mvwprintw(view -> window, line, offset, "%d", modules[m] -> pins[p].logical);
      if (modules[m] -> pins[p].logical < 10) offset -= 1;
      
      if (modules[m] -> pins[p].physical < 10) offset += 1;
      offset += 11;
      mvwprintw(view -> window, line, offset, "%d", modules[m] -> pins[p].physical);
      if (modules[m] -> pins[p].physical < 10) offset -= 1;
      
      // print out the human-readable state
      offset += 7;
      if      (modules[m] -> pins[p].state == PI_INPUT)   mvwprintw(view -> window, line, offset, "Input" );
      else if (modules[m] -> pins[p].state == PI_OUTPUT)  mvwprintw(view -> window, line, offset, "Output");

      int color = 2;
      if (modules[m] -> initialized == false) color = 4;

      wattron(view -> window, COLOR_PAIR(color));
      if (modules[m] -> pins[p].state == I2C_STATE)  mvwprintw(view -> window, line, offset, "I2C"   );
      else if (modules[m] -> pins[p].state == UART_STATE) mvwprintw(view -> window, line, offset, "UART"  );
      wattroff(view -> window, COLOR_PAIR(color));
      line++;
    }
    line++;
    if (! modules[m] -> enabled) wattroff(view -> window, A_DIM);
  }
  
  refresh();
  wrefresh(setup_views[0] -> view -> window);

  // Draw a printf window
  view = print_views[0] -> view;
  view -> outer_width  = setup_views[0] -> view -> outer_width;
  view -> outer_height = LINES - setup_views[0] -> view -> outer_height;
  view -> inner_width  = view -> outer_width  - 2;
  view -> inner_height = view -> outer_height - 4;

  view -> window = newwin(view -> outer_height + 1,
			  view -> outer_width,
			  setup_views[0] -> view -> outer_height - 1,
			  COLS - view -> outer_width);
  box(view -> window, 0, 0);
  mvwaddch(view -> window, 0, 0, ACS_LTEE);
  mvwaddch(view -> window, 0, view -> outer_width - 1, ACS_RTEE);

  print_window_title(view -> window, 1, 0, view -> outer_width, "System log", COLOR_PAIR(3));
  mvwaddch(view -> window, 2, 0, ACS_LTEE);
  mvwhline(view -> window, 2, 1, ACS_HLINE, view -> outer_width - 2);
  mvwaddch(view -> window, 2, view -> outer_width - 1, ACS_RTEE);
  
  refresh();
  wrefresh(view -> window);

  
  // Draw a printf window
  view = print_views[1] -> view;
  view -> outer_width  = (COLS - setup_views[0] -> view -> outer_width) / 2 + 1;
  view -> outer_height = 14;//12
  view -> inner_width  = view -> outer_width  - 2;
  view -> inner_height = view -> outer_height - 4;

  print_views[1] -> number_of_lines = view -> inner_height;

  view -> window = newwin(view -> outer_height, view -> outer_width, 0, 0);
  box(view -> window, 0, 0);
  mvwaddch(view -> window, 0, view -> outer_width - 1, ACS_TTEE);
  mvwaddch(view -> window, 2, view -> outer_width - 1, ACS_PLUS);
  mvwaddch(view -> window, view -> outer_height - 1, view -> outer_width - 1, ACS_RTEE);

  print_window_title(view -> window, 1, 0, view -> outer_width, "Controls", COLOR_PAIR(5));
  mvwaddch(view -> window, 2, 0, ACS_LTEE);
  mvwhline(view -> window, 2, 1, ACS_HLINE, view -> outer_width - 2);
  mvwaddch(view -> window, 2, view -> outer_width - 1, ACS_RTEE);
  
  refresh();
  wrefresh(view -> window);


  // Draw a printf window
  view = print_views[2] -> view;
  view -> outer_width  = COLS - setup_views[0] -> view -> outer_width - print_views[1] -> view -> outer_width + 2;
  view -> outer_width += COLS % 2; // HERERERE
  view -> outer_height = print_views[1] -> view -> outer_height;
  view -> inner_width  = view -> outer_width  - 2;
  view -> inner_height = view -> outer_height - 4;

  view -> window = newwin(view -> outer_height,
			  print_views[1] -> view -> outer_width,
			  0,
			  print_views[1] -> view -> outer_width - 1);
  box(view -> window, 0, 0);
  mvwaddch(view -> window, 0, 0, ACS_TTEE);
  mvwaddch(view -> window, 0, view -> outer_width - 1, ACS_TTEE);
  mvwaddch(view -> window, view -> outer_height - 1, 0, ACS_BTEE);
  mvwaddch(view -> window, view -> outer_height - 1, view -> outer_width - 1, ACS_RTEE);

  print_window_title(view -> window, 1, 0, view -> outer_width, "Operating System Status", COLOR_PAIR(7));
  mvwaddch(view -> window, 2, 0, ACS_PLUS);
  mvwhline(view -> window, 2, 1, ACS_HLINE, view -> outer_width - 2);
  mvwaddch(view -> window, 2, view -> outer_width - 1, ACS_PLUS);

  refresh();
  wrefresh(view -> window);


  // Draw a graphing window
  view = graph_views[0] -> view;
  view -> outer_width  = COLS  - setup_views[0] -> view -> outer_width + 1;
  view -> outer_height = LINES - print_views[1] -> view -> outer_height + 1;
  view -> inner_width  = view -> outer_width  - 2;
  view -> inner_height = view -> outer_height - 4;

  view -> window = newwin(
			  view -> outer_height,
			  view -> outer_width,
			  print_views[1] -> view -> outer_height - 1,
			  0);
  box(view -> window, 0, 0);
  mvwaddch(view -> window, 0, 0, ACS_LTEE);
  mvwaddch(view -> window, 0, view -> outer_width - 1, ACS_RTEE);
  //mvwaddch(view -> window, 2, view -> outer_width - 1, ACS_PLUS);
  mvwaddch(view -> window, view -> outer_height - 1, view -> outer_width - 1, ACS_BTEE);

  print_window_title(view -> window, 1, 0, view -> outer_width, "Temperatures v.s. Time", COLOR_PAIR(7));
  mvwaddch(view -> window, 2, 0, ACS_LTEE);
  mvwhline(view -> window, 2, 1, ACS_HLINE, view -> outer_width - 2);
  mvwaddch(view -> window, 2, view -> outer_width - 1, ACS_RTEE);

  refresh();
  wrefresh(view -> window);

  // Instantiate printing lists of the proper sizes
  for (uint8_t p = 0; p < NUMBER_OF_PRINT_VIEWS; p++) {
    print_views[p] -> lines = create_list(print_views[p] -> view -> inner_height, true);    // DLL Ring
    print_views[p] -> colors = create_list(print_views[p] -> view -> inner_height, true);   // DLL Ring
  }
  
  // Let everyone know
  number_of_data_points_plottable = view -> inner_width - 2 - 6;
  graph_views[0] -> vertical_tick_marks = 7;
  graph_views[0] -> horizontal_tick_marks = 8;

  // Final window corrections
  mvwaddch(print_views[0] -> view -> window, 0, 0, ACS_LTEE);
  mvwaddch(print_views[0] -> view -> window, 2, 0, ACS_LTEE);

  refresh();
  wrefresh(print_views[0] -> view -> window);
  
  mvwaddch(print_views[2] -> view -> window, print_views[2] -> view -> outer_height - 1, 0, ACS_BTEE);
  
  refresh();
  wrefresh(print_views[2] -> view -> window);
  ready_to_graph = true;
  
  //print(2, "CPU   SPAWNED   SUCCESS", 2);

  print(2, "CPU    SPAWNED   SUCCESS", 2);
  print(2, "I2C    SPAWNED   SUCCESS", 2);
  
  /*
  if (i2c_device -> initialized)    print(2, "I2C   SPAWNED   SUCCESS", 2);
  else                              print(2, "I2C   FAILURE"          , 4);*/
  

  print(0, "5 Threads are running", 0);
  print(0, "  - MPU is being sampled at 10 Hz", 0);
  print(0, "  - CPU is being sampled at 1 Hz", 0);
  print(0, "Logging 3 files asynchronously", 0);
  //print(0, "The OS is running 21 threads", 0);
  //print(1, "c: cycle graphs"             , 0);
  //print(1, "m: manual control"           , 0);
  //print(1, "q: quit"                     , 0);
  
  //print(2, "The universe appears flat :)", 2);
  
  // Debug
  //print(0, "Run with valgrind for sad mode :(", 0);
  
}

void terminate_graphics() {
  endwin();
}

void print_window_title(WINDOW *win, int starty, int startx, int width, char *string, chtype color) {
  int length, x, y;
  float temp;

  if (win == NULL) win = stdscr;
  getyx(win, y, x);
  if (startx != 0) x = startx;
  if (starty != 0) y = starty;
  if (width == 0)  width = 80;

  length = strlen(string);
  temp = (width - length) / 2;
  x = startx + (int) temp;
  wattron(win, color);
  mvwprintw(win, y, x, "%s", string);
  wattroff(win, color);
  refresh();
}

Plot * create_plot(char * name, unsigned char number_of_lists) {

  Plot * plot = malloc(sizeof(Plot));
  plot -> name = name;
  plot -> has_data = false;
  plot -> number_of_lists = number_of_lists;
  plot -> lists = malloc(number_of_lists * sizeof(Plot *));
  
  for (unsigned char l = 0; l < number_of_lists; l++) {
    plot -> lists[l] = create_list(number_of_data_points_plottable, true);   // DLL Ring
  }
  return plot;
}

void clear_print_window(unsigned char window_number) {
  // Clears the print window by writing a space character to every location
  
  // Space buffer
  unsigned char line_length = print_views[window_number] -> view -> inner_width - 2;
  char spaces[line_length + 1];
  for (unsigned char x = 0; x < line_length; x++) spaces[x] = ' ';
  spaces[line_length] = '\0';

  // Print buffer over lines
  for (unsigned char l = 0; l < print_views[window_number] -> number_of_lines; l++) {
    mvwprintw(print_views[window_number] -> view -> window, 3 + l, 2, "%s", spaces);
  }
}

void print(unsigned char window_number, char * string, unsigned int color) {
  // Prints a string to the window provided
  
  if (window_number >= NUMBER_OF_PRINT_VIEWS) return;   // Ensure print view exists
  if (!ready_to_graph) return;

  clear_print_window(window_number);
  
  print_view * printer = print_views[window_number];

  int chars_to_print = printer -> view -> inner_width - 2;
  if (strlen(string) < chars_to_print) chars_to_print = strlen(string);
  
  list_insert(printer -> lines,  create_node((void *) strdup(string)));   // Consider leak possibility - LEAK
  list_insert(printer -> colors, create_node((void *) color ));           // Consider leak possibility

  //wattron(printer -> view -> window, COLOR_PAIR(color));
  Node * node = printer -> lines -> head -> prev;
  Node * lcolor = printer -> colors -> head -> prev;
  int i = 0;
  
  for (; node != printer -> lines -> head; node = node -> prev, lcolor = lcolor -> prev, i++) {
    wattron(printer -> view -> window, COLOR_PAIR((int) (lcolor -> value)));
    mvwprintw(printer -> view -> window, 3 + i, 2, "%s", (char *) (node -> value));
    wattroff(printer -> view -> window, COLOR_PAIR((int) (lcolor -> value)));

    // Clear the rest of the line
    for (short s = strlen((char *) (node -> value)) + 2; s < printer -> view -> inner_width - 2; s++) {
      mvwprintw(printer -> view -> window, 3 + i, s, " ");
    }
  }
  wattron(printer -> view -> window, COLOR_PAIR((int) (lcolor -> value)));
  mvwprintw(printer -> view -> window, 3 + i, 2, "%s", (char *) (node -> value));
  wattroff(printer -> view -> window, COLOR_PAIR((int) (lcolor -> value)));

  // Clear the rest of the last line
  for (short s = strlen((char *) (node -> value)) + 2; s < printer -> view -> inner_width - 2; s++) {
    mvwprintw(printer -> view -> window, 3 + i, s, " ");
  }

  // Seeks the front of the last line
  // This allows other functions to use wgetstr() without writing off the line
  mvwprintw(printer -> view -> window, 3 + i, 2, "");   
  //wattroff(printer -> view -> window, COLOR_PAIR(color));
  
  refresh();
  wrefresh(printer -> view -> window);
}

void erase_print_window(unsigned char window_number) {
  for (unsigned char l = 0; l < print_views[window_number] -> number_of_lines; l++) {
    print(window_number, "", 0);
  }
}

void update_state_graphic(unsigned char line, bool state) {
  
  if (state) wattron(setup_views[0] -> view -> window, COLOR_PAIR(8));

  mvwprintw(setup_views[0] -> view -> window, line, 33, "Output");
  
  if (state) wattroff(setup_views[0] -> view -> window, COLOR_PAIR(8));

  refresh();
  wrefresh(setup_views[0] -> view -> window);
}

void plot_add_value(Plot * plot, List * list, Node * node) {
  
  // track extrema
  if (plot -> has_data) {
    if (*(float *) &node -> value < plot -> min_value) plot -> min_value = *(float *) &node -> value;
    if (*(float *) &node -> value > plot -> max_value) plot -> max_value = *(float *) &node -> value;
  }

  // first datum
  else {
    
    // The graphics library might not be finished initializing at this point
    while (number_of_data_points_plottable == 0);   // Spin lock threads until library is set up

    // Update all lists in the plot to the initialized value
    for (unsigned char l = 0; l < plot -> number_of_lists; l++) {
      plot -> lists[l] -> elements_limit = number_of_data_points_plottable;
    }
    
    plot -> min_value = *(float *) &node -> value;
    plot -> max_value = *(float *) &node -> value;
    plot -> has_data = true;
  }
  list_insert(list, node);
}

void graph_plot(Plot * plot) {
  
  if (!ready_to_graph) return;                          // Ensure race condition doesn't slaughter the terminal
  if (plot -> max_value == plot -> min_value) return;   // Can't plot uniform data
  if (plot != graph_owner) return;                      // If not owner, don't plot
  
  graph_view * graph = graph_views[0];
  View * view = graph -> view;

  //printf("%s", plot -> name);
  print_window_title(view -> window, 1, 0, view -> outer_width, plot -> name, COLOR_PAIR(2));

  //return; // Shorting - SHORT
  
  int y_axis_position = view -> inner_width - 1 - plot -> lists[0] -> elements;
  if (y_axis_position < 7) y_axis_position = 7;

  // Clear the screen
  for (unsigned char r = 0; r < view -> inner_height; r++) {
    for (unsigned char c = 0; c < view -> inner_width; c++) {
      mvwprintw(view -> window, r + 3, c + 1, " ");
    }
  }
  
  mvwvline(view -> window, 3, y_axis_position, ACS_VLINE, view -> inner_height);
  
  float vertical_interval = (float) view -> inner_height / graph -> vertical_tick_marks;

  float vertical_range = plot -> max_value - plot -> min_value;
  float vertical_scale = (float) view -> inner_height / vertical_range;   // Single vertical space
  
  for (unsigned char c = 0; c < graph -> vertical_tick_marks; c++) {
    
    mvwprintw(view -> window,
	      view -> inner_height - c * vertical_interval + 2,
	      y_axis_position - 5, "%.2f",
	      plot -> min_value + vertical_range * ((float) c / graph -> vertical_tick_marks));
    
    mvwaddch(view -> window,
	     view -> inner_height - c * vertical_interval + 2,
	     y_axis_position,
	     ACS_PLUS);
  }
  
  int graph_color = 2;
  for (unsigned char l = 0; l < plot -> number_of_lists; l++) {
    wattron(view -> window, COLOR_PAIR(graph_color));
    List * list = plot -> lists[l];
    Node * node = list -> head;
    int horizontal_position = view -> inner_width;
    float p_height = view -> inner_height - view -> inner_height * ((*(float *) &node -> value) - plot -> min_value) / vertical_range + 2;
    if (p_height < 3) p_height = 3;
    mvwprintw(view -> window, p_height, horizontal_position--, "-");
    for (node = node -> next; node != list -> head; node = node -> next) {
      p_height = view -> inner_height - view -> inner_height * ((*(float *) &node -> value) - plot -> min_value) / vertical_range + 2;
      if (p_height < 3) p_height = 3;
      mvwprintw(view -> window, p_height, horizontal_position--, "-");
    }
    wattroff(view -> window, COLOR_PAIR(graph_color++));
  }
  
  // draw x-axis
  float x_axis_position = view -> inner_height - view -> inner_height * (0.0 - plot -> min_value) / vertical_range + 2;
  if (plot -> min_value < 0) {
    mvwhline(view -> window, x_axis_position, 2, ACS_HLINE, view -> inner_width - 2);
  }

  // draw x-axis tick marks
  int horizontal_interval = (number_of_data_points_plottable) / graph -> horizontal_tick_marks;
  
  for (unsigned char h = 0; h < graph -> horizontal_tick_marks; h++) {
    int tick_position = view -> inner_width - number_of_data_points_plottable - 1 + h * horizontal_interval;
    if (tick_position < y_axis_position) continue;
    mvwaddch(view -> window, x_axis_position, tick_position, ACS_PLUS);
  }

  mvwaddch(view -> window, x_axis_position, y_axis_position, ACS_PLUS);

  refresh();
  wrefresh(view -> window);
}
