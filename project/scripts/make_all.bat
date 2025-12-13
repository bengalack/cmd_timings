@echo off
@set start=%time%

@set dir=%cd%

sdcc -v

@echo ***********************************************************************
@echo ***********************************************************************
@cd  project
call .\scripts\make_defines.bat


@echo ***********************************************************************
@echo ***********************************************************************
call .\scripts\make.bat
if %errorlevel% NEQ 0 (
GOTO END_BUGGY
)

@echo ***********************************************************************
@echo ***********************************************************************
@echo Making mainrom
call .\scripts\make_mainrom.bat
if %errorlevel% NEQ 0 (
GOTO END_BUGGY
)

@echo ***********************************************************************
@echo ***********************************************************************
pypy .\scripts\make_summary.py

GOTO END_OK
:END_BUGGY
@echo ***********************************************************************
@echo                                ERROR
@echo ***********************************************************************
:END_OK

@cd %dir%

@echo off
@echo.
@echo ***********************************************************************
@echo ***********************************************************************
For /F "Tokens=*" %%I in ('Time /T') Do Set StrTime=%%I
echo Build all-script ended: %StrTime%

set end=%time%
set options="tokens=1-4 delims=:.,"
for /f %options% %%a in ("%start%") do set start_h=%%a&set /a start_m=100%%b %% 100&set /a start_s=100%%c %% 100&set /a start_ms=100%%d %% 100
for /f %options% %%a in ("%end%") do set end_h=%%a&set /a end_m=100%%b %% 100&set /a end_s=100%%c %% 100&set /a end_ms=100%%d %% 100

set /a hours=%end_h%-%start_h%
set /a mins=%end_m%-%start_m%
set /a secs=%end_s%-%start_s%
set /a ms=%end_ms%-%start_ms%
if %ms% lss 0 set /a secs = %secs% - 1 & set /a ms = 100%ms%
if %secs% lss 0 set /a mins = %mins% - 1 & set /a secs = 60%secs%
if %mins% lss 0 set /a hours = %hours% - 1 & set /a mins = 60%mins%
if %hours% lss 0 set /a hours = 24%hours%
if 1%ms% lss 100 set ms=0%ms%

:: Mission accomplished
set /a totalsecs = %hours%*3600 + %mins%*60 + %secs%
echo Build all-script took %hours%:%mins%:%secs%.%ms% (%totalsecs%.%ms%s total)
@echo on