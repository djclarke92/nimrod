#-----------------------------------------------------------------------
# Nimrod is released under the GNU General Public License v2 only 
# for non-profit organisations.
#
# Commercial and/or profit making organsations MUST obtain a commercial 
# license before using Nimrod.
#-----------------------------------------------------------------------

# create the database
create database if not exists nimrod;
use nimrod; 


# Table of hardware devices.
CREATE TABLE IF NOT EXISTS `devices` (
	`de_DeviceNo` int(10) unsigned NOT NULL auto_increment,		# unique devices table record number
	`de_ComPort` varchar(100) NOT NULL default '',				# /dev/ttyUSB0
	`de_Address` int(10) NOT NULL default '0',					# modbus address 0x01 to 0xfe
	`de_NumInputs` int(10) NOT NULL default '0',				# LE MAX_INPUTS
	`de_NumOutputs` int(10) NOT NULL default '0',				# LE MAX_OUTPUTS
	`de_Type` int(10) NOT NULL default '0',						# see enum E_DEVICE_TYPE
	`de_Name` varchar(100) NOT NULL default '',					# nice name for this device
	`de_Status` int(10) NOT NULL default '0',					# device status: alive, dead, buried
	`de_Hostname` varchar(20) NOT NULL default '',				# pi hostname or ip address
	PRIMARY KEY (`de_DeviceNo`),
	UNIQUE KEY `de_address_index` (`de_Address`)
) ;

# table of io channels on each device
CREATE TABLE IF NOT EXISTS `deviceinfo` (
	`di_DeviceInfoNo` int(10) unsigned NOT NULL auto_increment,		# unique record number
	`di_DeviceNo` int(10) unsigned NOT NULL default '0',		# link to devices table
	`di_IOChannel` int(10) unsigned NOT NULL default '0',		# 0,1,2,3...15: LE MAX_INPUTS/MAX_OUTPUTS
	`di_IOName` varchar(100) NOT NULL default '',				# human readable name for this input/output, 'bathroom light'
	`di_IOType` int(10) NOT NULL default '0',					# see enum E_IO_TYPE
	`di_OnPeriod` int(10) NOT NULL default '0',					# output is on for this many seconds
	`di_StartTime` int(10) NOT NULL default '0',				# start time of day in minutes 0-1439
	`di_Hysteresis` int(10) NOT NULL default '0',				# hysteresis value for temperature changes
	`di_Temperature` decimal(4,1) NOT NULL default 0.0,			# temperature trigger value
	`di_OutOnStartTime` int(10) NOT NULL default '0',			# output on start time (time_t)
	`di_OutOnPeriod` int(10) NOT NULL default '0',				# output on period in seconds
	`di_Weekdays` char(7) NOT NULL default 'YYYYYYY',			# days of the week, sunday to saturday
	`di_AnalogType` char(1) NOT NULL default ' ',				# type of analog sensor: Voltage or Amps
	`di_CalcFactor` decimal(7,3) NOT NULL default 0.0,			# analog conversion factor
	`di_Voltage` decimal(4,1) NOT NULL default 0.0,				# voltage trigger value
	`di_Offset` decimal(7,3) NOT NULL default 0.0,				# analog offset factor for current measurement
	`di_MonitorPos` char(15) NOT NULL default '   ',			# display this device on the monitor page, 5 sets of 3 chars: 1F1, [12][FLR][1-4]
	`di_MonitorHi` decimal(4,1) NOT NULL default 0.0,			# upper value for range monitoring
	`di_MonitorLo` decimal(4,1) NOT NULL default 0.0,			# lower value for range monitoring
	`di_ValueRangeHi` char(10) NOT NULL default '',				# y axis high value, or null
	`di_ValueRangeLo` char(10) NOT NULL default '',				# y axis low value, or null  
	PRIMARY KEY (`di_DeviceInfoNo`),
	KEY (`di_DeviceNo`,`di_IOChannel`),
	UNIQUE KEY `di_name_index` (`di_IOName`)
) ;

# table of device/channel links
create TABLE IF NOT EXISTS `iolinks` (
	`il_LinkNo` int(10) unsigned NOT NULL auto_increment,		# unique record number
	`il_InDeviceNo` int(10) NOT NULL default '0',				# input device no
	`il_InChannel` int(10) NOT NULL default '0',				# input device channel
	`il_OutDeviceNo` int(10) NOT NULL default '0',				# output device no
	`il_OutChannel` int(10) NOT NULL default '0',				# output channel address
	`il_EventType` int(10) NOT NULL default '0',				# event type: click, dbl click, long press
	`il_OnPeriod` int(10) NOT NULL default '0',					# output on period if timer triggered
	`il_LinkDeviceNo` int(10) NOT NULL default '0',
	`il_LinkChannel` int(10) NOT NULL default '0',
	`il_LinkTest` varchar(5) NOT NULL default ' ',				# LT, GT, LE, GE, EQ, NE
	`il_LinkValue` decimal(4,1) NOT NULL default '0.0', 
	PRIMARY KEY (`il_LinkNo`),
	KEY `il_link_index` (`il_InDeviceNo`,`il_InChannel`)
) ;

# table of events
# ev_DeviceNo=-1: trigger file
# ev_DeviceNo=-2: database structure version
# ev_DeviceNo=-10xx: device channel click event
create TABLE IF NOT EXISTS `events` (
	`ev_EventNo` int(10) unsigned NOT NULL auto_increment,		# unique record number
	`ev_Timestamp` timestamp NOT NULL default '0000-00-00',		# event timestamp
	`ev_DeviceNo` int(10) NOT NULL default '0',					#
	`ev_IOChannel` int(10) NOT NULL default '0',				#
	`ev_EventType` int(10) NOT NULL default '0',				#
	`ev_Value` int(10) NOT NULL default '0',					#
	`ev_Description` varchar(250) NOT NULL default '',			#
	PRIMARY KEY (`ev_EventNo`),
	KEY `ev_device_index` (`ev_DeviceNo`,`ev_Timestamp`)
) ;

# table of users
create table IF NOT EXISTS `users` (
	`us_Username` varchar(50) NOT NULL default '',				# user name / email address
	`us_Name` varchar(100) NOT NULL default '',					# name
	`us_Password` text(256) NOT NULL default '',				# SHA-256 hash
	`us_AuthLevel` INT(10) NOT NULL default 0,					# security auth level
	`us_Features` char(10) NOT NULL default 'NNNNNNNNNN',		# optional features for each user
	PRIMARY KEY (`us_Username`),
	KEY `us_name_index` (`us_Name`)
);

# add default table records:
# 
insert into users (us_Username,us_Name,us_Password,us_AuthLevel) values ('nimrod@nimrod.co.nz','Nimrod Admin User','258497f62679c89a7ac952b27c2d2c6040cf8da412b8dd044d11156db0986b55',9);

# nimrod and nimrod_user are changed to 
GRANT insert,select,update,delete,create,drop,alter,lock tables on nimrod.* to nimrod_user@'localhost' identified by 'passw0rd.23';
GRANT insert,select,update,delete,create,drop,alter,lock tables on nimrod.* to nimrod_user@'%' identified by 'passw0rd.23';
