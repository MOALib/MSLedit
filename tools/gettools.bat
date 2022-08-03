@echo off
set F="%RANDOM%gte.bat"
type gettools.exec > "%F%"
cmd /c "%F%"
rm "%F%"