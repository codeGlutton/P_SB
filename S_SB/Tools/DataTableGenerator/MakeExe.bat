pushd %~dp0
pyinstaller --onefile DataTableGenerator.py
MOVE .\dist\DataTableGenerator.exe .\GenTables.exe
@RD /S /Q .\build
@RD /S /Q .\dist
DEL /S /F /Q .\DataTableGenerator.spec
PAUSE