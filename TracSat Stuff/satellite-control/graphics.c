

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pigpio.h>
#include <curses.h>
#include <menu.h>

#include "graphics.h"
#include "femta.h"
#include "selector.h"

// Unified Controller Macros
#define NUMBER_OF_MODULES 7
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
  
  for (uchar p = 0; p < NUMBER_OF_PRINT_VIEWS; p++) print_views[p] = malloc(sizeof(print_view));
  for (uchar g = 0; g < NUMBER_OF_GRAPH_VIEWS; g++) graph_views[g] = malloc(sizeof(graph_view));
  for (uchar s = 0; s < NUMBER_OF_SETUP_VIEWS; s++) setup_views[s] = malloc(sizeof(setup_view));
  
  for (uchar p = 0; p < NUMBER_OF_PRINT_VIEWS; p++) print_views[p] -> view = malloc(sizeof(View));
  for (uchar g = 0; g < NUMBER_OF_GRAPH_VIEWS; g++) graph_views[g] -> view = malloc(sizeof(View));
  for (uchar s = 0; s < NUMBER_OF_SETUP_VIEWS; s++) setup_views[s] -> view = malloc(sizeof(View));

  
  // Let everyone know how wide the graph will be
  number_of_data_points_graphable = COLS - 14;

  
  // Draw the System Congifuration window
  View * view = setup_views[0] -> view;
  view -> inner_width  = 40;
  view -> inner_height = 22;
  view -> outer_width  = view -> inner_width + 2;
  view -> outer_height = view -> inner_height + 4;
  
  view -> window = newwin(view -> outer_height, view -> outer_width, 0, COLS - view -> outer_width);
  box(view -> window, 0, 0);
  
  //keypad(window, TRUE);

  print_window_title(view -> window, 1, 0, view -> outer_width, "System Configuration", COLOR_PAIR(5));
  //mvwaddch(view -> window, 2, 0, ACS_PLUS);
  mvwhline(view -> window, 2, 1, ACS_HLINE, view -> outer_width - 2);
  mvwaddch(view -> window, 2, view -> outer_width - 1, ACS_RTEE);

  uchar line = 4;
  uchar offset = 10;

  wattron(view -> window, COLOR_PAIR(5));
  mvwprintw(view -> window, line++, offset, "logical   physical   state");
  wattroff(view -> window, COLOR_PAIR(5));
  
  for (char m = 0; m < NUMBER_OF_MODULES; m++) {
    if (! modules[m] -> show_pins) continue;
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
    print_views[p] -> lines  = create_list(print_views[p] -> view -> inner_height, true, false);    // DLL Ring
    print_views[p] -> colors = create_list(print_views[p] -> view -> inner_height, true, false);   // DLL Ring
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
  
  if (!MPU -> initialized) {
    print(GENERAL_WINDOW, "MPU failed initialization", 4);
  }

  if (!MPRLS -> initialized) {
    print(GENERAL_WINDOW, "MPRLS failed initialization", 4);
  }
  
  if (MPU -> initialized || MPRLS -> initialized) {
    print(OPERATE_WINDOW, "I2C    SPAWNED   SUCCESS", 2);
    print(GENERAL_WINDOW, "I2C thread spawned", 2);
  }
  else {
    print(OPERATE_WINDOW, "I2C    FAILURE"             , 4);
    print(GENERAL_WINDOW, "I2C thread failed to spawn", 4);
  }
  

  //print(0, "5 Threads are running", 0);
  //print(0, "  - MPU is being sampled at 10 Hz", 0);
  //print(0, "  - CPU is being sampled at 1 Hz", 0);
  //print(0, "Logging 3 files asynchronously", 0);
  //print(0, "The OS is running 21 threads", 0);
  
  //print(2, "The universe appears flat :)", 2);
  
  // Debug
  //print(0, "Run with valgrind for sad mode :(", 0);
  
}

void terminate_graphics() {
  endwin();
}

void reprint_interface() {

  // Reprint configuration pane
  View * view = setup_views[0] -> view;
  box(view -> window, 0, 0);
  print_window_title(view -> window, 1, 0, view -> outer_width, "System Configuration", COLOR_PAIR(5));

  mvwhline(view -> window, 2, 1, ACS_HLINE, view -> outer_width - 2);
  mvwaddch(view -> window, 2, view -> outer_width - 1, ACS_RTEE);

  uchar line = 4;
  uchar offset = 10;

  wattron(view -> window, COLOR_PAIR(5));
  mvwprintw(view -> window, line++, offset, "logical   physical   state");
  wattroff(view -> window, COLOR_PAIR(5));
  
  for (char m = 0; m < NUMBER_OF_MODULES; m++) {
    if (! modules[m] -> show_pins) continue;
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

  // Reprint general window
  view = print_views[0] -> view;
  box(view -> window, 0, 0);
  mvwaddch(view -> window, 0, 0, ACS_LTEE);
  mvwaddch(view -> window, 0, view -> outer_width - 1, ACS_RTEE);

  print_window_title(view -> window, 1, 0, view -> outer_width, "System log", COLOR_PAIR(3));
  mvwaddch(view -> window, 2, 0, ACS_LTEE);
  mvwhline(view -> window, 2, 1, ACS_HLINE, view -> outer_width - 2);
  mvwaddch(view -> window, 2, view -> outer_width - 1, ACS_RTEE);
  
  refresh();
  wrefresh(view -> window);

  // Reprint control pane
  view = print_views[1] -> view;
  
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

  // Reprint pane
  view = print_views[2] -> view;

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

  // Reprint the graphing window
  view = graph_views[0] -> view;
  box(view -> window, 0, 0);
  mvwaddch(view -> window, 0, 0, ACS_LTEE);
  mvwaddch(view -> window, 0, view -> outer_width - 1, ACS_RTEE);
  mvwaddch(view -> window, view -> outer_height - 1, view -> outer_width - 1, ACS_BTEE);

  print_window_title(view -> window, 1, 0, view -> outer_width, "Temperatures v.s. Time", COLOR_PAIR(7));
  mvwaddch(view -> window, 2, 0, ACS_LTEE);
  mvwhline(view -> window, 2, 1, ACS_HLINE, view -> outer_width - 2);
  mvwaddch(view -> window, 2, view -> outer_width - 1, ACS_RTEE);

  refresh();
  wrefresh(view -> window);

  // Final window corrections
  mvwaddch(print_views[0] -> view -> window, 0, 0, ACS_LTEE);
  mvwaddch(print_views[0] -> view -> window, 2, 0, ACS_LTEE);
  refresh();
  wrefresh(print_views[0] -> view -> window);
  
  mvwaddch(print_views[2] -> view -> window, print_views[2] -> view -> outer_height - 1, 0, ACS_BTEE);
  
  refresh();
  wrefresh(print_views[2] -> view -> window);
}

void clear_and_redraw(void * nil) {

  Plot * old_owner = graph_owner;
  graph_owner = NULL;

  erase();
  refresh();

  graph_owner = old_owner;
  reprint_interface();
}

void switch_to_full_graph(void * graph) {

  visible_selector = (Selector *) graph;
  
  presentation_mode = PRESENT_GRAPH;
  
  erase();

  attron(COLOR_PAIR(1));
  mvaddstr(0, 0, "Sensor Overview");
  mvprintw(0, COLS / 2 - 16, "%s", graph_owner -> name);
  mvaddstr(LINES - 3,  1, "Legend"  );
  mvaddstr(LINES - 2,  4,   "0th:"  );
  mvaddstr(LINES - 2, 10, ", 1st:"  );
  mvaddstr(LINES - 2, 18, ", 2nd:"  );
  mvaddstr(LINES - 1,  4,   "0&1:"  );
  mvaddstr(LINES - 1, 10, ", 0&2:"  );
  mvaddstr(LINES - 1, 18, ", 1&2:"  );
  mvaddstr(LINES - 1, 26, ", all: *");

  mvaddstr(LINES - 3, COLS - 39, "Controls");
  mvaddstr(LINES - 2, COLS - 36, "c: cycle graph");
  mvaddstr(LINES - 2, COLS - 18, "r: repair graph");
  //mvaddstr(LINES - 2, COLS - 1 "f: full experiment");
  mvaddstr(LINES - 1, COLS - 36, "b: back");

  mvaddstr(0, COLS - 6, "FEMTA"  );
  attroff(COLOR_PAIR(1));

  attron(COLOR_PAIR(4));
  mvaddstr(LINES - 2, 9, "*");
  attroff(COLOR_PAIR(4));

  attron(COLOR_PAIR(2));
  mvaddstr(LINES - 2, 17, "*");
  attroff(COLOR_PAIR(2));

  attron(COLOR_PAIR(6));
  mvaddstr(LINES - 2, 25, "*");
  attroff(COLOR_PAIR(6));

  attron(COLOR_PAIR(5));
  mvaddstr(LINES - 1, 9, "*");
  attroff(COLOR_PAIR(5));

  attron(COLOR_PAIR(3));
  mvaddstr(LINES - 1, 17, "*");
  attroff(COLOR_PAIR(3));

  attron(COLOR_PAIR(7));
  mvaddstr(LINES - 1, 25, "*");
  attroff(COLOR_PAIR(7));
  
  refresh();
}

void switch_to_normal(void * nil) {

  presentation_mode = PRESENT_NORMAL;

  reprint_interface();
}

void print_window_title(WINDOW * win, int starty, int startx, int width, char * string, chtype color) {
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

Plot * create_plot(char * name, uchar number_of_lists, uint number_to_average) {

  Plot * plot = malloc(sizeof(Plot));
  plot -> name = name;
  plot -> has_data = false;
  plot -> number_of_lists = number_of_lists;

  plot -> number_to_average = number_to_average;
  plot -> next_averages = calloc(number_of_lists, sizeof(float));
  plot -> next_numbers  = calloc(number_of_lists, sizeof(float));
  
  plot -> lists    = malloc(number_of_lists * sizeof(List *));
  plot -> averages = malloc(number_of_lists * sizeof(List *));
  
  for (uchar l = 0; l < number_of_lists; l++) {
    plot -> lists[l]    = create_list(number_of_data_points_plottable, true, false);   // DLL Ring
    plot -> averages[l] = create_list(number_of_data_points_plottable, true, false);   // --------
  }
  return plot;
}

void clear_print_window(uchar window_number) {
  // Clears the print window by writing a space character to every location
  
  // Space buffer
  uchar line_length = print_views[window_number] -> view -> inner_width - 2;
  char spaces[line_length + 1];
  for (uchar x = 0; x < line_length; x++) spaces[x] = ' ';
  spaces[line_length] = '\0';

  // Print buffer over lines
  for (uchar l = 0; l < print_views[window_number] -> number_of_lines; l++) {
    mvwprintw(print_views[window_number] -> view -> window, 3 + l, 2, "%s", spaces);
  }
}

void print(uchar window_number, char * string, uint color) {
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

void erase_print_window(uchar window_number) {
  for (uchar l = 0; l < print_views[window_number] -> number_of_lines; l++) {
    print(window_number, "", 0);
  }
}

void stomp_printer(uchar window_number, char * string, uint color) {
  // Destroys and replaces the last line of the printer
  // This is useful for taking user input and printing over a blank line
  
  print_view * printer = print_views[window_number];

  printer -> lines  -> head -> value = (void *) strdup(string);   // Another leak
  printer -> colors -> head -> value = (void *) color;
}

void update_state_graphic(uchar line, bool state) {
  
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
    while (number_of_data_points_plottable == 0) {   // Spin lock threads until library is set up
      sleep(0);
    }

    // Update all lists in the plot to the initialized value
    for (uchar l = 0; l < plot -> number_of_lists; l++) {
      plot -> lists[l]    -> elements_limit = number_of_data_points_plottable;
      plot -> averages[l] -> elements_limit = number_of_data_points_graphable;
    }
    
    plot -> min_value = *(float *) &node -> value;
    plot -> max_value = *(float *) &node -> value;
    plot -> has_data = true;
  }

  // Insert for normal list
  list_insert(list, node);

  
  // Contribute to average for average list

  uchar index;
  for (index = 0; true; index++) {
    if (list == plot -> lists[index]) break;    // Get the right index for averaging
  }
  
  plot -> next_numbers[index]++;
  plot -> next_averages[index] += (*(float *) &node -> value);

  if (plot -> next_numbers[index] == plot -> number_to_average) {    

    float average = plot -> next_averages[index] / plot -> number_to_average;
    list_insert(plot -> averages[index], create_node((void *) *((int *) &average)));

    plot -> next_numbers[index] = 0;
    plot -> next_averages[index] = 0;

  }
}

void graph_plot(Plot * plot) {
  
  if (!ready_to_graph) return;                          // Ensure race condition doesn't slaughter the terminal
  if (plot -> max_value == plot -> min_value) return;   // Can't plot uniform data
  if (plot != graph_owner) return;                      // If not owner, don't plot
  
  if (presentation_mode != PRESENT_NORMAL) {
    // System is presenting full graph
    
    uchar points_available = number_of_data_points_graphable;
    for (uchar n = 0; n < plot -> number_of_lists; n++) {
      // Get min number of points
      
      if (plot -> averages[n] -> elements < points_available) {
	points_available = plot -> averages[n] -> elements;
      }
    }
    
    if (!points_available) {
      // No data, so bail out
      
      refresh();
      return;
    }
    
    // Get min and max averages
    float min_average = plot -> max_value;    // DANGEROUS
    float max_average = plot -> min_value;    // ---------
    for (uchar l = 0; l < plot -> number_of_lists; l++) {
      
      Node * node = (Node *) 0x1;    // Bypass first for-loop check
      
      for (; node && node != plot -> averages[l] -> head; node = node -> next) {

	if (node == (Node *) 0x1) node = plot -> averages[l] -> head;    // Bypass complete

	float average = (*(float *) &node -> value);

	if (average < min_average) min_average = average;
	if (average > max_average) max_average = average;
      }      
    }

    if (min_average == max_average) {
      // Uniform data, bail out
      
      refresh();
      return;
    }
    
    // Draw y labels
    for (uchar l = 2; l < LINES - 5; l++) {

      float left_label = l * (max_average - min_average) / (LINES - 5 - 2) + min_average;
      
      mvprintw(LINES - 4 - l, 2 + (left_label >= 0), "%.2f", left_label);
    }

    for (uchar y = 2; y < LINES - 5; y++) {
      for (uchar x = 10; x < COLS - 3; x++) {
	mvaddch(y, x, ' ');
      }
    }
    
    // Plot points    

    Node * nodes[plot -> number_of_lists];
    for (uchar n = 0; n < plot -> number_of_lists; n++) {
      nodes[n] = plot -> averages[n] -> head;
    }
    
    for (uchar c = 0; c < number_of_data_points_graphable; c++) {

      if (c > points_available - 1) break;
      
      //float left_label = 0 * (plot -> max_value - plot -> min_value) / (LINES - 5 - 2) + plot -> min_value;

      uchar heights[plot -> number_of_lists];
      for (uchar h = 0; h < plot -> number_of_lists; h++) {
	float relative = (*(float *) &nodes[h] -> value);

	//mvprintw(LINES - 5 + h, 8, "%.2f", relative);
	
	relative = (relative - min_average) / (max_average - min_average);
	//relative = 1;
	
	if (relative < 0) relative = 0;
	if (relative > 1) relative = 1;
	
        relative *= LINES - 8;

	heights[h] = (uchar) relative;
      }

      if (plot -> number_of_lists == 1) {
	attron(COLOR_PAIR(4));
	mvaddch(LINES - 6 - heights[0], 8 + number_of_data_points_graphable - c, '*');
	attroff(COLOR_PAIR(4));
      }
      if (plot -> number_of_lists == 3) {

	attron(COLOR_PAIR(4));
	mvaddch(LINES - 6 - heights[0], 8 + number_of_data_points_graphable - c, '*');
	attroff(COLOR_PAIR(4));

	attron(COLOR_PAIR(2));
	mvaddch(LINES - 6 - heights[1], 9 + number_of_data_points_graphable - c, '*');
	attroff(COLOR_PAIR(2));

	attron(COLOR_PAIR(6));
	mvaddch(LINES - 6 - heights[2], 9 + number_of_data_points_graphable - c, '*');
	attroff(COLOR_PAIR(6));

	if (heights[0] == heights[1]) {
	  attron(COLOR_PAIR(5));
	  mvaddch(LINES - 6 - heights[0], 9 + number_of_data_points_graphable - c, '*');
	  attroff(COLOR_PAIR(5));
	}

	if (heights[1] == heights[2]) {
	  attron(COLOR_PAIR(7));
	  mvaddch(LINES - 6 - heights[1], 9 + number_of_data_points_graphable - c, '*');
	  attroff(COLOR_PAIR(7));
	}

	if (heights[0] == heights[2]) {
	  attron(COLOR_PAIR(3));
	  mvaddch(LINES - 6 - heights[0], 9 + number_of_data_points_graphable - c, '*');
	  attroff(COLOR_PAIR(3));
	}

	if (heights[0] == heights[1] && heights[0] == heights[2]) {
	  attron(COLOR_PAIR(1));
	  mvaddch(LINES - 6 - heights[0], 9 + number_of_data_points_graphable - c, '*');
	  attroff(COLOR_PAIR(1));
	}
      }
      
      /*for (uchar h = 0; h < plot -> number_of_lists; h++) {
	mvprintw(LINES - 7 - heights[h], number_of_data_points_graphable - c, "*");
	}*/
      
      for (uchar n = 0; n < plot -> number_of_lists; n++) {
	nodes[n] = nodes[n] -> next;
      }
    }
    
    refresh();
    return;
  }
  
  graph_view * graph = graph_views[0];
  View * view = graph -> view;

  //printf("%s", plot -> name);
  print_window_title(view -> window, 1, 0, view -> outer_width, plot -> name, COLOR_PAIR(2));

  //return; // Shorting - SHORT
  
  int y_axis_position = view -> inner_width - 1 - plot -> lists[0] -> elements;
  if (y_axis_position < 7) y_axis_position = 7;

  // Clear the screen
  for (uchar r = 0; r < view -> inner_height; r++) {
    for (uchar c = 0; c < view -> inner_width; c++) {
      mvwprintw(view -> window, r + 3, c + 1, " ");
    }
  }
  
  mvwvline(view -> window, 3, y_axis_position, ACS_VLINE, view -> inner_height);
  
  float vertical_interval = (float) view -> inner_height / graph -> vertical_tick_marks;

  float vertical_range = plot -> max_value - plot -> min_value;
  float vertical_scale = (float) view -> inner_height / vertical_range;   // Single vertical space
  
  for (uchar c = 0; c < graph -> vertical_tick_marks; c++) {
    
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
  for (uchar l = 0; l < plot -> number_of_lists; l++) {
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
  
  for (uchar h = 0; h < graph -> horizontal_tick_marks; h++) {
    int tick_position = view -> inner_width - number_of_data_points_plottable - 1 + h * horizontal_interval;
    if (tick_position < y_axis_position) continue;
    mvwaddch(view -> window, x_axis_position, tick_position, ACS_PLUS);
  }

  mvwaddch(view -> window, x_axis_position, y_axis_position, ACS_PLUS);

  refresh();
  wrefresh(view -> window);
}
