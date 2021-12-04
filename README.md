# clock
terminal world clock

clock.cc is a simple world clock for the terminal

it needs to locate two files in your home directory:

astro.dat with format
country  city   longtitude latitude        region
Tanzania Babati -4.2166667   35.75   Africa/DaresSalaam 3

and .clock with format
<city> <daylightsaving correction, according to your location>
Amsterdam 0
CapeTown 0 
NewYorkCity -1
Moscow 0
..and so on until up to 63 entries
the first entry in .clock must be your current location or point of reference
daylightsaving according to location is the relation between your location reference dst and the city in question. example: if your reference (first .clock entry) is now in dst +1 and the city in question is not in daylight saving (like a US city), then the city must be followed by a -1. if your reference is not in dst and the city is, the city must be followed with a 1.
if a city entry is ended with *, for example NewYorkCity* then it will be displayed in bold


keys 
----
space toggles 12/24 hour mode 
enter toggles seconds on/off
ESC will exit clock
------------------------------------------------------------------------------------------------------------------------------------------------------------------
wclock.c is a color terminal world clock

it needs to locate a .wclock file in your root directory with format:
 city_name GMT_offset DST_correction_in_relation_to_your_location(first entry)
 NewYorkCity -4 0
  
..up to 567 entries are allowed, this can easily change though
  
keys 
----
space toggles 12/24 hour mode 
enter toggles seconds on/off
pgup, pgdown move through city pages
ESC will exit clock
