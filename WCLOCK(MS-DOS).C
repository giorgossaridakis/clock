#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>

// constants
#define version 9.05
#define MAXPAGEENTRIES 63
#define MAXLINE 80
#define NAME 25
#define ESCAPE 27
#define SPACE 32
#define ENTER 13
#define APPLYDST '*'
#define SECONDSONOFF '!'
#define FINDPOINT '@'
#define FINDREFERENCE '#'
#define CYCLESCHEMES '+'
#define OTHERKEY 0
#define PGUP 73
#define PGDOWN 81
#define INITIALIZE 1000

typedef unsigned int ui;
enum { NONE=0, WORD };
enum { BOLD=1, REFERENCE, BOTH };
enum { BORDER=0, BOTTOM, BEFORE, AFTER, SAME, HIGHLIGHTED }; // color positions
int colorschemes[16][6]= {
    { WHITE, WHITE, WHITE, WHITE, WHITE, WHITE },
    { BLUE, MAGENTA, YELLOW, GREEN, CYAN, RED },
    { YELLOW, CYAN
    , WHITE, WHITE, WHITE, WHITE },
    { BLUE, RED, WHITE, WHITE, WHITE, MAGENTA },
    { RED, RED, WHITE, WHITE, WHITE, BLUE },
    { YELLOW, CYAN, BLUE, RED, YELLOW, GREEN },
    { CYAN, YELLOW, BLUE, RED, MAGENTA, GREEN },
    { MAGENTA, GREEN, YELLOW, CYAN, BLUE, RED },
    { RED, BLUE, CYAN, YELLOW, MAGENTA, RED },
    { RED, WHITE, MAGENTA, BLUE, RED, CYAN },
    { WHITE, RED, BLUE, CYAN, MAGENTA, RED },
    { GREEN, YELLOW, MAGENTA, CYAN, BLUE, RED },
    { GREEN, GREEN, CYAN, CYAN, BLUE, RED },
    { CYAN, CYAN, RED, RED, WHITE, BLUE },
    { BLUE, BLUE, WHITE, WHITE, GREEN, RED },
    { WHITE, MAGENTA, RED, BLUE, CYAN, GREEN }
};
unsigned int schemes=16;

typedef struct {
  char City[NAME];
  double localOffset;
  double dstCorrection;
  ui Bold:1;
  char Time[NAME]; } Location;
Location locations[MAXPAGEENTRIES];

ui secondson=1, clock24=1, applydst=1, scheme=1;
double mylocalOffset=-999;
int alllocationsnumber=0, currentpage=1, totalpages=0, locatecitymode=0;
char *filename;

// function declarations
int locatepointofinterest(int limit);
int searchcity();
void loadpage(int pagenumber, char *filename, int *locationsnumber);
int readlocationentries(int fd);
int fastforwardfile(int fd, int page);
int assignvaluestoarray(int fd, char array[MAXPAGEENTRIES*3][MAXLINE], int entries);
int locatecity(char *filename, char *city);
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
char *setupfilepathtoexecutable(const char *argument, const char *file);

int main(int argc, char *argv[])
{
  int i, i1, opt, x, y, row, c=INITIALIZE, locationsnumber, entrypos=0, color, blink;
  char line[MAXLINE], citytofind[NAME];
  filename=setupfilepathtoexecutable(argv[0], "wclock.dat");

   // read city entries
   if ((i1=open(filename, O_RDONLY))==-1)
    exit(-1);
   printf("initializing...");
   while ((readfileentry(i1, line))) {
    ++alllocationsnumber;
    if ((iscityboldreference(line))>BOLD) {
     if ((readfileentry(i1, line)==0))
      break;
     mylocalOffset=atof(line);
    }
   }
   alllocationsnumber/=3;
   totalpages=alllocationsnumber/MAXPAGEENTRIES;
   if (alllocationsnumber % MAXPAGEENTRIES)
    ++totalpages;

    while (c!=ESCAPE) {

      // read all entries, determine mylocalOffset
      if (c==INITIALIZE) {
       c=INITIALIZE+1;
       drawscreen(currentpage);
       if (mylocalOffset==-999)
	mylocalOffset=locations[0].localOffset;
       loadpage(currentpage, filename, &locationsnumber);
       if (argc==2) {
	strcpy(citytofind, argv[1]);
	searchcity(citytofind);
	loadpage(currentpage, filename, &locationsnumber);
       }
       drawscreen(currentpage);
      }

      if (kbhit()) {
       c=getch();
       if ((isalpha(c) || isdigit(c)) && entrypos<NAME-1) {
	citytofind[entrypos++]=c;
	continue;
       }
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
     drawscreen(currentpage);
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
     drawscreen(currentpage);
	break;
	case FINDPOINT:
	 if ((i1=locatepointofinterest(BOLD))) {
	  currentpage+=i1;
	  loadpage(currentpage, filename, &locationsnumber);
	  drawscreen(currentpage);
	 }
	break;
	case FINDREFERENCE:
	 opt=currentpage;
	 currentpage=1;
	 if ((i1=locatepointofinterest(REFERENCE)))
	  currentpage+=i1;
	 else
	  currentpage=opt;
	 loadpage(currentpage, filename, &locationsnumber);
	 drawscreen(currentpage);
	break;
	case CYCLESCHEMES:
	 scheme++;
	 if (scheme==schemes)
	  scheme=0;
	 drawscreen(currentpage);
	break;
	case OTHERKEY:
	 c=getch();
	 if (c==PGUP-1 || c==PGDOWN-1) // assume arrow keys as well
	  ++c;
	 switch (c) {
	  case PGUP:
	   if (currentpage==1 || currentpage==9999)
	    break;
	    showmessage("turning page...");
	   --currentpage;
	   loadpage(currentpage, filename, &locationsnumber);
	   drawscreen(currentpage);
	  break;
	  case PGDOWN:
	   if (locationsnumber<MAXPAGEENTRIES) // last page not fully loaded
	    break;
	   showmessage("turning page...");
	   loadpage(currentpage+1, filename, &locationsnumber);
	   drawscreen(++currentpage);
	  break; }
	  c=-1;
	 break;
	default:
	// nothing
       break; }
      }
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
       color=colorschemes[scheme][AFTER];
      if (mylocalOffset<locations[i1].localOffset)
       color=colorschemes[scheme][BEFORE];
      if (mylocalOffset==locations[i1].localOffset)
       color=colorschemes[scheme][SAME];
      if (locations[i1].Bold)
       color=colorschemes[scheme][HIGHLIGHTED];
      blink=0;
      if (locatecitymode==2 && !strcmp(locations[i1].City, citytofind))
       blink=BLINK;
      textcolor(color+blink);
      cprintf("%s", addspacestoline(line));
      x+=27;
      if (++row>2) {
       row=0;
       x=1;
       ++y;
      }
     }
     // erase reading message
     if (y<12)
      showmessage("                    ");
     sleep(1);
    }

    textcolor(WHITE);
    clrscr();

 return 0;
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

// load-reload page
void loadpage(int pagenumber, char *filename, int *locationsnumber)
{
  static int fd;
  int nread;

   if (pagenumber<=currentpage || !isfdopen(fd))
    if ((fd=open(filename, O_RDONLY))==-1)
     exit(-1);
   showmessage("reading database...");
   if (pagenumber>currentpage)
    pagenumber-=currentpage;
   nread=fastforwardfile(fd, pagenumber);
   *locationsnumber=readlocationentries(fd);
   if (!nread)
    close(fd);
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
   while ((nread=readfileentry(i1, line))) {
    line[0]=toupper(line[0]);
    if (!strcmp(city, line))
     break;
    ++read;
   }
   close(i1);

  if (nread==0)
   return 0;

  if (read<MAXPAGEENTRIES*3)
   read=1;
  else {
   read/=3*MAXPAGEENTRIES;
   if (read % MAXPAGEENTRIES)
    ++read;
  }

 return read;
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

// setup filepath to .dat file
char *setupfilepathtoexecutable(const char *argument, const char *file)
{
 int i;
 static char tfilename[MAXLINE];

  strcpy(tfilename, argument);
  for (i=0;i<strlen(tfilename);i++)
   if (tfilename[i]=='\\')
    tfilename[i]='/';
  while (tfilename[i]!='/' && i)
   --i;
  tfilename[i]='\0';
  strcat(tfilename, "/");
  strcat(tfilename, file);

 return tfilename;
}

// draw screen
void drawscreen(int pagenumber)
{
  int x, y;
  char line[MAXLINE];
  time_t mytime; struct tm* timeinfo;

     clrscr();
     // draw ascii frame & print information
     textcolor(colorschemes[scheme][BORDER]);
     for (x=1;x<81;x++) {
      gotoxy(x, 2);
      putch('-');
      gotoxy(x, 24);
      putch('-');
     }
     for (y=2;y<24;y++) {
      gotoxy(1, y);
      putch('-');
      gotoxy(28, y);
      putch('-');
      gotoxy(54, y);
      putch('-');
      gotoxy(80, y);
      putch('-');
     }
     time (&mytime);
     timeinfo = localtime (&mytime);
     strftime(line, MAXLINE, "%A %d %B %Y", timeinfo);
     x=4-numberofzeroes(totalpages);
     gotoxy(x, 24);
     textcolor(colorschemes[scheme][BOTTOM]);
     cprintf("%s cities:%d page:%d/%d <pgup/down> <+@#> <esc> quit", line, alllocationsnumber, pagenumber, totalpages);
     gotoxy(2, 1);
     textcolor(colorschemes[scheme][BORDER]);
     cprintf("World Clock %.2lf <*>dst on/off <!>seconds on/off <space>12/24hours <enter>find ", version);
}

// show a message
void showmessage(char *message)
{
  gotoxy(32, 12);
  textcolor(RED);
  cprintf("%s", message);
  textcolor(BLACK);
  gotoxy(1,1);
}

// return number of decimals in an integer
int numberofzeroes(int num)
 {
   char buffer[NAME];

    sprintf(buffer, "%d", num);

return strlen(buffer)-1;
}
