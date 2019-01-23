#!/bin/sh

./protoc --cpp_out=../Src/ GameProtocol_Common.proto || exit 1
./protoc --cpp_out=../Src/ GameProtocol_Account.proto || exit 1
./protoc --cpp_out=../Src/ GameProtocol_MsgID.proto || exit 1
./protoc --cpp_out=../Src/ GameProtocol_World.proto || exit 1
./protoc --cpp_out=../Src/ GameProtocol_CS.proto || exit 1
./protoc --cpp_out=../Src/ GameProtocol_Zone.proto || exit 1
./protoc --cpp_out=../Src/ GameProtocol_RegAuth.proto || exit 1

#记录的关键日志，用于对账
#./protoc --cpp_out=../Src/ ./GameProtocol_LOG.proto || exit 1

#后台DB数据存储的相关结构的定义
./protoc --cpp_out=../Src/ GameProtocol_USERDB.proto || exit 1
 
mv ../Src/GameProtocol_Common.pb.cc ../Src/GameProtocol_Common.pb.cpp
mv ../Src/GameProtocol_Account.pb.cc ../Src/GameProtocol_Account.pb.cpp
mv ../Src/GameProtocol_MsgID.pb.cc ../Src/GameProtocol_MsgID.pb.cpp
mv ../Src/GameProtocol_CS.pb.cc ../Src/GameProtocol_CS.pb.cpp
mv ../Src/GameProtocol_World.pb.cc ../Src/GameProtocol_World.pb.cpp
mv ../Src/GameProtocol_Zone.pb.cc ../Src/GameProtocol_Zone.pb.cpp
mv ../Src/GameProtocol_RegAuth.pb.cc ../Src/GameProtocol_RegAuth.pb.cpp

#后台DB数据存储的相关结构的定义
mv ../Src/GameProtocol_USERDB.pb.cc ../Src/GameProtocol_USERDB.pb.cpp

echo "Success to Build all protos to C++"

#Compile protos to python
#protoc *.proto --python_out=../proto-gen-py -I=. || exit 1
#echo "Successfully built protos to python"
