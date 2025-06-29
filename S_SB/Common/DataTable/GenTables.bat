pushd %~dp0

GenTables.exe --path=../../S_SB/DataTable/ --row_output=TableRow.h
GenTables.exe --path=../../S_SB/DataTable/ --row_output=TableRow.h --prefix=SB

IF ERRORLEVEL 1 PAUSE

COPY /Y TableRow.h "../../S_SB"

COPY /Y SBTableRow.h "../../../C_SB/Source/C_SB/DataTable"

DEL /Q /F *.h

PAUSE