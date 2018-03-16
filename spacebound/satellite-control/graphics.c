
#include <pigpio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <menu.h>

#include "graphics.h"
#include "unified-controller.h"

// Unified Controller Macros
#define NUMBER_OF_MODULES 4
#define I2C_STATE 2
#define UART_STATE 3

// Graphical Macros
#define NUMBER_OF_PRINT_VIEWS 3
#define NUMBER_OF_GRAPH_VIEWS 1
#define NUMBER_OF_SETUP_VIEWS 1

void print_window_title();

print_view ** print_views;
graph_view ** graph_views;
setup_view ** setup_views;

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
  view -> outer_height = 12;
  view -> inner_width  = view -> outer_width  - 2;
  view -> inner_height = view -> outer_height - 4;

  view -> window = newwin(view -> outer_height, view -> outer_width, 0, 0);
  box(view -> window, 0, 0);
  mvwaddch(view -> window, 0, view -> outer_width - 1, ACS_TTEE);
  mvwaddch(view -> window, 2, view -> outer_width - 1, ACS_PLUS);
  mvwaddch(view -> window, view -> outer_height - 1, view -> outer_width - 1, ACS_RTEE);

  print_window_title(view -> window, 1, 0, view -> outer_width, "System log", COLOR_PAIR(3));
  mvwaddch(view -> window, 2, 0, ACS_LTEE);
  mvwhline(view -> window, 2, 1, ACS_HLINE, view -> outer_width - 2);
  mvwaddch(view -> window, 2, view -> outer_width - 1, ACS_RTEE);

  refresh();
  wrefresh(view -> window);


  // Draw a printf window
  view = print_views[2] -> view;
  view -> outer_width  = COLS - setup_views[0] -> view -> outer_width - print_views[1] -> view -> outer_width + 2;
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

  print_window_title(view -> window, 1, 0, view -> outer_width, "Real-time Graphs", COLOR_PAIR(7));
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
  temp = (width - length)/ 2;
  x = startx + (int) temp;
  wattron(win, color);
  mvwprintw(win, y, x, "%s", string);
  wattroff(win, color);
  refresh();
}

void print(unsigned char window_number, char * string, unsigned char color) {
  
}
