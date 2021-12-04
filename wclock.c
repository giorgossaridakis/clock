#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <ncurses.h>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include "cursesscreen.c"

#define version 2.0
#define MAXPAGEENTRIES 63
#define MAXLINE 80
#define NAME 25
#define ESCAPE 27
#define SPACE 32
#define ENTER '\n'
#define OTHERKEY 0
#define PGUP 339
#define PGDOWN 338

typedef unsigned int ui;
typedef struct {
  char City[NAME];
  double localOffset;
  double dstCorrection;
  ui Bold:1;
  char Time[NAME]; } Location;
Location locations[MAXPAGEENTRIES];

ui secondson=1, clock24=1;
double mylocalOffset;
int alllocationsnumber=0, currentpage=1;

// function declarations
void loadpage(char *filename, int *locationsnumber);
int readconfigfile(char *filename);
int fastforwardfile(int fd, int page);
size_t readfileentry(int fd, char *line);
int assignvaluestoarray(int fd, char array[MAXPAGEENTRIES*3][MAXLINE], int entries);
void createtimestring(int entryid) ;
char* addspacestoline(char *line);
unsigned int isseparationchar(char t);
unsigned int isfdopen(int fd);
char *setupfilepathtoexecutable(const char *argument, const char *file);
void drawscreen();

int main(int argc, char *argv[])
{
  int i, i1, x, y, row, c, locationsnumber;
  char filename[MAXLINE], line[MAXLINE];
  struct passwd *pw = getpwuid(getuid());
  sprintf(filename, "%s/.wclock", pw->pw_dir);
  initscreen();
  
  // read city entries
   if ((i1=open(filename, O_RDONLY))==-1)
    exit(-1);
   while ((readfileentry(i1, line)))
    ++alllocationsnumber;
   alllocationsnumber/=3;

   loadpage(filename, &locationsnumber);
   mylocalOffset=locations[0].localOffset;
   wtimeout(win1, 1000); // block getch() for 1000ms

    while (c!=ESCAPE) {

      c=getch();
      switch(c) {
       case ENTER:
	    secondson=(secondson) ? 0 : 1;
       break;
       case SPACE:
	    clock24=(clock24) ? 0 : 1;
       break;
	   case PGUP:
	   if (currentpage==1 || currentpage==9)
	    break;
	    --currentpage;
	    loadpage(filename, &locationsnumber);
       break;
	   case PGDOWN:
	   if (locationsnumber<MAXPAGEENTRIES) // last page not fully loaded
	    break;
	   ++currentpage;
	   loadpage(filename, &locationsnumber);
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
      if (locations[i1].Bold)
       textcolor(RED);
//        attron(A_BOLD);
      printw("%s", addspacestoline(line));
//       attroff(A_BOLD);
      x+=27;
      if (++row>2) {
       row=0;
       x=1;
       ++y;
      }
     }
     refresh();
    }
    endwin();

 return 0;
}

// load-reload page
void loadpage(char *filename, int *locationsnumber)
{
  int i;

   *locationsnumber=readconfigfile(filename);
   drawscreen();
}

// read config file
int readconfigfile(char *filename)
{
  int i, i1, infile, entriesnumber, locationsnumber=0;
  char array[MAXPAGEENTRIES*3][MAXLINE];
  char tlines[3][MAXLINE];

   if ((infile=open(filename, O_RDONLY))==-1)
    exit(-1);
   fastforwardfile(infile, currentpage);
   entriesnumber=assignvaluestoarray(infile, array, MAXPAGEENTRIES*3);

   for (i=locationsnumber=0;i<entriesnumber;locationsnumber++, i+=3) {
    for (i1=0;i1<3;i1++)
     strcpy(tlines[i1], array[i+i1]);
    strcpy(locations[locationsnumber].City, tlines[0]);
    if (locations[locationsnumber].City[strlen(locations[locationsnumber].City)-1]=='*') {
     locations[locationsnumber].City[strlen(locations[locationsnumber].City)-1]='\0';
     locations[locationsnumber].Bold=1;
    }
    else
     locations[locationsnumber].Bold=0;
    locations[locationsnumber].localOffset=atof(tlines[1]);
    locations[locationsnumber].dstCorrection=abs(atof(tlines[2]));
    if (locationsnumber==0)
     locations[locationsnumber].dstCorrection=0;
    locations[locationsnumber].localOffset-=locations[locationsnumber].dstCorrection;
   }

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

   currentpage=i;

 return nread;
}

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

// draw screen
void drawscreen()
{
  int x, y, totalpages;
  char line[MAXLINE];
  time_t mytime; struct tm* timeinfo;
  totalpages=alllocationsnumber/MAXPAGEENTRIES;
  if (alllocationsnumber % MAXPAGEENTRIES)
   ++totalpages;

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
     gotoxy(4, 24);
     textcolor(MAGENTA);
     printw("%s cities: %d page: %d/%d <pgup> <pgdown> <esc> quit",  line, alllocationsnumber, currentpage, totalpages);
     gotoxy(5, 1);
     textcolor(BLUE);
     printw("World Clock %.2lf <space> toggle 12/24hours <enter> toggle seconds on/off", version);
     refresh();
}
