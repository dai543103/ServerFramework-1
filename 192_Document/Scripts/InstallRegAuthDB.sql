drop database if exists 1_AccountDB;
create database 1_AccountDB;
use 1_AccountDB;

create table IF NOT EXISTS t_accountdata
(
	account varchar(64) not null,
	platformtype tinyint not null,
	uin int unsigned not null,
	worldid int unsigned not null,
	createtime datetime default null,
	password varchar(128),
	thirdparty varchar(64),
	deviceid varchar(128),
	channelid varchar(128),
	clienttype varchar(32),
	appid varchar(32),

	primary key(account,platformtype)
)engine = innodb default charset=utf8;

drop database if exists 2_AccountDB;
create database 2_AccountDB;
use 2_AccountDB;

create table IF NOT EXISTS t_accountdata
(
	account varchar(64) not null,
	platformtype tinyint not null,
	uin int unsigned not null,
	worldid int unsigned not null,
	createtime datetime default null,
	password varchar(128),
	thirdparty varchar(64),
	deviceid varchar(128),
	channelid varchar(128),
	clienttype varchar(32),
	appid varchar(32),

	primary key(account,platformtype)
)engine = innodb default charset=utf8;

drop database if exists 3_AccountDB;
create database 3_AccountDB;
use 3_AccountDB;

create table IF NOT EXISTS t_accountdata
(
	account varchar(64) not null,
	platformtype tinyint not null,
	uin int unsigned not null,
	worldid int unsigned not null,
	createtime datetime default null,
	password varchar(128),
	thirdparty varchar(64),
	deviceid varchar(128),
	channelid varchar(128),
	clienttype varchar(32),
	appid varchar(32),

	primary key(account,platformtype)
)engine = innodb default charset=utf8;

drop database if exists 4_AccountDB;
create database 4_AccountDB;
use 4_AccountDB;

create table IF NOT EXISTS t_accountdata
(
	account varchar(64) not null,
	platformtype tinyint not null,
	uin int unsigned not null,
	worldid int unsigned not null,
	createtime datetime default null,
	password varchar(128),
	thirdparty varchar(64),
	deviceid varchar(128),
	channelid varchar(128),
	clienttype varchar(32),
	appid varchar(32),

	primary key(account,platformtype)
)engine = innodb default charset=utf8;

drop database if exists 5_AccountDB;
create database 5_AccountDB;
use 5_AccountDB;

create table IF NOT EXISTS t_accountdata
(
	account varchar(64) not null,
	platformtype tinyint not null,
	uin int unsigned not null,
	worldid int unsigned not null,
	createtime datetime default null,
	password varchar(128),
	thirdparty varchar(64),
	deviceid varchar(128),
	channelid varchar(128),
	clienttype varchar(32),
	appid varchar(32),

	primary key(account,platformtype)
)engine = innodb default charset=utf8;

drop database if exists 6_AccountDB;
create database 6_AccountDB;
use 6_AccountDB;

create table IF NOT EXISTS t_accountdata
(
	account varchar(64) not null,
	platformtype tinyint not null,
	uin int unsigned not null,
	worldid int unsigned not null,
	createtime datetime default null,
	password varchar(128),
	thirdparty varchar(64),
	deviceid varchar(128),
	channelid varchar(128),
	clienttype varchar(32),
	appid varchar(32),

	primary key(account,platformtype)
)engine = innodb default charset=utf8;

drop database if exists 7_AccountDB;
create database 7_AccountDB;
use 7_AccountDB;

create table IF NOT EXISTS t_accountdata
(
	account varchar(64) not null,
	platformtype tinyint not null,
	uin int unsigned not null,
	worldid int unsigned not null,
	createtime datetime default null,
	password varchar(128),
	thirdparty varchar(64),
	deviceid varchar(128),
	channelid varchar(128),
	clienttype varchar(32),
	appid varchar(32),

	primary key(account,platformtype)
)engine = innodb default charset=utf8;

drop database if exists 8_AccountDB;
create database 8_AccountDB;
use 8_AccountDB;

create table IF NOT EXISTS t_accountdata
(
	account varchar(64) not null,
	platformtype tinyint not null,
	uin int unsigned not null,
	worldid int unsigned not null,
	createtime datetime default null,
	password varchar(128),
	thirdparty varchar(64),
	deviceid varchar(128),
	channelid varchar(128),
	clienttype varchar(32),
	appid varchar(32),

	primary key(account,platformtype)
)engine = innodb default charset=utf8;
