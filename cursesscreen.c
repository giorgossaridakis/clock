// cursescreen.cc
WINDOW *win1;

int initscreen();
void endscreen();
void textcolor(int choice);
void gotoxy(int x, int y);

enum { RED=1, GREEN=2, YELLOW, BLUE, MAGENTA, CYAN, WHITE=58 };

// Initialize screen
int initscreen()
{
  win1=initscr();
  noecho();
  cbreak();
  keypad(win1, TRUE);
//   mousemask(ALL_MOUSE_EVENTS, NULL);
  if (has_colors() == FALSE) {
   endwin();
   printf("Your terminal does not support color\n");
  raise(2); }
  curs_set(1);
  start_color();
  
  /* colors  
        COLOR_BLACK   0
        COLOR_RED     1
        COLOR_GREEN   2
        COLOR_YELLOW  3
        COLOR_BLUE    4
        COLOR_MAGENTA 5
        COLOR_CYAN    6
        COLOR_WHITE   7 
    
    attributes
    A_NORMAL        Normal display (no highlight)
    A_STANDOUT      Best highlighting mode of the terminal.
    A_UNDERLINE     Underlining
    A_REVERSE       Reverse video
    A_BLINK         Blinking
    A_DIM           Half bright
    A_BOLD          Extra bright or bold
    A_PROTECT       Protected mode
    A_INVIS         Invisible or blank mode
    A_ALTCHARSET    Alternate character set
    A_CHARTEXT      Bit-mask to extract a character
    COLOR_PAIR(n)   Color-pair number n */
 
  // basic pairs
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_YELLOW, COLOR_BLACK);
  init_pair(4, COLOR_BLUE, COLOR_BLACK);
  init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(6, COLOR_CYAN, COLOR_BLACK);
  // mixed colors 
  init_pair(7, COLOR_RED, COLOR_GREEN);
  init_pair(8, COLOR_RED, COLOR_YELLOW);
  init_pair(9, COLOR_RED, COLOR_BLUE);
  init_pair(10, COLOR_RED, COLOR_MAGENTA);
  init_pair(11, COLOR_RED, COLOR_CYAN);
  init_pair(12, COLOR_RED, COLOR_WHITE);
  init_pair(13, COLOR_GREEN, COLOR_RED);  
  init_pair(14, COLOR_GREEN, COLOR_YELLOW);  
  init_pair(15, COLOR_GREEN, COLOR_BLUE);
  init_pair(16, COLOR_GREEN, COLOR_MAGENTA);
  init_pair(17, COLOR_GREEN, COLOR_CYAN);
  init_pair(18, COLOR_GREEN, COLOR_WHITE);
  init_pair(19, COLOR_RED, COLOR_GREEN); // twice declared :)
  init_pair(20, COLOR_YELLOW, COLOR_RED);  
  init_pair(21, COLOR_YELLOW, COLOR_GREEN);   
  init_pair(22, COLOR_YELLOW, COLOR_BLUE); 
  init_pair(23, COLOR_YELLOW, COLOR_MAGENTA); 
  init_pair(24, COLOR_YELLOW, COLOR_CYAN);  
  init_pair(25, COLOR_YELLOW, COLOR_WHITE); 
  init_pair(26, COLOR_BLUE, COLOR_RED); 
  init_pair(27, COLOR_BLUE, COLOR_GREEN); 
  init_pair(28, COLOR_BLUE, COLOR_YELLOW); 
  init_pair(29, COLOR_BLUE, COLOR_MAGENTA); 
  init_pair(30, COLOR_BLUE, COLOR_CYAN);
  init_pair(31, COLOR_BLUE, COLOR_WHITE); 
  init_pair(32, COLOR_MAGENTA, COLOR_RED); 
  init_pair(33, COLOR_MAGENTA, COLOR_GREEN); 
  init_pair(34, COLOR_MAGENTA, COLOR_YELLOW); 
  init_pair(35, COLOR_MAGENTA, COLOR_BLUE); 
  init_pair(36, COLOR_MAGENTA, COLOR_CYAN); 
  init_pair(37, COLOR_MAGENTA, COLOR_WHITE); 
  init_pair(38, COLOR_CYAN, COLOR_RED);   
  init_pair(39, COLOR_CYAN, COLOR_GREEN); 
  init_pair(40, COLOR_CYAN, COLOR_YELLOW); 
  init_pair(41, COLOR_CYAN, COLOR_BLUE); 
  init_pair(42, COLOR_CYAN, COLOR_MAGENTA); 
  init_pair(43, COLOR_CYAN, COLOR_WHITE); 
  init_pair(44, COLOR_WHITE, COLOR_RED);
  init_pair(45, COLOR_WHITE, COLOR_GREEN);
  init_pair(46, COLOR_WHITE, COLOR_YELLOW);  
  init_pair(47, COLOR_WHITE, COLOR_BLUE);
  init_pair(48, COLOR_WHITE, COLOR_MAGENTA);
  init_pair(49, COLOR_WHITE, COLOR_CYAN);
  // same colors background and foreground
  init_pair(50, COLOR_BLACK, COLOR_BLACK);
  init_pair(51, COLOR_RED, COLOR_RED);
  init_pair(52, COLOR_GREEN, COLOR_GREEN);  
  init_pair(53, COLOR_YELLOW, COLOR_YELLOW);  
  init_pair(54, COLOR_BLUE, COLOR_BLUE);
  init_pair(55, COLOR_MAGENTA, COLOR_MAGENTA);
  init_pair(56, COLOR_CYAN, COLOR_CYAN);
  init_pair(57, COLOR_WHITE, COLOR_WHITE);
  // white on COLOR_BLACK  
  init_pair(58, COLOR_WHITE, COLOR_BLACK);

  attron(COLOR_PAIR(58));  
  
 return 0;
}

// close screen
void endscreen()
{
  delwin(win1);
  endwin();
  curs_set(1);
  refresh();
}

// change color
void textcolor(int choice)
{
  int color;  
  
  if (!choice)
   color=58;
  else
   color=choice;
   attron(COLOR_PAIR(color));
}

void gotoxy(int x, int y)
{
  move(y-1, x-1);
}

