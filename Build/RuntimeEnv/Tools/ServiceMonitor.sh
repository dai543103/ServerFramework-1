#!/bin/sh

#服务器进程监控脚本

#使用方法
Usage()
{
	echo "Usage"
}

#Zone Monitor
Zone_Monitor()
{
	if [ $# -ne 1 ]
	then
		Usage
		exit 1
	fi

	ZonePID=`pgrep -fl ZoneServer | grep -w "\-z $1"`

	if [ -z "$ZonePID" ]
	then
		#进程不存在
		cd ~/RuntimeEnv/World/Zone/ZoneServer/admin/
		./start.sh
	fi
}

#World Monitor

while getopts ":Z:WRER" options
do
	case $options in
		Z)
			if [ $# -ne 2 ]
			then
				Usage		
				exit 1
			fi
			
			Zone_Monitor 1

			exit 1
			;;
		
		*)
			Usage
			exit 1
			;;

	esac
done

Usage
