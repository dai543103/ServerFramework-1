#!/bin/sh

#本脚本负责服务器运营环境安装、部署和更新

#修改为脚本所在目录
PROJECT_HOME=`pwd`

#脚本使用帮助文档
Usage()
{
	echo "Usage: ./ProjectTool.sh -I <SERVERIP> <ZONE_PORT> <REGAUTH_PORT> <WORLDID> <ZONEID> <DBUser> <DBPasswd>"
	echo "                        --Init Server Project"
	echo "       ./ProjectTool.sh -D"
	echo "                        --Deploy Server Project to local env"
	echo "       ./ProjectTool.sh -K"
	echo "                        --Kill All Servers of local env"
	echo "       ./ProjectTool.sh -S"
	echo "                        --Restart All Servers of local env"

	echo "       ./ProjectTool.sh -R"
	echo "                        --Reload All Servers of local env"

	exit 1
}

#-I 选项： 初始化服务器
Init_Server()
{
	if [ $# -ne 7 ]
	then
		echo "Usage: ./ProjectTool.sh -I <SERVERIP> <ZONE_PORT> <REGAUTH_PORT> <WORLDID> <ZONEID> <DBUser> <DBPasswd>"
		exit 1
	fi

	#先进行整个项目的初始化

	#如果原来的运行时环境存在，先清空
	if [ -d "~/RuntimeEnv" ]
	then
		rm -rf ~/RuntimeEnv
	fi

	#建立运行时环境
	mkdir -p ~/RuntimeEnv

	cp ../* ~/RuntimeEnv/ -rf

	#设置ZoneGatewayServer绑定的服务器IP
	perl -i -p -e 's/'#ZONE_GATEWAY_IP#'/'$1'/g' ~/RuntimeEnv/World/Zone/ZoneGatewayServer/conf/LotusListener.tcm ~/RuntimeEnv/Global/RegAuthServer/conf/ServerList.tcm

	#设置ZoneGatewayServer绑定的服务器端口
	perl -i -p -e 's/'#ZONE_GATEWAY_PORT#'/'$2'/g' ~/RuntimeEnv/World/Zone/ZoneGatewayServer/conf/LotusListener.tcm ~/RuntimeEnv/Global/RegAuthServer/conf/ServerList.tcm

	#设置RegAuthGatewayServer绑定的服务器IP
	perl -i -p -e 's/'#REGAUTH_GATEWAY_IP#'/'$1'/g' ~/RuntimeEnv/Global/RegAuthGatewayServer/conf/LotusListener.tcm
	
	#设置RegAuthGatewayServer绑定的服务器端口
	perl -i -p -e 's/'#REGAUTH_GATEWAY_PORT#'/'$3'/g' ~/RuntimeEnv/Global/RegAuthGatewayServer/conf/LotusListener.tcm

	#设置所有配置文件中的服务器通信BUS
	perl -i -p -e 's/'#WORLDID#'/'$4'/g' ~/RuntimeEnv/World/Zone/ZoneServer/conf/GameServer.tcm ~/RuntimeEnv/World/RoleDBServer/conf/GameServer.tcm ~/RuntimeEnv/World/WorldServer/conf/GameServer.tcm ~/RuntimeEnv/World/NameDBServer/conf/GameServer.tcm ~/RuntimeEnv/World/RechargeServer/conf/GameServer.tcm ~/RuntimeEnv/World/ExchangeServer/conf/GameServer.tcm ~/RuntimeEnv/World/LogServer/conf/GameServer.tcm

	perl -i -p -e 's/'#ZONEID#'/'$5'/g' ~/RuntimeEnv/World/Zone/ZoneServer/conf/GameServer.tcm ~/RuntimeEnv/World/WorldServer/conf/GameServer.tcm

	perl -i -p -e 's/'#SERVERIP#'/'$1'/g' ~/RuntimeEnv/World/Zone/ZoneServer/conf/GameServer.tcm ~/RuntimeEnv/Global/RegAuthServer/conf/GameServer.tcm ~/RuntimeEnv/Global/RegAuthDBServer/conf/GameServer.tcm ~/RuntimeEnv/World/RoleDBServer/conf/GameServer.tcm ~/RuntimeEnv/World/WorldServer/conf/GameServer.tcm ~/RuntimeEnv/World/NameDBServer/conf/GameServer.tcm ~/RuntimeEnv/World/RechargeServer/conf/GameServer.tcm ~/RuntimeEnv/World/ExchangeServer/conf/GameServer.tcm ~/RuntimeEnv/World/LogServer/conf/GameServer.tcm

	#BUS端口，生成规则为： 10000 + WorldID * 1000 + FuncID(Server端) * 100 + ZoneID*10 + InstanceID
	#RegAuth2DB
	RegAuth2DBPort=`expr 10000 + 0 \* 1000 + 6 \* 100 + 0 \* 10 + 1`
	perl -i -p -e 's/'#REGAUTH2DB#'/'$RegAuth2DBPort'/g' ~/RuntimeEnv/Global/RegAuthServer/conf/GameServer.tcm ~/RuntimeEnv/Global/RegAuthDBServer/conf/GameServer.tcm

	#World2NameDB
	World2NameDBPort=`expr 10000 + $4 \* 1000 + 7 \* 100 + 0 \* 10 + 1`
	perl -i -p -e 's/'#WORLD2NAMEDB#'/'$World2NameDBPort'/g' ~/RuntimeEnv/World/WorldServer/conf/GameServer.tcm ~/RuntimeEnv/World/NameDBServer/conf/GameServer.tcm

	#World2RoleDB
	World2RoleDBPort=`expr 10000 + $4 \* 1000 + 3 \* 100 + 0 \* 10 + 1`
	perl -i -p -e 's/'#WORLD2ROLEDB#'/'$World2RoleDBPort'/g' ~/RuntimeEnv/World/WorldServer/conf/GameServer.tcm ~/RuntimeEnv/World/RoleDBServer/conf/GameServer.tcm

	#Zone2World
	Zone2WorldPort=`expr 10000 + $4 \* 1000 + 1 \* 100 + 1 \* 10 + 1`
	perl -i -p -e 's/'#ZONE2WORLD#'/'$Zone2WorldPort'/g' ~/RuntimeEnv/World/WorldServer/conf/GameServer.tcm ~/RuntimeEnv/World/Zone/ZoneServer/conf/GameServer.tcm

	#Recharge2World
	Recharge2WorldPort=`expr 10000 + $4 \* 1000 + 8 \* 100 + 0 \* 10 + 1`
	perl -i -p -e 's/'#RECHARGE2WORLD#'/'$Recharge2WorldPort'/g' ~/RuntimeEnv/World/WorldServer/conf/GameServer.tcm ~/RuntimeEnv/World/RechargeServer/conf/GameServer.tcm

	#World2Exchange
	World2ExchangePort=`expr 10000 + $4 \* 1000 + 10 \* 100 + 0 \* 10 + 1`
	perl -i -p -e 's/'#WORLD2EXCHANGE#'/'$World2ExchangePort'/g' ~/RuntimeEnv/World/WorldServer/conf/GameServer.tcm ~/RuntimeEnv/World/ExchangeServer/conf/GameServer.tcm

	#World2LogServer
	World2LogServerPort=`expr 10000 + $4 \* 1000 + 11 \* 100 + 0 \* 10 + 1`
	perl -i -p -e 's/'#WORLD2LOGSERVER#'/'$World2LogServerPort'/g' ~/RuntimeEnv/World/WorldServer/conf/GameServer.tcm ~/RuntimeEnv/World/LogServer/conf/GameServer.tcm

	#设置游戏使用DB的详细信息
	perl -i -p -e 's/'#DB_IP#'/'$1'/g' ~/RuntimeEnv/World/RoleDBServer/conf/DBMSConf.xml ~/RuntimeEnv/World/NameDBServer/conf/DBMSConf.xml ~/RuntimeEnv/Global/RegAuthDBServer/conf/DBMSConf_UniqUin.xml ~/RuntimeEnv/Global/RegAuthDBServer/conf/DBMSConf.xml ~/RuntimeEnv/World/LogServer/conf/DBMSConf.xml ~/RuntimeEnv/World/ExchangeServer/conf/DBMSConf.xml
	perl -i -p -e 's/'#DB_USER#'/'$6'/g' ~/RuntimeEnv/World/RoleDBServer/conf/DBMSConf.xml ~/RuntimeEnv/World/NameDBServer/conf/DBMSConf.xml ~/RuntimeEnv/Global/RegAuthDBServer/conf/DBMSConf_UniqUin.xml ~/RuntimeEnv/Global/RegAuthDBServer/conf/DBMSConf.xml ~/RuntimeEnv/World/LogServer/conf/DBMSConf.xml ~/RuntimeEnv/World/ExchangeServer/conf/DBMSConf.xml
	perl -i -p -e 's/'#DB_PASSWD#'/'$7'/g' ~/RuntimeEnv/World/RoleDBServer/conf/DBMSConf.xml ~/RuntimeEnv/World/NameDBServer/conf/DBMSConf.xml ~/RuntimeEnv/Global/RegAuthDBServer/conf/DBMSConf_UniqUin.xml ~/RuntimeEnv/Global/RegAuthDBServer/conf/DBMSConf.xml ~/RuntimeEnv/World/LogServer/conf/DBMSConf.xml ~/RuntimeEnv/World/ExchangeServer/conf/DBMSConf.xml
	
	#World中的所有分区服务器在WorldServer/conf下面，是需要手动配置的

	#设置所有启动脚本的启动参数
	cd ~/RuntimeEnv/
	perl -i -p -e 's/'#WORLDID#'/'$4'/g' World/RoleDBServer/admin/ScriptFrameSvr.sh
	perl -i -p -e 's/'#WORLDID#'/'$4'/g' World/WorldServer/admin/ScriptFrameSvr.sh
	perl -i -p -e 's/'#WORLDID#'/'$4'/g' World/Zone/ZoneServer/admin/ScriptFrameSvr.sh
	perl -i -p -e 's/'#ZONEID#'/'$5'/g' World/Zone/ZoneServer/admin/ScriptFrameSvr.sh
	perl -i -p -e 's/'#WORLDID#'/'$4'/g' World/NameDBServer/admin/ScriptFrameSvr.sh	

	cd -

	find ~/RuntimeEnv/ -name '*svn*' | xargs rm -rf

	find ~/RuntimeEnv/ -name '*sh' | xargs chmod a+x
	find ~/RuntimeEnv/ -name '*dbg' | xargs chmod a+x

	echo "Success to Init Server development env......."
	echo "Have a good time......"
	echo "Finished!"

	exit 1
}

#-D 选项： 部署服务器安装包
Deploy_Server()
{
	CURRENT_DIR=`pwd`

	#先停掉所有的Server
	Kill_Server
	
	cd $CURRENT_DIR

	#直接将安装包解压到对应目录即可
	#todo jasonxiong 后面考虑增加版本号参数，可以安装对应的版本

	cp ../* ~/RuntimeEnv/ -rf

	find ~/RuntimeEnv/ -name *dbg | xargs chmod a+x

	#重启所有的服务
	Start_Server

	echo "Success to deploy server runtime env!!!"

	exit 0	
}

#-S 选项： 重新启动所有的服务
Start_Server()
{
	ulimit -c unlimited
	
	#重新启动所有的服务
	cd ~/RuntimeEnv

	#启动RechargeServer
	cd World/RechargeServer/admin
	./restart.sh
	cd ../../../

	#启动ExchangeServer
	cd World/ExchangeServer/admin
	./restart.sh
	cd ../../../

	#启动LogServer
	cd World/LogServer/admin
	./restart.sh
	cd ../../../

	#启动WorldServer
	cd World/WorldServer/admin
	./restart.sh
	cd ../../../

	#启动RoleDBServer
	cd World/RoleDBServer/admin
	./restart.sh
	cd ../../../

	#启动NameDBServer
	cd World/NameDBServer/admin
	./restart.sh
	cd ../../../

	#启动ZoneGatewayServer
	cd World/Zone/ZoneGatewayServer/admin
	./restart.sh
	cd ../../../..

	#启动ZoneServer
	cd World/Zone/ZoneServer/admin
	./restart.sh
	cd ../../../..

	#启动RegAuthGatewayServer
	cd Global/RegAuthGatewayServer/admin
	./restart.sh
	cd ../../..
	
	#启动RegAuthServer
	cd Global/RegAuthServer/admin
	./restart.sh
	cd ../../..

	#启动RegAuthDBServer
	cd Global/RegAuthDBServer/admin
	./restart.sh
	cd ../../..

	echo "Success to restart all servers!!!"
}

#-R 选项： Reload所有的服务，重新加载配置
Reload_Server()
{
	#Reload所有的服务配置
	cd ~/RuntimeEnv

	#重新加载RechargeServer
	cd World/RechargeServer/admin
	./reload.sh
	cd ../../../

	#重新加载ExchangeServer
	cd World/ExchangeServer/admin
	./reload.sh
	cd ../../../

	#重新加载LogServer
	cd World/LogServer/admin
	./reload.sh
	cd ../../../

	#重新加载WorldServer
	cd World/WorldServer/admin
	./reload.sh
	cd ../../../

	#重新加载RoleDBServer
	cd World/RoleDBServer/admin
	./reload.sh
	cd ../../../

	#重新加载NameDBServer
	cd World/NameDBServer/admin
	./reload.sh
	cd ../../../

	#重新加载ZoneGatewayServer
	cd World/Zone/ZoneGatewayServer/admin
	./reload.sh
	cd ../../../..

	#重新加载ZoneServer
	cd World/Zone/ZoneServer/admin
	./reload.sh
	cd ../../../..

	#重新加载RegAuthGatewayServer
	cd Global/RegAuthGatewayServer/admin
	./reload.sh
	cd ../../..
	
	#重新加载RegAuthServer
	cd Global/RegAuthServer/admin
	./reload.sh
	cd ../../..

	#重新加载RegAuthDBServer
	cd Global/RegAuthDBServer/admin
	./reload.sh
	cd ../../..

	echo "Success to reload all servers!!!"
}

#-K 选项： 停止所有的服务
Kill_Server()
{
	#停止所有的服务
	cd ~/RuntimeEnv

	#停止ZoneGatewayServer
	cd World/Zone/ZoneGatewayServer/admin
	./stop.sh
	cd ../../../..

	#停止RechargeServer
	cd World/RechargeServer/admin
	./stop.sh
	cd ../../../

	#停止ExchangeServer
	cd World/ExchangeServer/admin
	./stop.sh
	cd ../../../

	#停止LogServer
	cd World/LogServer/admin
	./stop.sh
	cd ../../../

	#停止WorldServer
	cd World/WorldServer/admin
	./stop.sh
	cd ../../../

	#停止RoleDBServer
	cd World/RoleDBServer/admin
	./stop.sh
	cd ../../../

	#停止NameDBServer
	cd World/NameDBServer/admin
	./stop.sh
	cd ../../../

	#停止ZoneServer
	cd World/Zone/ZoneServer/admin
	./stop.sh
	cd ../../../..

	#停止RegAuthGatewayServer
	cd Global/RegAuthGatewayServer/admin
	./stop.sh
	cd ../../..

	#停止RegAuthServer
	cd Global/RegAuthServer/admin
	./stop.sh
	cd ../../..

	#停止RegAuthDBServer
	cd Global/RegAuthDBServer/admin
	./stop.sh
	cd ../../..

	echo "Success to stop all servers!!!"
}

#主循环，根据输入的命令行参数来决定执行的脚本操作

while getopts ":I:DKSR" options
do
	case $options in
		I)
			if [ $# -ne 8 ]
			then
				Usage

				exit 1
			fi

			Init_Server $2 $3 $4 $5 $6 $7 $8

			exit 1
			;;

		D)
			Deploy_Server
			exit 1
			;;

		K)
			Kill_Server
			exit 1
			;;

		S)
			Start_Server
			exit 1
			;;
		R)
			Reload_Server
			exit 1
			;;

		?)
			Usage
			exit 1
			;;

		*)
			Usage
			exit 1
			;;

		esac

done

Usage
