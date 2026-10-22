@echo off
cd /d "%~dp0"
echo Lancement de Mewgenics avec le mod coop...
echo (si Steam relance le jeu, ca peut prendre une minute ou deux)
echo.
mgmp_loader.exe > loader.txt 2>&1
type loader.txt
echo.
echo Le message du loader est aussi enregistre dans loader.txt
pause
