// analogue clock
#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define version 3.0
#define M_PI 3.14159265358979323846

char *hours[]={ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12" };
int hourpositions[12][2] = { 0 };
double hourangles[12] = { 0 };
int midx, midy, radius;
int SCHEMES[5][4] = { 
    { BROWN, RED, BLUE, CYAN },
    { YELLOW, GREEN, MAGENTA, BROWN },
    { MAGENTA, BROWN, BLUE, CYAN },
    { RED, MAGENTA, GREEN, BLUE },
    { WHITE, WHITE, WHITE, WHITE }
};
int scheme=0;

typedef struct
 {
   int x;
   int y;
 } Pointer;
Pointer ph, pm, ps;
enum { HOUR = 0, MINUTE, SECOND };
 
// function declarations
void drawscreen(char *buffer);
void setuphours(void);
void setuppointer(struct tm* ttime, Pointer* tpointer, int type);
int sdlgetch(SDL_Event *tevent);

int main(int argc, char *argv[])
{
  int gd = DETECT, gm;
  int i, dstadjust=0, run=1, c;
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];
  sprintf(buffer, "Analogue Clock %.2f", version);
  if (argc==2)
   strcpy(buffer, argv[1]);
  
  // SDL keyboard event setup
  SDL_Event event;
  SDL_EnableKeyRepeat(0,0);

   // initiliaze, draw the screen
   initgraph(&gd, &gm, NULL);
   midx = getmaxx() / 2;
   midy = getmaxy() / 2;
   radius=midx-100;
   setuphours();
   drawscreen(buffer);
   
   // watch loop
   while (run) {
    
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    if (dstadjust) {
     timeinfo->tm_min=timeinfo->tm_min+dstadjust;
     mktime(timeinfo);
   }
    
    setuppointer(timeinfo, &ph, HOUR);
    setuppointer(timeinfo, &pm, MINUTE);
    setuppointer(timeinfo, &ps, SECOND);
    
    // draw time pointers
    setcolor(SCHEMES[scheme][1]);
    line(midx, midy, ph.x, ph.y);
    setcolor(SCHEMES[scheme][2]);
    line(midx, midy, pm.x, pm.y);
    setcolor(SCHEMES[scheme][3]);
    line(midx, midy, ps.x, ps.y);
    sleep(1);
    // erase pointers
    setcolor(BLACK);
    line(midx, midy, ph.x, ph.y);
    line(midx, midy, pm.x, pm.y);
    line(midx, midy, ps.x, ps.y);

    // poll keyboard
    if ( (c=sdlgetch(&event)) ) {
     switch (c) {

      case SDLK_ESCAPE:
       run=0;
      break;
      case SDLK_KP_PLUS:
       dstadjust+=30;
      break;
      case SDLK_KP_MINUS:
       dstadjust-=30;
      break;
      case SDLK_TAB:
       ++scheme;
       if (scheme>4)
        scheme=0;
       drawscreen(buffer);
      break;
      default:
       break;
      
     }
    }
   }

   closegraph();

  return 0;
}

// draw basic screen
void drawscreen(char *buffer)
{
   cleardevice();
   setcolor(SCHEMES[scheme][0]);
   circle(midx, midy, radius); // clock frame
   outtextxy(midx-(strlen(buffer)*3), 0, buffer);
   for (int i=0;i<12;i++)   
    outtextxy(hourpositions[i][0], hourpositions[i][1], hours[i]);
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

// SDL getch
int sdlgetch(SDL_Event *tevent)
{
  int c=0;

    while ( SDL_PollEvent( tevent ) ) {
        
    switch( tevent->type ){
      case SDL_KEYDOWN:
       c=tevent->key.keysym.sym;
      break;
      case SDL_KEYUP:
       // nothing
      break;
      default:
       break;
     }
     
    }
    
 return c;
}
