// wclock.c, a world clock for the ncurses terminal

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include <ctype.h>
#include <pwd.h>

// constants
#define version 9.02
#define MAXPAGEENTRIES 63
#define MAXLINE 80
#define NAME 25
#define ESCAPE 27
#define SPACE 32
#define ENTER '\n'
#define APPLYDST '*'
#define SECONDSONOFF '!'
#define FINDPOINT '@'
#define FINDREFERENCE '#'
#define PGUP 339
#define PGDOWN 338
#define INITIALIZE 1000

// global variables and structures
WINDOW *win1;

typedef unsigned int ui;
typedef struct {
  char City[NAME];
  double localOffset;
  double dstCorrection;
  ui Bold:1;
  char Time[NAME]; } Location;
Location locations[MAXPAGEENTRIES];

ui secondson=1, clock24=1, explicitmylocaloffset=0, applydst=1;
double mylocalOffset;
int alllocationsnumber=0, currentpage=1, totalpages=0, locatecitymode=0;
char *filename;
enum { RED=1, GREEN=2, YELLOW, BLUE, MAGENTA, CYAN, WHITE, BLACK };
enum { BOLD=1, REFERENCE, BOTH };

// function declarations
void loadpage(int pagenumber, char *filename, int *locationsnumber);
int readlocationentries(int fd);
int fastforwardfile(int fd, int page);
int assignvaluestoarray(int fd, char array[MAXPAGEENTRIES*3][MAXLINE], int entries);
int locatecity(char *filename, char *city);
int locatepointofinterest(int limit);
int searchcity();
void createtimestring(int entryid);
int iscityboldreference(char *city);
size_t readfileentry(int fd, char *line);
char* addspacestoline(char *line);
unsigned int isseparationchar(char t);
unsigned int isfdopen(int fd);
char *setupfilepathtoexecutable(const char *argument, const char *file);
int numberofzeroes(int num);
void drawscreen(int pagenumber);
void showmessage(char *message);
int initscreen();
void endscreen();
void textcolor(int choice);
void gotoxy(int x, int y);
void showusage();

int main(int argc, char *argv[])
{
  int i, i1, opt, x, y, row, c=INITIALIZE, locationsnumber=MAXPAGEENTRIES, entrypos=0;
  char line[MAXLINE], citytofind[NAME];
  struct passwd *pw = getpwuid(getuid());
  sprintf((filename=malloc(MAXLINE)), "%s/.wclock", pw->pw_dir);
  
   // parse command line
   while ((opt = getopt(argc, argv, ":scd")) != -1) {
    switch (opt) {
     case 's':
      secondson=0;
     break;
     case 'c':
      clock24=0;
     break;
     case 'd':
      applydst=0;
     break;
     case '?':
      showusage();
     break;
    }
   }
  initscreen();
    
  // read city entries
   if ((i1=open(filename, O_RDONLY))==-1)
    exit(-1);
   while ((readfileentry(i1, line))) {
    ++alllocationsnumber;
    if ((iscityboldreference(line))>BOLD) {
     if ((readfileentry(i1, line)==0))
      break;
     mylocalOffset=atof(line);
     explicitmylocaloffset=1;
    }
   }
   alllocationsnumber/=3;
   totalpages=alllocationsnumber/MAXPAGEENTRIES;
   if (alllocationsnumber % MAXPAGEENTRIES)
    ++totalpages;
   
   wtimeout(win1, 1000); // block getch() for 1000ms

    while (c!=ESCAPE) {

      c=(c==INITIALIZE) ? INITIALIZE+1 : getch(); // skip 1 second wait on entry
      // and read all entries, determine mylocalOffset
      if (c==INITIALIZE+1) {
       if (explicitmylocaloffset==0) {
        mylocalOffset=locations[0].localOffset;
        explicitmylocaloffset=1;
       }
       loadpage(currentpage, filename, &locationsnumber);
       if (optind<argc) {
	    strcpy(citytofind, argv[optind]);
	    searchcity(citytofind);
	    loadpage(currentpage, filename, &locationsnumber);
       }
      }
      if ((isalpha(c) || isdigit(c)) && entrypos<NAME-1)
       citytofind[entrypos++]=c;
      switch(c) {
       case ENTER:
        if (entrypos==0)
         break;
        citytofind[entrypos]='\0';
        entrypos=0;
	    if ((i=atoi(citytofind))) {
	     if (i==currentpage || i<0 || i>totalpages)
	      break;
	     showmessage("jumping to page...");
	     currentpage=i;
	    }
        else
	     searchcity(citytofind);
        loadpage(currentpage, filename, &locationsnumber);
       break;
       case SECONDSONOFF:
	    secondson=(secondson) ? 0 : 1;
       break;
       case SPACE:
	    clock24=(clock24) ? 0 : 1;
       break;
       case APPLYDST:
        applydst=(applydst) ? 0 : 1;
        loadpage(currentpage, filename, &locationsnumber);
       break;
	   case FINDPOINT:
	    if ((i1=locatepointofinterest(BOLD))) {
	     currentpage+=i1;
	     loadpage(currentpage, filename, &locationsnumber);
	    }
       break;
	   case FINDREFERENCE:
	    if ((i1=locatepointofinterest(REFERENCE))) {
	     currentpage+=i1;
	     loadpage(currentpage, filename, &locationsnumber);
	    }
	   break;
	   case PGUP:
	   if (currentpage==1 || currentpage==999)
	    break;
       locatecitymode=0;
	   showmessage("turning page...");       
	    --currentpage;
        loadpage(currentpage, filename, &locationsnumber);
       break;
	   case PGDOWN:
	   if (locationsnumber<MAXPAGEENTRIES) // last page not fully loaded
	    break;
       locatecitymode=0;
	   showmessage("turning page...");
	   ++currentpage;
	   loadpage(currentpage, filename, &locationsnumber);
      break;
      default:
	  // nothing
     break; }
    
     // construct time string for entries and print
     for (x=1, y=2, row=i1=0;i1<locationsnumber;i1++) {
      createtimestring(i1);
      sprintf(line, "%s %s", locations[i1].City, locations[i1].Time);
      line[NAME]='\0';
      switch (row) {
       case 0:
	    gotoxy(x+2, y+1);
       break;
       case 1:
	    gotoxy(x+1, y+1);
       break;
       case 2:
	    gotoxy(x, y+1);
      break;
      }
      if (mylocalOffset>locations[i1].localOffset)
       textcolor(GREEN);
      if (mylocalOffset<=locations[i1].localOffset)
       textcolor(YELLOW);
      if (mylocalOffset==locations[i1].localOffset)
       textcolor(CYAN);
      if (locations[i1].Bold) {
       if (has_colors())
        textcolor(RED);
       else
        attron(A_BOLD);
      }
      if (locatecitymode==2 && !strcmp(locations[i1].City, citytofind))
       attron(A_BLINK);
      printw("%s", addspacestoline(line));
      attroff(A_BOLD);
      attroff(A_BLINK);
      x+=27;
      if (++row>2) {
       row=0;
       x=1;
       ++y;
      }
     }
     // erase reading message
     if (y<12)
      showmessage("                   ");
     refresh();
    }
    endwin();

 return 0;
}

// wclock specific routines

// load-reload page
void loadpage(int pagenumber, char *filename, int *locationsnumber)
{
  int fd;
  
   if ((fd=open(filename, O_RDONLY))==-1)
    exit(-1);
   showmessage("reading database...");
   fastforwardfile(fd, pagenumber);
   *locationsnumber=readlocationentries(fd);
   close(fd);
   drawscreen(pagenumber);
   refresh();
}

// read config file
int readlocationentries(int fd)
{
  int i, i1, tbold, entriesnumber, locationsnumber=0;
  char array[MAXPAGEENTRIES*3][MAXLINE];
  char tlines[3][MAXLINE];

   entriesnumber=assignvaluestoarray(fd, array, MAXPAGEENTRIES*3);
   for (i=locationsnumber=0;i<entriesnumber;locationsnumber++, i+=3) {
    for (i1=0;i1<3;i1++)
     strcpy(tlines[i1], array[i+i1]);
    strcpy(locations[locationsnumber].City, tlines[0]);
    locations[locationsnumber].City[0]=toupper(locations[locationsnumber].City[0]);
    locations[locationsnumber].Bold=0;
    if ((tbold=iscityboldreference(locations[locationsnumber].City))==BOLD || tbold==BOTH)
     locations[locationsnumber].Bold=tbold;
    locations[locationsnumber].localOffset=atof(tlines[1]);
    locations[locationsnumber].dstCorrection=abs(atof(tlines[2]));
   }
   
   if (applydst)
    for (i=0;i<locationsnumber;i++)
     if (mylocalOffset!=locations[i].localOffset)
      locations[i].localOffset-=locations[i].dstCorrection;
   

 return locationsnumber;
}

// fastforward file to entries
int fastforwardfile(int fd, int page)
{
  int i, i1=0, nread=1;
  char tline[MAXLINE];

   for (i=1;i<page && nread;i++)
    for (i1=0;i1<MAXPAGEENTRIES*3 && nread;i1++)
     nread=readfileentry(fd, tline);

 return nread;
}

// using readfileentry, assign strings to array of pointers
int assignvaluestoarray(int fd, char array[MAXPAGEENTRIES*3][MAXLINE], int entries)
{
  int i, actualentries=0;
  char tline[MAXLINE];

   for (i=0;i<entries && (isfdopen(fd));i++)
    if ((readfileentry(fd, tline)))
     strcpy(array[actualentries++], tline);


 return actualentries;
}

// locate city in database
int locatecity(char *filename, char *city)
{
  int i1, nread, read=0;
  char line[MAXLINE];
  
   if ((i1=open(filename, O_RDONLY))==-1)
    exit(-1);
   city[0]=toupper(city[0]);
   while ((nread=readfileentry(i1, line))) {
    if (!strcmp(city, line))
     break;
    ++read;
   }
   close(i1);
   
   if (nread==0)
    return 0;
   
   if (read<3*MAXPAGEENTRIES)
    read=1;
   else {
    read/=3*MAXPAGEENTRIES;
    if (read % MAXPAGEENTRIES)
     ++read;
   }

 return read;
}

// find point of interest
int locatepointofinterest(int limit)
{
  int i, i1, locationsnumber, page;
  char line[MAXLINE];
  locationsnumber=0; page=1;

  if (currentpage==totalpages)
   return 0;
  
   showmessage("locating point...");
   if ((i=open(filename, O_RDONLY))==-1)
    exit(-1);
   fastforwardfile(i, currentpage+1);
   while ((i1=readfileentry(i, line))) {
    ++locationsnumber;
    if ((iscityboldreference(line))>=limit )
     break;
    if ((locationsnumber/3)>=MAXPAGEENTRIES) {
     ++page;
     locationsnumber=0;
    }
   }
   close(i);

 return (i1) ? page : i1;
}

// locate city
int searchcity(char *city)
{
  int i;

    city[0]=toupper(city[0]);
    locatecitymode=1;
    showmessage("locating city...");
    i=locatecity(filename, city);
    locatecitymode=(i) ? 2 : 0;
    if (i)
     currentpage=i;

  return i;
}

// use mktime to create time string for locations
void createtimestring(int entryid)
{
  time_t rawtime;
  struct tm * timeinfo;
  double offsetfraction;
  int offsetinteger;
  char buffer[NAME];

  time (&rawtime);
  timeinfo = localtime (&rawtime);

   // remake time structure according to timezone, for locations above first
   if (locations[entryid].localOffset!=mylocalOffset) {
    offsetinteger=abs(mylocalOffset-locations[entryid].localOffset);
    offsetfraction=abs((locations[entryid].localOffset - ((int)locations[entryid].localOffset))*60);

    // cases of mylocalOffset versus localOffset
    if (mylocalOffset>locations[entryid].localOffset) {
     timeinfo->tm_hour-=offsetinteger;
     timeinfo->tm_min-=offsetfraction;
    }
    if (locations[entryid].localOffset>mylocalOffset) {
     timeinfo->tm_hour+=offsetinteger;
     timeinfo->tm_min+=offsetfraction;
    }

    mktime(timeinfo);
   }
   if (clock24) {
    if (secondson)
     strftime(buffer, NAME, "%H:%M:%S", timeinfo);
    else
     strftime(buffer, NAME, "%H:%M", timeinfo);
   }
   else {
    if (secondson)
     strftime(buffer, NAME, "%I:%M:%S%p", timeinfo);
    else
     strftime(buffer, NAME, "%I:%M%p", timeinfo);
   }
 strcpy(locations[entryid].Time, buffer);

}

// is city reference
int iscityboldreference(char *city)
{
  int tlength=strlen(city)-1, count=0;
  
  while (tlength) {
   if (city[tlength]=='*') {
    ++count;
    city[tlength--]='\0';;
   }
   if (city[tlength]=='&') {
    count=(count==0) ? 2 : 3;
    city[tlength--]='\0';
   }
   if (city[tlength]!='*' && city[tlength]!='&')
    tlength=0;
  }
  
 return count;
}

// library routines


// read entry from file
size_t readfileentry(int fd, char *line)
{
  char buf[1];
  int i=0;
  size_t nread;

    while ((nread=read(fd, buf, sizeof(buf)))) {
     if (isseparationchar(buf[0])) {
      if (i==0) // no characters in line, separation character skipped
       continue;
      break; // break read, separation character not recorded
     }
     line[i++]=buf[0];
    }
    line[i]='\0';

    // file ended, close file descriptor
    if (nread==0)
     close(fd);
    
 return nread;
}

// add spaces to the beggining of string
char* addspacestoline(char *line)
{
  int i=0, spaces=25-strlen(line);
  char tline[25];

   for (i=0;i<spaces;i++)
    tline[i]=' ';
   tline[i]='\0';
   if (spaces) {
    strcat(tline, line);
    strcpy(line, tline);
   }

 return line;
}

enum { NONE=0, WORD };
// word separation characters
unsigned int isseparationchar(char t)
{
  if (t==' ' || t=='\n' || t=='\r')
   return WORD;
  if (locatecitymode==1)
   if (t=='*' || t=='&')
    return WORD;

 return NONE;
}

// if fd open
unsigned int isfdopen(int fd)
{
  struct stat tfd;

   if (fstat(fd, &tfd) == -1)
    return 0;

 return 1;
}

// return number of decimals in an integer
int numberofzeroes(int num)
 {
   char buffer[NAME];
   
    sprintf(buffer, "%d", num);
    
return strlen(buffer)-1;
}

// ncurses & screen related

// draw screen
void drawscreen(int pagenumber)
{
  int x, y;
  char line[MAXLINE];
  time_t mytime; struct tm* timeinfo;

     clear();
     // draw ascii frame & print information
     textcolor(BLUE);
     for (x=1;x<81;x++) {
      gotoxy(x, 2);
      addch('-');
      gotoxy(x, 24);
      addch('-');
     }
     for (y=2;y<24;y++) {
      gotoxy(1, y);
      addch('-');
      gotoxy(28, y);
      addch('-');
      gotoxy(54, y);
      addch('-');
      gotoxy(80, y);
      addch('-');
     }
     time (&mytime);
     timeinfo = localtime (&mytime);
     strftime(line, MAXLINE, "%A %d %B %Y", timeinfo);
     x=5-numberofzeroes(totalpages);
     gotoxy(x, 24);
     textcolor(MAGENTA);
     printw("%s cities:%d page:%d/%d <pgup/down> <@#> <esc> quit", line, alllocationsnumber, pagenumber, totalpages);
     gotoxy(2, 1);
     textcolor(BLUE);
     printw("World Clock %.2lf <*>dst on/off <!>seconds on/off <space>12/24hours <enter>find ", version);
     refresh();
}

// initialize ncurses screen
int initscreen()
{
  win1=initscr();
  noecho();
  cbreak();
  keypad(win1, TRUE);
  curs_set(0);
  start_color();
 
  // basic pairs
  init_pair(RED, COLOR_RED, COLOR_BLACK);
  init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
  init_pair(BLACK, COLOR_BLACK, COLOR_BLACK);
  
 return has_colors();
}

// show message
void showmessage(char *message)
{
   gotoxy(32, 12);
   textcolor(RED);
   printw("%s", message);
   gotoxy(1,1);
   refresh();
}

// close screen
void endscreen()
{
  delwin(win1);
  endwin();
  curs_set(1);
}

// change color
void textcolor(int choice)
{
   attron(COLOR_PAIR(choice));
}

// gotoxy adjusted for ncurses move
void gotoxy(int x, int y)
{
  move(y-1, x-1);
}

// show usage
void showusage()
{
  printf("Usage:\n wclock [options] city \n\nA terminal world clock.\n\nOptions:\n");
  printf(" -s\t\tseconds off, default on\n -c\t\t12 hour clock, default 24hours\n -d\t\tdo not apply daylight savings\n     --help\tdisplay this help\n\nDistributed under the GNU Public licence.\n");
  exit (-1);
}
