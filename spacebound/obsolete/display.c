#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <panel.h>
#include <menu.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 4

typedef struct Display_Menu {
  ITEM ** items;
  MENU *  menu;
  int n_choices;
} Display_Menu;

typedef struct Display_Window {
  PANEL * panel;
  WINDOW * window;
  MENU * menu;
} Display_Window;

Display_Menu * main_menu;
Display_Window * main_window;

Display_Window * left_windows;

Display_Window * focus_display;

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color) {
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

void open_large_window();
void terminate_display();

ITEM ** create_items(char ** item_names, int number) {

  ITEM ** items = (ITEM **) calloc(number + 1, sizeof(ITEM *));

  for (int8_t i = 0; i < number; i++) items[i] = new_item(item_names[i], NULL);
  items[number] = (ITEM *) NULL;
  
  return items;
}


void change_display_mode(char * selection) {
  if (strcmp(selection, "Temperature Plots") == 0) {
    show_panel(left_windows[0].panel);
    update_panels();
    focus_display = &left_windows[0];
  }
}

void reset_display_mode() {
  hide_panel(left_windows[0].panel);
  update_panels();
  focus_display = main_window;
}

void create_main_window() {

  // Allocate clean space for items
  int8_t n_choices = 5;

  char * item_names[] = {
    "Temperature Plots",
    "Cubesat Configuration",
    "Compilation Tree",
    "Miscellaneous",
    "Exit",
  };
  
  ITEM ** items = create_items(item_names, n_choices);

  // Set function pointer bindings for menu items
  for (int i = 0; i < 4; i++) set_item_userptr(items[i], change_display_mode);
  set_item_userptr(items[4], terminate_display);
  
  // Create menu
  MENU * menu = new_menu((ITEM **) items);

  WINDOW * window = newwin(9, 29, 1, 2);
  keypad(window, TRUE);

  set_menu_win(menu, window);
  set_menu_sub(menu, derwin(window, 6, 27, 3, 1));

  set_menu_mark(menu, "   ");

  box(window, 0, 0);
  print_in_middle(window, 1, 0, 30, "Main Menu", COLOR_PAIR(2));
  mvwaddch(window, 2, 0,  ACS_LTEE);
  mvwhline(window, 2, 1,  ACS_HLINE, 27);
  mvwaddch(window, 2, 28, ACS_RTEE);

  // Set structure components
  main_menu -> menu  = menu;
  main_menu -> items = items;
  main_menu -> n_choices = n_choices;
  
  main_window -> window = window;
  main_window -> menu = menu;
  
  // Print static messages on screen
  mvprintw(LINES - 3, 2, "<Enter> to make a selection");
  mvprintw(LINES - 2, 2, "Use arrow keys to navigate");

  // Show the menu
  refresh();
  post_menu(main_menu -> menu);
  wrefresh(main_window -> window);
}

void create_left_windows() {
  
  char * item_names[] = {
    "Raspberry Pi CPU     ",
    "MPU 9250",
    "BNO 055",
    "Back",
  };

  int8_t n_choices = 4;  
  ITEM ** items = create_items(item_names, n_choices);
  
  // Set function pointer bindings for menu items
  for (int i = 0; i < n_choices; i++) set_item_userptr(items[i], change_display_mode);
  set_item_userptr(items[n_choices], reset_display_mode);

  MENU * menu = new_menu((ITEM **) items);
  WINDOW * window = newwin(LINES - 12, 29, 11, 2);
  PANEL * panel = new_panel(window);
  keypad(window, TRUE);

  set_menu_win(menu, window);
  set_menu_sub(menu, derwin(window, 6, 27, 3, 1));

  set_menu_mark(menu, "   ");

  box(window, 0, 0);
  print_in_middle(window, 1, 0, 29, item_names[0], COLOR_PAIR(2));
  mvwaddch(window, 2, 0,  ACS_LTEE);
  mvwhline(window, 2, 1,  ACS_HLINE, 27);
  mvwaddch(window, 2, 28, ACS_RTEE);
  
  left_windows[0].window = window;
  left_windows[0].panel = panel;
  left_windows[0].menu = menu;

  //refresh();
  post_menu(menu);
  //wrefresh(window);
  //update_panels();
  hide_panel(panel);
  //doupdate();
  update_panels();
  //show_panel(panel);
  //update_panels();
}

/*void open_large_window(char * selection) {
    
  WINDOW * window = newwin(LINES - 2, COLS - 35, 1, 33);
  keypad(window, TRUE);

  
  box(window, 0, 0);
  print_in_middle(window, 1, 0, COLS - 35, selection, COLOR_PAIR(2));
  mvwaddch(window, 2, 0,  ACS_LTEE);
  mvwhline(window, 2, 1,  ACS_HLINE, COLS - 37);
  mvwaddch(window, 2, COLS - 36, ACS_RTEE);


  refresh();
  wrefresh(window);
  }*/


void terminate_display() {
  unpost_menu(main_menu -> menu);
  for (int i = 0; i < main_menu -> n_choices; ++i) free_item(main_menu -> items[i]);
  free_menu(main_menu -> menu);
  free(main_menu); main_menu = NULL;
  endwin();
}

int main() {

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

  
  // Allocate memory for the main display
  main_menu   = malloc(sizeof(Display_Menu));
  main_window = malloc(sizeof(Display_Window));

  create_main_window();

  focus_display = main_window;
  
  left_windows = malloc((main_menu -> n_choices) * sizeof(Display_Window *));
  create_left_windows();
  
  int key_code;
  
  while (main_menu != NULL) {
    key_code = wgetch(focus_display -> window);
    switch(key_code) {
      
    case KEY_DOWN:
      menu_driver(focus_display -> menu, REQ_DOWN_ITEM);
      break;

    case KEY_UP:
      menu_driver(focus_display -> menu, REQ_UP_ITEM);
      break;

    case 10: /* Enter */
      {
	ITEM *cur;
	void (*p)(char *);
      
	cur = current_item(focus_display -> menu);
	p = item_userptr(cur);
	p((char *) item_name(cur));
	//if (main_menu == NULL) break;
	pos_menu_cursor(focus_display -> menu);
	break;
      }

      break;
    }
  }

  terminate_display();
}
