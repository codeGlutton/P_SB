pushd %~dp0

GenTables.exe --path=../../S_SB/DataTable/ --output=TableRow.h --project=SB --isUE=False
GenTables.exe --path=../../S_SB/DataTable/ --output=TableRow.h --project=SB --isUE=True --table=DataTable.h

IF ERRORLEVEL 1 PAUSE

COPY /Y TableRow.h "../../S_SB"

COPY /Y SBTableRow.h "../../../C_SB/Source/C_SB/DataTable"
COPY /Y SBDataTable.h "../../../C_SB/Source/C_SB/DataTable"

DEL /Q /F *.h

PAUSE