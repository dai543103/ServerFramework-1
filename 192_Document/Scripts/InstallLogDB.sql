drop database if exists 1_MofangDB;
create database 1_MofangDB;
use 1_MofangDB;

create table IF NOT EXISTS t_mofang_login
(
	areaid int unsigned not null,
	gameid int unsigned not null,
	numid  int unsigned not null,
	username varchar(64) not null,
	deviceid varchar(128) not null,
	channelid varchar(128) not null,
	newplayer tinyint not null,
	ip varchar(64) not null,
	clienttype tinyint not null,
	logtime datetime not null
)engine = innodb default charset=utf8;

create table IF NOT EXISTS t_mofang_pay
(
	areaid int unsigned not null,
	gameid int unsigned not null,
	numid  int unsigned not null,
	username varchar(64) not null,
	deviceid varchar(128) not null,
	channelid varchar(128) not null,
	orderid varchar(128) not null,
	amount	int unsigned not null,
	virtualcurrency int unsigned not null,
	ip varchar(64) not null,
	clienttype tinyint not null,
	logtime datetime not null
)engine = innodb default charset=utf8;

create table if not exists t_tally_info
(
	numid int unsigned not null,
	deviceid varchar(128) not null,
	channelid varchar(128) not null,
	coin bigint not null,
	ticket bigint not null,
	cash bigint not null,
	bronze int unsigned not null,
	silver int unsigned not null,
	gold int unsigned not null,
	bindbronze int unsigned not null,
	bindsilver int unsigned not null,
	bindgold int unsigned not null,
	addcoin bigint not null,
	addticket bigint not null,
	addcash bigint not null,
	addwin bigint not null,
	totalwin bigint not null,
	logtime datetime not null
)engine = innodb default charset=utf8;
