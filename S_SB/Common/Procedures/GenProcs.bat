pushd %~dp0

GenProcs.exe --path=../../S_SB/GameDB.xml --output=GenProcedures.h

IF ERRORLEVEL 1 PAUSE

COPY /Y GenProcedures.h "../../S_SB"

DEL /Q /F *.h

PAUSE