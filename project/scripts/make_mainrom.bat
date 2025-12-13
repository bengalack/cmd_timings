@echo off
java -jar glass.jar -I . -I Debug/objs -I content/audio/o4s_etc -I content/gfx -I src/segments src/mainrom.s Debug/bin/cmd_cpu.rom Debug/objs/mainrom.sym

@REM if %errorlevel% NEQ 0 (
@REM echo FAIL!
@REM EXIT %errorlevel%
@REM )
    