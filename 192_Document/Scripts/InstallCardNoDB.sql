drop database if exists 1_CardNoDB;
create database 1_CardNoDB;
use 1_CardNoDB;

create table IF NOT EXISTS t_cardno_info
(
	cardno varchar(64) not null,
	cardpwd varchar(64) not null,
	cardid tinyint not null,
	cardtype tinyint not null,
	uin int unsigned,
	usetime datetime,
	primary key(cardno)
)engine = innodb default charset=utf8;

