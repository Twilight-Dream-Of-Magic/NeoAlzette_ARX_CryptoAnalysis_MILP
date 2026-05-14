@echo off
setlocal EnableExtensions DisableDelayedExpansion

rem ===== Only edit these paths =====
set "REPO=E:\_ABOUT~1\_CODEP~1\C__~1\NEOALZ~3"
set "MSYS=E:\_ABOUT~1\MSYS2"

rem ===== Derived paths =====
set "SRC=%REPO%\SCIPOP~1.2"
set "BUILD=%REPO%\scipoptsuite-build-static-papilo-short-fresh"
set "MINGW=%MSYS%\mingw64"
set "GXX=%MINGW%\bin\g++.exe"

set "PATH=%MINGW%\bin;%MSYS%\usr\bin;%PATH%"

if not exist "%GXX%" (
    echo ERROR: g++.exe not found:
    echo "%GXX%"
    exit /b 1
)

rem ===== Common compile and link flags =====
set "CFLAGS=-O2 -std=c++20 -static"
set "CFLAGS=%CFLAGS% -I%SRC%\scip\src"
set "CFLAGS=%CFLAGS% -I%BUILD%\scip"
set "CFLAGS=%CFLAGS% -I%SRC%\soplex\src"
set "CFLAGS=%CFLAGS% -I%BUILD%\soplex"
set "CFLAGS=%CFLAGS% -I%SRC%\papilo\src"
set "CFLAGS=%CFLAGS% -I%BUILD%\papilo"
set "CFLAGS=%CFLAGS% -I%MINGW%\include"

set "LIBS=%BUILD%\lib\libscip.a"
set "LIBS=%LIBS% %BUILD%\lib\libsoplex.a"
set "LIBS=%LIBS% %BUILD%\papilo\libpapilo-core.a"
set "LIBS=%LIBS% %MINGW%\lib\libboost_iostreams-mt.a"
set "LIBS=%LIBS% %MINGW%\lib\libboost_program_options-mt.a"
set "LIBS=%LIBS% %MINGW%\lib\libboost_serialization-mt.a"
set "LIBS=%LIBS% %MINGW%\lib\libgmpxx.a"
set "LIBS=%LIBS% %MINGW%\lib\libgmp.a"
set "LIBS=%LIBS% %MINGW%\lib\libmpfr.a"
set "LIBS=%LIBS% %MINGW%\lib\libquadmath.a"
set "LIBS=%LIBS% %MINGW%\lib\libz.a"
set "LIBS=%LIBS% %MINGW%\lib\libbz2.a"
set "LIBS=%LIBS% %MINGW%\lib\libreadline.a"
set "LIBS=%LIBS% %MINGW%\lib\libtermcap.a"
set "LIBS=%LIBS% %MINGW%\lib\libtbb12.dll.a"
set "LIBS=%LIBS% %MINGW%\lib\libwinpthread.a"
set "LIBS=%LIBS% -lws2_32 -lshlwapi -lversion -lpsapi"

call :BuildOne "1" "3" "Differential MILP search" "%REPO%\differential\neoalzette_scip_round_milp_search.cpp" "%REPO%\differential\neoalzette_scip_round_milp_search.exe"
if errorlevel 1 exit /b 1

call :BuildOne "2" "3" "Differential HULL search" "%REPO%\differential\neoalzette_scip_round_hull_search.cpp" "%REPO%\differential\neoalzette_scip_round_hull_search.exe"
if errorlevel 1 exit /b 1

call :BuildOne "3" "3" "Differential HULL multi-core runner" "%REPO%\differential\hull_multiple_thread_runner.cpp" "%REPO%\differential\hull_multiple_thread_runner.exe"
if errorlevel 1 exit /b 1

echo.
echo Differential build OK.
exit /b 0

:BuildOne
set "STEP=%~1"
set "TOTAL=%~2"
set "NAME=%~3"
set "SOURCE=%~4"
set "FINAL_EXE=%~5"

for %%A in ("%FINAL_EXE%") do set "TMP_EXE=%%~dpnA.tmp%%~xA"

echo.
echo [%STEP%/%TOTAL%] ===== Building: %NAME% =====
echo [%STEP%/%TOTAL%] Source:
echo "%SOURCE%"
echo [%STEP%/%TOTAL%] Output:
echo "%FINAL_EXE%"

if not exist "%SOURCE%" (
    echo ERROR: source file not found:
    echo "%SOURCE%"
    exit /b 1
)

del /Q "%TMP_EXE%" >nul 2>nul

echo [%STEP%/%TOTAL%] Compile+link starting...
"%GXX%" %CFLAGS% "%SOURCE%" -o "%TMP_EXE%" %LIBS%
if errorlevel 1 (
    echo ERROR: compiler or linker failed:
    echo "%SOURCE%"
    exit /b 1
)

if not exist "%TMP_EXE%" (
    echo ERROR: temporary executable was not generated:
    echo "%TMP_EXE%"
    exit /b 1
)

for %%F in ("%TMP_EXE%") do (
    if %%~zF LEQ 0 (
        echo ERROR: temporary executable is empty:
        echo "%TMP_EXE%"
        exit /b 1
    )
    echo [%STEP%/%TOTAL%] Temporary executable size: %%~zF bytes
)

move /Y "%TMP_EXE%" "%FINAL_EXE%" >nul
if errorlevel 1 (
    echo ERROR: failed to replace final executable:
    echo "%FINAL_EXE%"
    exit /b 1
)

for %%F in ("%FINAL_EXE%") do echo [%STEP%/%TOTAL%] Final executable size: %%~zF bytes
echo [%STEP%/%TOTAL%] OK: %NAME%
exit /b 0
