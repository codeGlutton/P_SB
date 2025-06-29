pushd %~dp0

protoc.exe -I=./ --cpp_out=./ ./Enum.proto
protoc.exe -I=./ --cpp_out=./ ./Struct.proto
protoc.exe -I=./ --cpp_out=./ ./Protocol.proto
protoc.exe -I=./ --cpp_out=./ ./HttpProtocol.proto
protoc.exe -I=./ --cpp_out=./ ./RedisProtocol.proto

protoc.exe -I=./ --csharp_out=./ ./Enum.proto
protoc.exe -I=./ --csharp_out=./ ./Struct.proto
protoc.exe -I=./ --csharp_out=./ ./Protocol.proto
protoc.exe -I=./ --csharp_out=./ ./HttpProtocol.proto
protoc.exe -I=./ --csharp_out=./ ./RedisProtocol.proto

GenPackets.exe --path=./Protocol.proto --output=ClientPacketHandler --recv=C_ --send=S_
GenPackets.exe --path=./Protocol.proto --output=ServerPacketHandler --recv=S_ --send=C_
GenHttpPackets.exe --path=./HttpProtocol.proto --output=BaseProtobufController --recv=REQ_ --send=RES_ --lang=c#
GenHttpPackets.exe --path=./HttpProtocol.proto --output=ServerHttpPacketHandler --recv=RES_ --send=REQ_ --lang=c++
IF ERRORLEVEL 1 PAUSE

COPY /Y Enum.pb.h "../../../S_SB"
COPY /Y Enum.pb.cc "../../../S_SB"
COPY /Y Struct.pb.h "../../../S_SB"
COPY /Y Struct.pb.cc "../../../S_SB"
COPY /Y Protocol.pb.h "../../../S_SB"
COPY /Y Protocol.pb.cc "../../../S_SB"
COPY /Y RedisProtocol.pb.h "../../../S_SB"
COPY /Y RedisProtocol.pb.cc "../../../S_SB"
COPY /Y ClientPacketHandler.h "../../../S_SB"
COPY /Y CopyList/ByteConverters.h "../../../S_SB"

COPY /Y Enum.pb.h "../../../../C_SB/Source/C_SB/Protocol"
COPY /Y Enum.pb.cc "../../../../C_SB/Source/C_SB/Protocol"
COPY /Y Struct.pb.h "../../../../C_SB/Source/C_SB/Protocol"
COPY /Y Struct.pb.cc "../../../../C_SB/Source/C_SB/Protocol"
COPY /Y Protocol.pb.h "../../../../C_SB/Source/C_SB/Protocol"
COPY /Y Protocol.pb.cc "../../../../C_SB/Source/C_SB/Protocol"
COPY /Y HttpProtocol.pb.h "../../../../C_SB/Source/C_SB/Protocol"
COPY /Y HttpProtocol.pb.cc "../../../../C_SB/Source/C_SB/Protocol"
COPY /Y ServerPacketHandler.h "../../../../C_SB/Source/C_SB/Main"
COPY /Y ServerHttpPacketHandler.h "../../../../C_SB/Source/C_SB/Main"
COPY /Y CopyList/ByteConverters.h "../../../../C_SB/Source/C_SB/Utils"

COPY /Y Enum.cs "../../../../W_SB/WS_SB/Protobuf"
COPY /Y Struct.cs "../../../../W_SB/WS_SB/Protobuf"
COPY /Y HttpProtocol.cs "../../../../W_SB/WS_SB/Protobuf"
COPY /Y RedisProtocol.cs "../../../../W_SB/WS_SB/Protobuf"
COPY /Y BaseProtobufController.cs "../../../../W_SB/WS_SB/Controllers"

DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h
DEL /Q /F *.cs

PAUSE