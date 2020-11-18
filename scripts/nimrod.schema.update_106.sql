use database nimrod;


alter table deviceinfo add column `di_MonitorPos` char(15) NOT NULL default '   '	after di_Offset;	# display this device on the monitor page, 5 sets of 3 chars: 1F1, [12][FLR][1-4]
alter table deviceinfo add column `di_MonitorHi` decimal(4,1) NOT NULL default 0.0 after di_MonitorPos;		# upper value for range monitoring
alter table deviceinfo add column `di_MonitorLo` decimal(4,1) NOT NULL default 0.0 after di_MonitorHi;		# lower value for range monitoring
alter table deviceinfo add column `di_ValueRangeHi` char(10) NOT NULL default ''	after di_MonitorLo;	# y axis high value, or null
alter table deviceinfo add column `di_ValueRangeLo` char(10) NOT NULL default '' after di_ValueRangeHi;		# y axis low value, or null  

# table of users
create table IF NOT EXISTS `users` (
        `us_Username` varchar(50) NOT NULL default '',                  # user name / email address 
        `us_Name` varchar(100) NOT NULL default '',                     # name
        `us_Password` text(256) NOT NULL default '',                    # SHA-256 hash
        `us_AuthLevel` INT(10) NOT NULL default 0,                      # security auth level
        `us_Features` char(10) NOT NULL default 'NNNNNNNNNN',           # optional features for each user
        PRIMARY KEY (`us_Username`),
        KEY `us_name_index` (`us_Name`)
);


# add default table records:
#
insert into users (us_Username,us_Name,us_Password,us_AuthLevel) values ('nimrod@nimrod.co.nz','Nimrod Admin User','258497f62679c89a7ac952b27c2d2c6040cf8da412b8dd044d11156db0986b55',9);

