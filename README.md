# clock
terminal world clock

wclock.c is a world clock for the ncurses terminal.

it needs to locate a .wclock file in your root directory with format:

 city_name GMT_offset DST_correction_in_relation_to_your_location(first entry)
 
 NewYorkCity -4 0
  
build with cc wclock wclock.c -lncurses
  
keys 
 
space toggles 12/24 hour mode 

enter toggles seconds on/off

pgup, pgdown move through city pages

ESC will exit wclock
