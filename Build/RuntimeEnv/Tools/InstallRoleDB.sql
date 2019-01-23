drop database if exists 1_RoleDB;
create database 1_RoleDB;
use 1_RoleDB;

create table IF NOT EXISTS t_userdata
(
	uin int unsigned not null,
	seq int unsigned not null,
	base_info blob,
	quest_info blob,
	item_info blob,
	friend_info blob,
	mail_info blob,
	reserved1 blob,
	reserved2 blob,
	primary key(uin, seq)
)engine = innodb default charset=utf8;

drop database if exists 2_RoleDB;
create database 2_RoleDB;
use 2_RoleDB;

create table IF NOT EXISTS t_userdata
(
	uin int unsigned not null,
	seq int unsigned not null,
	base_info blob,
	quest_info blob,
	item_info blob,
	friend_info blob,
	mail_info blob,
	reserved1 blob,
	reserved2 blob,
	primary key(uin, seq)
)engine = innodb default charset=utf8;

drop database if exists 3_RoleDB;
create database 3_RoleDB;
use 3_RoleDB;

create table IF NOT EXISTS t_userdata
(
	uin int unsigned not null,
	seq int unsigned not null,
	base_info blob,
	quest_info blob,
	item_info blob,
	friend_info blob,
	mail_info blob,
	reserved1 blob,
	reserved2 blob,
	primary key(uin, seq)
)engine = innodb default charset=utf8;

drop database if exists 4_RoleDB;
create database 4_RoleDB;
use 4_RoleDB;

create table IF NOT EXISTS t_userdata
(
	uin int unsigned not null,
	seq int unsigned not null,
	base_info blob,
	quest_info blob,
	item_info blob,
	friend_info blob,
	mail_info blob,
	reserved1 blob,
	reserved2 blob,
	primary key(uin, seq)
)engine = innodb default charset=utf8;

drop database if exists 5_RoleDB;
create database 5_RoleDB;
use 5_RoleDB;

create table IF NOT EXISTS t_userdata
(
	uin int unsigned not null,
	seq int unsigned not null,
	base_info blob,
	quest_info blob,
	item_info blob,
	friend_info blob,
	mail_info blob,
	reserved1 blob,
	reserved2 blob,
	primary key(uin, seq)
)engine = innodb default charset=utf8;

drop database if exists 6_RoleDB;
create database 6_RoleDB;
use 6_RoleDB;

create table IF NOT EXISTS t_userdata
(
	uin int unsigned not null,
	seq int unsigned not null,
	base_info blob,
	quest_info blob,
	item_info blob,
	friend_info blob,
	mail_info blob,
	reserved1 blob,
	reserved2 blob,
	primary key(uin, seq)
)engine = innodb default charset=utf8;

drop database if exists 7_RoleDB;
create database 7_RoleDB;
use 7_RoleDB;

create table IF NOT EXISTS t_userdata
(
	uin int unsigned not null,
	seq int unsigned not null,
	base_info blob,
	quest_info blob,
	item_info blob,
	friend_info blob,
	mail_info blob,
	reserved1 blob,
	reserved2 blob,
	primary key(uin, seq)
)engine = innodb default charset=utf8;

drop database if exists 8_RoleDB;
create database 8_RoleDB;
use 8_RoleDB;

create table IF NOT EXISTS t_userdata
(
	uin int unsigned not null,
	seq int unsigned not null,
	base_info blob,
	quest_info blob,
	item_info blob,
	friend_info blob,
	mail_info blob,
	reserved1 blob,
	reserved2 blob,
	primary key(uin, seq)
)engine = innodb default charset=utf8;

