// clock, world clock for the terminal
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <fstream>
#include <ctime>
#include <vector>

using namespace std;

// type definitions
typedef unsigned long ul;
typedef unsigned int ui;
typedef const int ci;
typedef const double cd;

// global constants & variables
ci MAXNAME=100, NAME=25, MAXLOCATIONS=63, INITIALIZE=1000;
ci TOGGLE24HOUR=32, ESCAPE=27, TOGGLESECONDS='\n';
cd version=1.0;
bool clock24=TRUE, secondson=TRUE;
char datafilepath[500], cfgfilepath[500];
WINDOW *win1=newwin(80, 24, 1, 1);
class Location {
 public:
  char City[NAME];
  double localOffset;
  char Time[NAME];
  ui Bold:1;
  Location(char *city, double offset, ui bold): localOffset(offset), Bold(bold) {  strcpy(City, city); };
  void CreateTimeString();

  Location() { };
 ~Location() { };
};
vector<Location> locations;

// function declarations
int ReadLocationData(char city_name[], double dst);
int ReadConfigFile();
char* addspacestoline(char *line);
void Sleep(ul sleepMs) { sleep(sleepMs/1000); };

void Location::CreateTimeString() 
{
  time_t rawtime;
  struct tm * timeinfo;
  double mylocalOffset=locations[0].localOffset;

  time (&rawtime);
  timeinfo = localtime (&rawtime);
  
   // remake time structure according to timezone, for locations above first
   if (localOffset!=mylocalOffset) {
    int offsetinteger=mylocalOffset-localOffset;
    double offsetfraction=(localOffset - ((int)localOffset))*60;
    offsetinteger=(offsetinteger<0) ? offsetinteger*-1 : offsetinteger;
    offsetfraction=(offsetfraction<0) ? offsetfraction*-1 : offsetfraction;
    enum cases { ADD=0, SUBSTRACT };
    ui arithmeticaction;
    
    // cases of mylocalOffset versus localOffset
    if (mylocalOffset>localOffset)
     arithmeticaction=SUBSTRACT;
    if (localOffset>mylocalOffset)
     arithmeticaction=ADD; 
    if (arithmeticaction==ADD) {
     timeinfo->tm_hour+=offsetinteger;
     timeinfo->tm_min+=offsetfraction;
    }
    else {
     timeinfo->tm_hour-=offsetinteger;
     timeinfo->tm_min-=offsetfraction;
    }

    mktime(timeinfo);   
   }
   
   char buffer[NAME];
   if (secondson) {
    if (clock24)
     strftime (buffer, NAME, "%H:%M:%S", timeinfo);
    else
     strftime (buffer, NAME, "%I:%M:%S%p", timeinfo);
   }
   else {
    if (clock24)
     strftime (buffer, NAME, "%H:%M", timeinfo);
    else
     strftime (buffer, NAME, "%I:%M%p", timeinfo);
   }
   
 strcpy(Time, buffer);

}

int main()
{
   // construct filepaths
   struct passwd *pw = getpwuid(getuid());
   sprintf(datafilepath, "%s/astro.dat", pw->pw_dir);
   sprintf(cfgfilepath, "%s/.clock", pw->pw_dir);
   
   // initialize ncurses
   win1=initscr();
   noecho();
   cbreak();
   keypad(win1, TRUE);
   curs_set(0);
   int i=INITIALIZE, i1;
   // read .clock
   if ((i1=ReadConfigFile())==0)
    i=ESCAPE;
   char line[MAXNAME];
   int x, y, row;
   
   wtimeout(win1, 1000); // block getch() for 1000ms
   // main loop
   while (i!=ESCAPE) {

    i=(i==INITIALIZE) ? 0 : getch(); // skip getch() at first display
    if (i==TOGGLE24HOUR) // space toggles 12/24 hour
     clock24=(clock24) ? FALSE : TRUE;
    if (i==TOGGLESECONDS) // toggles seconds
     secondson=(secondson) ? FALSE : TRUE;
    clear();
    // draw ascii frame & print information
    for (x=0;x<80;x++) {
     move(1, x);
     addch('-');
     move(23, x);
     addch('-');
    }
    for (y=1;y<23;y++) {
     move(y, 0);
     addch('-');
     move(y, 27);
     addch('-');
     move(y, 53);
     addch('-');
     move(y, 79);
     addch('-');
    }       
    move(0, 0);
    printw("  terminal world clock %.2lf  <space> toggles 12/24hour <enter> toggles seconds", version);
    
    // construct time string for entries and print
    for (x=1, y=2, row=i1=0;i1<locations.size()-1;i1++) {
     locations[i1].CreateTimeString();
     sprintf(line, "%s %s", locations[i1].City, locations[i1].Time);
     line[NAME]='\0';
     switch (row) {
      case 0:
       move(y, x+1);
      break;
      case 1:
       move(y, x);
      break;
      case 2:
       move(y, x-1);
     break;
     }
     if (locations[i1].Bold)
      attron(A_BOLD);
     printw("%s", addspacestoline(line));
     attroff(A_BOLD);
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

// search for city data and load to vector
int ReadLocationData(char city_name[], double dst) // dst double in case it's ever needed
{
  ifstream datafile;
  char country[MAXNAME], city[MAXNAME], region[MAXNAME];
  float lat, lng;
  double offset;
  ui tbold=0;
  
   datafile.open(datafilepath);
   if (city_name[strlen(city_name)-1]=='*') { // star at end of city denotes bold
    city_name[strlen(city_name)-1]='\0';
    tbold=1;
   }
   // read entries
    while (datafile) {
     datafile >> country >> city >> lat >> lng >> region >> offset;
     if (!strcmp(city, city_name))
      break;
    }
    
    // end of file or location entries exceed screen limit
    if (datafile.eof() || locations.size()>MAXLOCATIONS)
     return -1;
    if (offset<0)
     dst*=-1;
    if (!locations.size())
     dst=0; // zero daylight saving for location reference
    Location tlocation(city, offset-dst, tbold);
    locations.push_back(tlocation);

    datafile.close();
  
 return 0;
}

// read .cfg file
int ReadConfigFile()
{
  ifstream cfgfile;
  char tcity[MAXNAME];
  double dst;
  int entries=0;
  
   locations.clear();
   cfgfile.open(cfgfilepath);
   
    // read .clock
    while (cfgfile) {
     cfgfile >> tcity >> dst;
     if ((ReadLocationData(tcity, dst)==0))
      ++entries;
    }
    
 return entries;
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
