

#include <stdlib.h>
#include <stdio.h>

#include <curses.h>
#include <string.h>


char * plot_title;


int main() {

  plot_title = "No data";
  
  
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
  
  erase();

  attron(COLOR_PAIR(1));
  mvaddstr(0, 0, "Sensor Overview");
  mvprintw(0, COLS / 2 - 16, "%s", plot_title);
  mvaddstr(LINES - 3,  1, "Legend"  );
  mvaddstr(LINES - 2,  4,   "0th:"  );
  mvaddstr(LINES - 2, 10, ", 1st:"  );
  mvaddstr(LINES - 2, 18, ", 2nd:"  );
  mvaddstr(LINES - 1,  4,   "0&1:"  );
  mvaddstr(LINES - 1, 10, ", 0&2:"  );
  mvaddstr(LINES - 1, 18, ", 1&2:"  );
  mvaddstr(LINES - 1, 26, ", all: *");

  //mvaddstr(LINES - 3, COLS - 39, "Controls");
  //mvaddstr(LINES - 2, COLS - 36, "c: cycle graph");
  //mvaddstr(LINES - 2, COLS - 18, "r: repair graph");
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
  

  char input[1024];
  
  for (;;) {
    gets(input);
    
    if (!strcmp(input, "quit")) break;
    
    
  }

  
  endwin();
}
