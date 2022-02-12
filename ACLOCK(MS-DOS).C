// analogue clock
#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <math.h>
#include <time.h>
#include <conio.h>

#define version 1.0
#define M_PI 3.14159265358979323846
#define ESC 27
#define MAXLINE 512

char *hours[]={ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12" };
int hourpositions[12][2] = { 0 };
double hourangles[12] = { 0 };
int midx, midy, radius;

typedef struct
 {
   int x;
   int y;
 } Pointer;
Pointer ph, pm, ps;
enum { HOUR = 0, MINUTE, SECOND };

void setuphours(void);
void setuppointer(struct tm* ttime, Pointer* tpointer, int type);
char *setupfilepathtoexecutable(const char *argument);

int main(int argc, char *argv[])
{
  int gd = DETECT, gm;
  int i, c, mincorrection=0;
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[MAXLINE];

   // set up screen
//   initgraph(&gd, &gm, "..\\bgi");
   initgraph(&gd, &gm, setupfilepathtoexecutable(argv[0]));
   midx = getmaxx() / 2;
   midy = getmaxy() / 2;
   radius=midx-(midx/3);
   setcolor(BROWN);
   circle(midx, midy, radius);
   setuphours();
   for (i=0;i<12;i++)
    outtextxy(hourpositions[i][0], hourpositions[i][1], hours[i]);
   sprintf(buffer, "Analogue Clock %.2f", version);
   if (argc==2)
    strcpy(buffer, argv[1]);
   outtextxy(midx-(strlen(buffer)*4), 1, buffer);

   // watch loop
   while (c!=ESC) {

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    timeinfo->tm_min = timeinfo->tm_min + mincorrection;
    mktime(timeinfo);

    setuppointer(timeinfo, &ph, HOUR);
    setuppointer(timeinfo, &pm, MINUTE);
    setuppointer(timeinfo, &ps, SECOND);

    // draw time pointers
    setcolor(RED);
    line(midx, midy, ph.x, ph.y);
    setcolor(BLUE);
    line(midx, midy, pm.x, pm.y);
    setcolor(CYAN);
    line(midx, midy, ps.x, ps.y);
    sleep(1);
    // erase pointers
    setcolor(BLACK);
    line(midx, midy, ph.x, ph.y);
    line(midx, midy, pm.x, pm.y);
    line(midx, midy, ps.x, ps.y);

    // read keyboard
    if (kbhit()) {
     c=getch();
     switch (c) {
      case '+':
       mincorrection+=30;
      break;
      case '-':
       mincorrection-=30;
      break;
      default:
     break;
    }
   }
   }

   closegraph();

  return 0;
}

// set up hour positions, angles
void setuphours(void)
{
  double x,y, angle=300;
  int i;

   // set hour positions, angles
   for (i=0;i<12;i++) {
    hourangles[i]=angle;
    angle+=30;
    if (angle>=360)
     angle=0;
   }
   for (i=0;i<12;i++) {
    x = midx + radius * cos(hourangles[i] * M_PI / 180);
    y = midy + radius * sin(hourangles[i] * M_PI / 180);
    // corrections
    if (i<5)
     x-=12;
    if (i==4)
     x-=7;
    if (i==5)
     y-=10;
    if (i>5 && i<11)
     x+=5;
    if (i==6)
     x+=5;
    hourpositions[i][0]=(int) x;
    hourpositions[i][1]=(int) y;
    }

}

// set up pointer positions
void setuppointer(struct tm* ttime, Pointer* tpointer, int type)
{
  double angle;
  int pcorrections[3] = { 75, 35, 20 };

   switch (type) {
    case HOUR:
     if (ttime->tm_hour>12)
      ttime->tm_hour-=12;
     angle=hourangles[ttime->tm_hour-1]+(ttime->tm_min*0.5);
    break;
    case MINUTE:
     angle=(ttime->tm_min*6)-90;
    break;
    case SECOND:
     angle=(ttime->tm_sec*6)-90;
   break; }

    tpointer->x = (int) midx + (radius-pcorrections[type]) * cos(angle * M_PI / 180);
    tpointer->y = (int) midy + (radius-pcorrections[type]) * sin(angle * M_PI / 180);

}

// setup filepath to .dat file
char *setupfilepathtoexecutable(const char *argument)
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

 return tfilename;
}