@echo off
cd /d "%~dp0"
tasklist /FI "IMAGENAME eq steam.exe" /NH | findstr /I /L "steam.exe" >nul
if errorlevel 1 (
  echo Steam est ferme : sans lui, le jeu se ferme avant que le mod se charge.
  echo Ouvre Steam, attends qu'il soit connecte, puis appuie sur une touche ici.
  echo.
  pause >nul
)
echo Lancement de Mewgenics avec le mod coop...
echo (si Steam relance le jeu, ca peut prendre une minute ou deux)
echo.
mgmp_loader.exe > loader.txt 2>&1
type loader.txt
echo.
echo Le message du loader est aussi enregistre dans loader.txt
pause
