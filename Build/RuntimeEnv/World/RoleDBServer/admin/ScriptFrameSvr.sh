#!/bin/sh

## server admin script
ProcessName="RoleDBServer.dbg"
BIN_DIR="../bin"
PID_FILE="../bin/App.pid"
SleepTime=5
SIG_STOP=5
SIG_DUMP=6

if [ $# -lt 1 ]
then
        echo "$0 start|stop|restart|check|init|dump"
        exit
fi

Get_ProcessID()
{
	if [ -f $PID_FILE ]
	then
		echo `read line<$PID_FILE;echo $line`
	else
		echo 0
	fi
}

Get_ProcessNum()
{
	SERVER_PID=`Get_ProcessID`
	if [ $SERVER_PID -ne 0 ]
	then
		echo `ps h u $SERVER_PID | wc -l`
	else
		echo 0
	fi
}

Kill_Server()
{	
	SERVER_PID=`Get_ProcessID`
	PROCESS_NUM=`Get_ProcessNum`
	if [ $SERVER_PID -ne 0 -a $PROCESS_NUM -ne 0 ]
	then
		echo $SERVER_PID | xargs kill -12
	fi
}

#先切换工作目录到bin
cd ${BIN_DIR}

if [ $1 == "start" ]
then
        echo "start $ProcessName ..."
		PROCESS_NUM=`Get_ProcessNum`
		if [ $PROCESS_NUM -ne 0 ]
        then
                echo "$ProcessName is up now ,please stop first."
                exit
        fi

        ulimit -c unlimited

        ./$ProcessName -w #WORLDID# &

        sleep $SleepTime

		PROCESS_NUM=`Get_ProcessNum`
        if [ $PROCESS_NUM -eq 0 ]
        then
                echo "start $ProcessName failed."
                exit
        fi
        echo "done."
elif [ $1 == "stop" ]
then
        echo "stop $ProcessName ..."
		PROCESSNUM=`Get_ProcessNum`
        if [ $PROCESSNUM -eq 0 ]
        then
                echo "$ProcessName is already down now."
                exit
        fi
		Kill_Server

        sleep $SleepTime

		
		PROCESS_NUM=`Get_ProcessNum`
        if [ $PROCESS_NUM -ne 0 ]
        then
                echo "stop $ProcessName failed."
                exit
        fi
        echo "done"
elif [ $1 == "restart" ]
then
	PROCESS_NUM=`Get_ProcessNum`
    if [ $PROCESS_NUM -ne 0 ]
    then
        echo "$ProcessName is up now, stop it now."
		
		Kill_Server

		sleep $SleepTime
    fi

	PROCESS_NUM=`Get_ProcessNum`
        if [ $PROCESS_NUM -ne 0 ]
        then
                echo "stop $ProcessName failed."
                exit
        fi

    echo "restart $ProcessName..."

    ./$ProcessName -w #WORLDID# &

    sleep $SleepTime
    echo "done"

elif [ $1 == "reload" ]
then
	PROCESS_NUM=`Get_ProcessNum`
    if [ $PROCESS_NUM -eq 0 ]
    then
		echo "$ProcessName is not up now, start it first!"
		exit
	fi
	
	echo "reload $ProcessName..."

	./$ProcessName reload &

	echo "done"

elif [ $1 == "init" ]
then
        echo "initializing... "
        echo 1805306368 > /proc/sys/kernel/shmmax
        echo "done"
else
        echo "$0 start|stop|check|init"
fi
