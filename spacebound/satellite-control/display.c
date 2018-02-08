#include <stdint.h>
#include <stdlib.h>
#include <curses.h>
#include <menu.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 4

typedef struct Display_Menu {
  ITEM ** items;
  MENU *  menu;
  int n_choices;
} Display_Menu;

Display_Menu * main_menu;

void nothing() {};

void create_display() {

  // Allocate memory for the main menu
  main_menu = malloc(sizeof(Display_Menu));

  // Setup ncurses
  initscr();
  use_default_colors();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_MAGENTA, COLOR_BLACK);


  // Allocate clean space for items
  int8_t n_choices = 3;
  ITEM ** items = (ITEM **) calloc(n_choices + 1, sizeof(ITEM *));

  // Create each menu item
  items[0] = new_item("Cubesat Configuration", NULL);
  items[1] = new_item("Sensor Plots", NULL);
  items[2] = new_item("Exit", NULL);
  items[n_choices] = (ITEM *) NULL;

  // Set function pointer bindings for menu items
  set_item_userptr(items[0], nothing);
  set_item_userptr(items[1], nothing);
  set_item_userptr(items[2], nothing);

  // Create menu
  MENU * menu = new_menu((ITEM **) items);

  // Set structure components
  main_menu -> menu = menu;
  main_menu -> items = items;
  main_menu -> n_choices = n_choices;

  // Print static messages on screen
  mvprintw(LINES - 3, 0, "Press <ENTER> to see the option selected");
  mvprintw(LINES - 2, 0, "Up and Down arrow keys to naviage (F1 to Exit)");

  // Show the menu
  post_menu(menu);
  refresh();
}

void terminate_display() {
  unpost_menu(main_menu -> menu);
  for (int i = 0; i < main_menu -> n_choices; ++i) free_item(main_menu -> items[i]);
  free_menu(main_menu -> menu);
  endwin();
}

int main() {

  create_display();
  
  int key_code;
  
  while ((key_code = getch()) != KEY_LEFT) {
    switch(key_code) {
    case KEY_DOWN:
      menu_driver(main_menu -> menu, REQ_DOWN_ITEM);
      break;
    case KEY_UP:
      menu_driver(main_menu -> menu, REQ_UP_ITEM);
      break;
    case 10: /* Enter */
      {
	ITEM *cur;
	void (*p)(char *);
	
	cur = current_item(main_menu -> menu);
	p = item_userptr(cur);
	p((char *) item_name(cur));
	pos_menu_cursor(main_menu -> menu);
	break;
      }
      break;
    }
  }

  terminate_display();
}
