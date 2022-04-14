// wclock manipulate data file dst entries

// C
#include <unistd.h>
#include <stdlib.h>
// c++
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


using namespace std;

const int MAXLINE=80;
void showusage();

int main(int argc, char *argv[])
{
  string city, datafilein, datafileout;
  double offset;
  int dst, newdst, count=0, opt, i;
  enum { CHANGEZEROTOONE=0, CHANGEONETOZERO, EXCHANGEONEANDZERO, REVERSEMINUSONEWITHONE };
  ifstream cityfile;
  ofstream newcityfile;
  vector<int> operations;
  vector<string> Cities;
  
   // parse command line
   while ( (opt = getopt(argc, argv, ":qwero:")) != -1 ) {
    switch (opt) {
     case 'q':
      operations.push_back(CHANGEZEROTOONE);
     break;
     case 'w':
      operations.push_back(CHANGEONETOZERO);
     break;
     case 'e':
      operations.push_back(EXCHANGEONEANDZERO);
     break;
     case 'r':
      operations.push_back(REVERSEMINUSONEWITHONE);
     break;
     case 'o':
      datafileout=optarg;
     break;
     case '?':
      showusage();
     break;
    }
   }
   if ( optind == 1 || optind == argc )
    showusage();
  
   // open files
   datafilein=argv[optind++];
   cityfile.open(datafilein.c_str());
   if ( datafileout.empty() )
    datafileout=datafilein + "_";
   newcityfile.open(datafileout.c_str());
   if (!cityfile || !newcityfile)
    showusage();
   // specific cities
   for ( ;optind<argc;optind++)
    Cities.push_back(argv[optind]);

     // manipulate DSTs
     while (cityfile) {
      
      cityfile >> city >> offset >> dst;
      
      newdst=dst;
      for (i=0;i<operations.size();i++) {
       switch ( operations[i] ) {
        case CHANGEZEROTOONE:
         if ( dst ==  0 )
          newdst=1;
         break;
         case CHANGEONETOZERO:
          if ( dst == 1 )
           newdst=0;
         break;
         case EXCHANGEONEANDZERO:
          if ( dst == 0 ) {
           newdst=1;
           break; 
          }
          if ( dst == 1)
           newdst=0;
         break;
         case REVERSEMINUSONEWITHONE:
          newdst=dst*-1;
         break;
         default:
        break;
       }
      }
      if (!cityfile)
       break;
      for (i=0;i<Cities.size();i++)
       if ( Cities[i] == city )
        break;
      if ( Cities.size() && i == Cities.size() ) // no match for requested city
       newdst=dst;
      newcityfile << city << " " << offset << " " << newdst << endl;
      if ( dst != newdst )
       ++count;
      
     }
      
    // all done
    cityfile.close();
    newcityfile.close();
    cout << "data resaved in " << datafileout << " updated " << count << " cities" << endl;
    
 return EXIT_SUCCESS;
}

// show usage
void showusage()
{
  printf("Usage:\n util [options] [data file] [cities ...] \n\nA clock data file DST manipulator.\n\nOptions:\n");
  printf(" -q\t\tchange 0 to 1\n -w\t\tchange 1 to 0\n -e\t\texchange 0<->1\n -r\t\treverse -1<->1\n -o<filename>\toutput to specific file\n      --help\tdisplay this help\n\nDistributed under the GNU Public licence.\n");
  exit (EXIT_FAILURE);
}
