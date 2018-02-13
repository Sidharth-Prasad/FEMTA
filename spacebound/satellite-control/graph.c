#include <stdint.h>
#include <string.h>
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

typedef struct Display_Window {
  WINDOW * window;
  MENU * menu;
} Display_Window;

Display_Menu   * control_menu;
Display_Window * control_window;
Display_Window * data_window;


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

ITEM ** create_items(char ** item_names, int number) {

  ITEM ** items = (ITEM **) calloc(number + 1, sizeof(ITEM *));

  for (int8_t i = 0; i < number; i++) items[i] = new_item(item_names[i], NULL);
  items[number] = (ITEM *) NULL;
  
  return items;
}

void terminate_display();

void change_display_mode(char * selection) {

}

void create_control_window() {

  // Allocate clean space for items
  int8_t n_choices = 5;

  char * item_names[] = {
    "Temperature",
    "Gyroscope",
    "Acceleration",
    "Magnetometer",
    "Exit",
  };
  
  ITEM ** items = create_items(item_names, n_choices);

  // Set function pointer bindings for menu items
  for (int i = 0; i < 4; i++) set_item_userptr(items[i], change_display_mode);
  set_item_userptr(items[4], terminate_display);
  
  // Create menu
  MENU * menu = new_menu((ITEM **) items);

  WINDOW * window = newwin(LINES - 6, 29, 1, 2);
  keypad(window, TRUE);

  set_menu_win(menu, window);
  set_menu_sub(menu, derwin(window, 6, 27, 3, 1));

  set_menu_mark(menu, "   ");

  box(window, 0, 0);
  print_in_middle(window, 1, 0, 30, "Data Selection", COLOR_PAIR(2));
  mvwaddch(window, 2, 0,  ACS_LTEE);
  mvwhline(window, 2, 1,  ACS_HLINE, 27);
  mvwaddch(window, 2, 28, ACS_RTEE);

  // Set structure components
  control_menu -> menu  = menu;
  control_menu -> items = items;
  control_menu -> n_choices = n_choices;
  
  control_window -> window = window;
  control_window -> menu = menu;
  
  // Print static messages on screen
  mvprintw(LINES - 3, 2, "Sample rate is 1Hz");
  mvprintw(LINES - 2, 2, "Use arrow keys to navigate");

  // Show the menu
  refresh();
  post_menu(control_menu -> menu);
  wrefresh(control_window -> window);
}



void create_data_window() {
    
  WINDOW * window = newwin(LINES - 2, COLS - 35, 1, 33);
  keypad(window, TRUE);

  
  box(window, 0, 0);
  print_in_middle(window, 1, 0, COLS - 35, "Data over time", COLOR_PAIR(2));
  mvwaddch(window, 2, 0,  ACS_LTEE);
  mvwhline(window, 2, 1,  ACS_HLINE, COLS - 37);
  mvwaddch(window, 2, COLS - 36, ACS_RTEE);

  mvwhline(window, LINES - 7, 2, ACS_HLINE, COLS - 39);

  data_window -> window = window;

  
  refresh();
  wrefresh(window);
}

int graph_cols;
int graph_rows;
int * graph_data;

FILE * temperatureFile;
double temperature;

void update_graph() {

  wattron(data_window -> window, COLOR_PAIR(4));

  temperatureFile = fopen("/sys/class/thermal/thermal_zone0/temp", "r");

  // Upon failure
  if (temperatureFile == NULL) {
    printf("The onboard temperature probe could not be read");
    exit(1);
  }

  fscanf(temperatureFile, "%lf", &temperature);
  //printf("\rTemperature: %6.3f °C.", T / 1000);
  //fflush(stdout);

  for (int x = 0; x < graph_cols; x++) {
    mvwprintw(data_window -> window, LINES - 8 - graph_data[x], x + 3, " ");
    /*for (int y = 0; y < graph_rows; y++) {
      mvwprintw(data_window -> window, LINES - 8 - y, x + 3, "#");
      }*/
  }
  
  for (int i = 1; i < graph_cols; i++) {
    graph_data[i - 1] = graph_data[i];
  }
  graph_data[graph_cols - 1] = (temperature / 1000 - 22);
  
  fclose(temperatureFile);
  
  for (int x = 0; x < graph_cols; x++) {
    mvwprintw(data_window -> window, LINES - 8 - graph_data[x], x + 3, "#");
    /*for (int y = 0; y < graph_rows; y++) {
      mvwprintw(data_window -> window, LINES - 8 - y, x + 3, "#");
      }*/
  }
  
  wattroff(data_window -> window, COLOR_PAIR(2));
  refresh();
  wrefresh(data_window -> window);

  sleep(1);
}

void terminate_display() {
  unpost_menu(control_menu -> menu);
  for (int i = 0; i < control_menu -> n_choices; ++i) free_item(control_menu -> items[i]);
  free_menu(control_menu -> menu);
  free(control_menu); control_menu = NULL;
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
  control_menu   = malloc(sizeof(Display_Menu));
  control_window = malloc(sizeof(Display_Window));
  data_window    = malloc(sizeof(Display_Window));

  graph_cols = COLS - 41;
  graph_rows = LINES - 10;
  graph_data = malloc(graph_cols * sizeof(int));
  
  create_control_window();
  create_data_window();
  
  int key_code;
  
  while (control_menu != NULL) {
    key_code = wgetch(control_window -> window);
    switch(key_code) {
      
    case KEY_DOWN:
      menu_driver(control_menu -> menu, REQ_DOWN_ITEM);
      break;

    case KEY_UP:
      menu_driver(control_menu -> menu, REQ_UP_ITEM);
      break;

    case 10: /* Enter */
      {

	for (;;) update_graph();
	
	ITEM *cur;
	void (*p)(char *);
      
	cur = current_item(control_menu -> menu);
	p = item_userptr(cur);
	p((char *) item_name(cur));
	//if (main_menu == NULL) break;
	pos_menu_cursor(control_menu -> menu);
	break;
      }

      break;
    }
  }

  terminate_display();
}
