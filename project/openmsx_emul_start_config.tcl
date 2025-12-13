set throttle off
cart "Debug/bin/cmd_cpu.rom" -romtype ASCII16
debug set_watchpoint read_io 0x2E

set speed 100
set limitsprites on

after time 15 {set throttle on}
set horizontal_stretch 320

