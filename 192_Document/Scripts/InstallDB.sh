#!/bin/sh

#创建MYSQL帐号并初始化权限
#mysql -uroot -pbymobile20170605 < GrantPrivileges.sql 
mysql -ubymobile -pbymobile20170605 < InstallRegAuthDB.sql 
mysql -ubymobile -pbymobile20170605 < InstallNameDB.sql 
mysql -ubymobile -pbymobile20170605 < InstallRoleDB.sql
mysql -ubymobile -pbymobile20170605 < InstallUniqUinDB.sql
mysql -ubymobile -pbymobile20170605 < InstallWorldOnlineDB.sql
mysql -ubymobile -pbymobile20170605 < InstallLogDB.sql
mysql -ubymobile -pbymobile20170605 < InstallCardNoDB.sql 

