pushd %~dp0
pyinstaller --onefile HttpPacketGenerator.py
MOVE .\dist\HttpPacketGenerator.exe .\GenHttpPackets.exe
@RD /S /Q .\build
@RD /S /Q .\dist
DEL /S /F /Q .\PacketGenerator.spec
PAUSE