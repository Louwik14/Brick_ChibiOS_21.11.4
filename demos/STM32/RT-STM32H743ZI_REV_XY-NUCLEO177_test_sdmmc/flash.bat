@echo off
REM ============================================================
REM  Flash STM32H743 - SWD sans NRST
REM ============================================================

REM --- Toolchain ARM
set PATH=C:\ChibiStudio\tools\GNU Tools ARM Embedded\11.3 2022.08\bin;%PATH%

REM --- OpenOCD (FORCÉ)
set "OPENOCD=C:\openocd\OpenOCD-20250710-0.12.0\bin\openocd.exe"

REM --- Dossier du projet
cd /d "%~dp0"

REM --- Récupération automatique du ELF
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

echo.
echo === Compilation du projet ===
make -j8 all
if errorlevel 1 (
    echo !!!
    echo !!! Erreur de compilation !!!
    echo !!!
    pause
    exit /b 1
)

echo.
echo === Test connexion SWD (sans NRST) ===
"%OPENOCD%" -f interface/stlink.cfg -f target/stm32h7x.cfg ^
  -c "transport select swd; adapter speed 200; init; halt; exit"

if errorlevel 1 (
    echo !!!
    echo !!! ECHEC connexion SWD !!!
    echo !!! Essai a 100 kHz
    echo !!!
    "%OPENOCD%" -f interface/stlink.cfg -f target/stm32h7x.cfg ^
      -c "transport select swd; adapter speed 100; init; halt; exit"
    if errorlevel 1 (
        echo !!!
        echo !!! ECHEC connexion SWD meme a 100 kHz !!!
        echo !!!
        pause
        exit /b 1
    )
)

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

echo.
echo === FLASH OK ===
pause
