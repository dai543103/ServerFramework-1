#!/bin/sh

## server admin script
ProcessName="ZoneGatewayServer.dbg"
BIN_DIR="../bin"
PID_FILE="../bin/App.pid"
SleepTime=5
SIG_STOP=5
SIG_DUMP=6
USER=`whoami`

if [ $# -lt 1 ]
then
        echo "$0 start|stop|restart|check|init|dump"
        exit
fi

Kill_Server()
{	
    PROCESSID=`pgrep -u $USER -f ${ProcessName}`
	if [ ! -z $PROCESSID ]
	then
		echo $PROCESSID | xargs kill -12
	fi
}

#先切换工作目录到bin
cd ${BIN_DIR}

if [ $1 == "start" ]
then
        echo "start $ProcessName ..."
        PROCESSID=`pgrep -u $USER -f ${ProcessName}`
        if [ ! -z $PROCESSID ]
        then
                echo "$ProcessName is up now ,please stop first."
                exit
        fi

        ulimit -c unlimited

        ./$ProcessName &

        sleep $SleepTime

        PROCESSID=`pgrep -u $USER -f ${ProcessName}`
        if [ -z $PROCESSID ]
        then
                echo "start $ProcessName failed."
                exit
        fi
        echo "done."
elif [ $1 == "stop" ]
then
        echo "stop $ProcessName ..."
        PROCESSID=`pgrep -u $USER -f ${ProcessName}`
        if [ -z $PROCESSID ]
        then
                echo "$ProcessName is already down now."
                exit
        fi
		Kill_Server

        sleep $SleepTime


        PROCESSID=`pgrep -u $USER -f ${ProcessName}`
        if [ ! -z $PROCESSID ]
        then
                echo "stop $ProcessName failed."
                exit
        fi
        echo "done"
elif [ $1 == "restart" ]
then
        PROCESSID=`pgrep -u $USER -f ${ProcessName}`
    if [ ! -z $PROCESSID ]
    then
        echo "$ProcessName is up now, stop it now."
		
		Kill_Server

		sleep $SleepTime
    fi

        PROCESSID=`pgrep -u $USER -f ${ProcessName}`
        if [ ! -z $PROCESSID ]
        then
                echo "stop $ProcessName failed."
                exit
        fi

    echo "restart $ProcessName..."

    ./$ProcessName &

    sleep $SleepTime
    echo "done"

fi

