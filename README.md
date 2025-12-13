# cmd_timings


ROM FILE

See https://map.grauw.nl/articles/vdp_commands_speed.php#speedtests

https://map.grauw.nl/articles/vdp-vram-timing/vdp-timing.html by Wouter Vermaelen

https://www.msx.org/forum/msx-talk/development/triple-buffer-vs-line-split-doubt?page=2

glass assembler assumed to be found in the root dir.

UNROLLED outis (18 MSX cycles), as this is a very likely scenario.


bunch of build scripts windows scripts
python

glass

uses keyboard ports directly, not bios, so not all keyboards are guaranteed. For example: Russian numbers are not mapped as the rest (Other keyboards => Russian keyboards):

7 => 6
6 => 5
5 => 4
4 => 3
3 => 2
2 => 1
1 => ;
0 => 9



screenmode:
C = screen 5 (default)
D = screen 8

UP = CPU push ON (default)
DOWN = CPU push OFF

ESC = Sprites OFF
RET = Sprites ON (default)

ASCII16 mapper

Final run file uses openmsx

Build file uses powershell for to get some debug output and symbol-files (not needed for building rom file)