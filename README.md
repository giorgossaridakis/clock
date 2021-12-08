# clock
terminal world clock

wclock.c is a world clock for the ncurses terminal.

it needs to locate a .wclock file in your root directory with format:

 city_name GMT_offset DST_correction_in_relation_to_your_location(first entry)
 
 NewYorkCity -4 0
 
if a city is ended with * it will be shown in red in a color terminal or in bold in a non color one.

wclock assumes your first city entry as your local time point of reference. you can change this by ending any other city in the .wclock file with &. this can be done once. in my sample .wclock, Sofia Bulgaria is the point of reference.
  
build with cc wclock wclock.c -lncurses
  
keys 

<*> apply daylight saving to cities other than reference city or not

! toggles seconds on/off

space toggles 12/24 hour mode 
 
any alphanumerical key will add to the city you wish to locate, enter will look for it

pgup, pgdown move through city pages

ESC will exit wclock
