# clock
terminal world clock

clock is a simple world clock for the terminal

it needs to locate two files in your home directory:

astro.dat with format
country  city   longtitude latitude        region
Tanzania Babati -4.2166667   35.75   Africa/DaresSalaam 3

and clock.cfg with format
<city> <daylightsaving correction, according to your location>
Amsterdam 0
CapeTown 0 
NewYorkCity -1
Moscow 0
..and so on until up to 63 entries
the first entry in clock.cfg must be your current location or point of reference
if a city entry is ended with *, for example NewYorkCity* then it will be displayed in bold


keys 
----
space toggles 12/24 hour mode 
enter toggles seconds on/off
ESC will exit clock
