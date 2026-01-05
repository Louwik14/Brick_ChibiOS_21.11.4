@echo off
REM ============================================================
REM  STM32H743 - BUILD + FLASH + DEBUG (OpenOCD + GDB)
REM ============================================================

REM ------------------------------------------------------------
REM  Toolchain ARM
REM ------------------------------------------------------------
set PATH=C:\ChibiStudio\tools\GNU Tools ARM Embedded\11.3 2022.08\bin;%PATH%

REM ------------------------------------------------------------
REM  OpenOCD
REM ------------------------------------------------------------
set "OPENOCD=C:\openocd\OpenOCD-20250710-0.12.0\bin\openocd.exe"

REM ------------------------------------------------------------
REM  Dossier du projet
REM ------------------------------------------------------------
cd /d "%~dp0"

REM ------------------------------------------------------------
REM  Récupération du ELF
REM ------------------------------------------------------------
set "ELF="
for %%F in ("%~dp0build\*.elf") do set "ELF=%%~fF"

if not defined ELF (
    echo !!!
    echo !!! Aucun fichier ELF trouve dans build\ !!!
    echo !!!
    pause
    exit /b 1
)

echo ELF utilise :
echo %ELF%

REM ------------------------------------------------------------
REM  Compilation
REM ------------------------------------------------------------
echo.
echo === Compilation ===
make -j8 all
if errorlevel 1 (
    echo !!!
    echo !!! Erreur de compilation !!!
    echo !!!
    pause
    exit /b 1
)

REM ------------------------------------------------------------
REM  Flash (optionnel mais garde)
REM ------------------------------------------------------------
echo.
echo === Flash STM32H743 ===
"%OPENOCD%" -f interface/stlink.cfg -f target/stm32h7x.cfg ^
  -c "transport select swd; adapter speed 200; init; halt; program {%ELF%} verify reset exit"

if errorlevel 1 (
    echo !!!
    echo !!! ECHEC FLASH !!!
    echo !!!
    pause
    exit /b 1
)

REM ------------------------------------------------------------
REM  Lancement OpenOCD (serveur debug)
REM ------------------------------------------------------------
echo.
echo === Lancement OpenOCD ===
start "OpenOCD STM32H7" cmd /k ^
""%OPENOCD%" -f interface/stlink.cfg -f target/stm32h7x.cfg ^
 -c "transport select swd; adapter speed 200; init; halt""

REM ------------------------------------------------------------
REM  Attente courte pour OpenOCD
REM ------------------------------------------------------------
timeout /t 2 >nul

REM ------------------------------------------------------------
REM  Lancement GDB
REM ------------------------------------------------------------
echo.
echo === Lancement GDB ===
start "GDB STM32H7" cmd /k ^
"arm-none-eabi-gdb "%ELF%" ^
 -ex "set confirm off" ^
 -ex "target extended-remote localhost:3333" ^
 -ex "monitor reset halt" ^
 -ex "load" ^
 -ex "break main""

echo.
echo === DEBUG PRET ===
echo Fenetre 1 : OpenOCD
echo Fenetre 2 : GDB
echo.
pause
