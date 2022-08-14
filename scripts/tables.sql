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
	`de_BaudRate` int(10) NOT NULL default 19200,				# baud rate
	`de_AlwaysPoweredOn` char(1) NOT NULL default 'Y',			# expect the device to always be there or not
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
	PRIMARY KEY (`il_LinkNo`),
	KEY `il_link_index` (`il_InDeviceNo`,`il_InChannel`)
) ;

# table of events
# ev_DeviceNo=-1: trigger file
# ev_DeviceNo=-2: database structure version
# ev_DeviceNo=-3: user login attempt
# ev_DeviceNo=-4: plcstate table changed
# ev_DeviceNo=-5: plcstate screen button event
# ev_DeviceNo=-6: plcstate DelayTime change event
# ev_DeviceNO=-7: plcstate messages
# ev_DeviceNo=-10xx: device channel click event
create TABLE IF NOT EXISTS `events` (
	`ev_EventNo` int(10) unsigned NOT NULL auto_increment,		# unique record number
	`ev_Timestamp` timestamp NOT NULL default '0000-00-00',		# event timestamp
	`ev_DeviceNo` int(10) NOT NULL default '0',					#
	`ev_IOChannel` int(10) NOT NULL default '0',				#
	`ev_EventType` int(10) NOT NULL default '0',				#
	`ev_Value` decimal(10,3) NOT NULL default '0',				#
	`ev_Description` varchar(250) NOT NULL default '',			#
	PRIMARY KEY (`ev_EventNo`),
	KEY `ev_device_index` (`ev_DeviceNo`,`ev_Timestamp`)
) ;

# table of users
create TABLE IF NOT EXISTS `users` (
	`us_Username` varchar(50) NOT NULL default '',				# user name / email address
	`us_Name` varchar(100) NOT NULL default '',					# name
	`us_Password` text(256) NOT NULL default '',				# SHA-256 hash
	`us_AuthLevel` INT(10) NOT NULL default 0,					# security auth level
	`us_Features` char(10) NOT NULL default 'NNNNNNNNNN',		# optional features for each user
	`us_CardNumber` char(10) NOT NULL default '',				# card number
	`us_CardPin` char(6) NOT NULL default '',					# card PIN
	`us_CardEnabled` char(1) NOT NULL default 'N',				# card is enabled
	`us_PinFailCount` int(10) NOT NULL default 0,				# consecutive pin failure count
	PRIMARY KEY (`us_Username`),
	KEY `us_name_index` (`us_Name`),
	KEY `us_cardnumber_index` (`us_CardNumber`)
);

# table of link conditions
create TABLE IF NOT EXISTS `conditions` (
	`co_ConditionNo` int(10) unsigned NOT NULL auto_increment,	# unique record number
	`co_LinkNo` int(10) NOT NULL default '0',					# link to iolinks table
	`co_LinkDeviceNo` int(10) NOT NULL default '0',
	`co_LinkChannel` int(10) NOT NULL default '0',
	`co_LinkTest` varchar(5) NOT NULL default ' ',				# LT, GT, LE, GE, EQ, NE
	`co_LinkValue` decimal(6,1) NOT NULL default '0.0', 
	PRIMARY KEY (`co_ConditionNo`),
	KEY `co_linkno_index` (`co_LinkNo`)
) ;

# table of camera info
create table IF NOT EXISTS `cameras` (
	`ca_CameraNo` int(10) unsigned NOT NULL auto_increment,		# unique record number
	`ca_Name` varchar(50) NOT NULL default ' ',					# camera name
	`ca_IPAddress` varchar(15) NOT NULL default ' ',			# camera IPv4 address
	`ca_PTZ` char(1) NOT NULL default 'N',						# PTZ capable Y/N
	`ca_Encoding` varchar(10) NOT NULL default ' ',				# H.264 / H.265 encoding
	`ca_Directory` varchar(250) NOT NULL default ' ',			# file system directory from root (/)
	`ca_UserId` varchar(20) NOT NULL default ' ',				# camera user id
	`ca_Password` varchar(50) NOT NULL DEFAULT ' ',				# camera password (encrypted)
	`ca_Model` varchar(20) NOT NULL default ' ',				# camera model, e.g. FI9853EP
	`ca_MJpeg` char(1) NOT NULL default 'N',					# supports MJpeg streaming Y/N/H (H means supports mjpeg over https)
	PRIMARY KEY (`ca_CameraNo`),
	KEY `ca_name_index` (`ca_Name`)
) ;

# table of plc states
create table IF NOT EXISTS `plcstates` (
	`pl_StateNo` int(10) unsigned NOT NULL auto_increment,			# unique record number
	`pl_Operation` varchar(50) NOT NULL default '',					# operation name, e.g. Setup Mode or Run Mode
	`pl_StateName` varchar(50) NOT NULL default '',					# state name, e.g. WAITING or MOVING_LEFT
	`pl_StateIsActive` char(1) NOT NULL default 'N',				# remember which state is active
	`pl_StateTimestamp` timestamp NOT NULL default '0000-00-00',	# when did this state become active
	`pl_RuleType` char(1) NOT NULL default '',						# I=Init, E=Event
	`pl_DeviceNo` int(10) NOT NULL default 0,						# link to the deviceinfo table
	`pl_IOChannel` int(10) NOT NULL default 0,						# link to the deviceinfo table
	`pl_Value` decimal(8,2) NOT NULL default 0,						# data value
	`pl_Test` varchar(5) NOT NULL default '',						# LT, GT, LE, GE, EQ, NE
	`pl_NextStateName` varchar(50) NOT NULL default "",				# link to the next state
	`pl_Order` int(10) NOT NULL default 0,							# optional execution order
	`pl_DelayTime` decimal(8,2) NOT NULL default 0,					# time delay in seconds to ignore the event
	`pl_TimerValues` varchar(50) NOT NULL default '',				# comma separated list of timer values, e.g. 10s,30s,2m
	`pl_PrintOrder` int(10) NOT NULL default 0,						# print ordering
	`pl_DelayKey` varchar(20) NOT NULL default '',					# key for updating DelayTime for multiple records
	`pl_InitialState` char(1) NOT NULL default 'N',					# this is the starting state
	PRIMARY KEY (`pl_StateNo`),
	KEY `pl_statename_index` (`pl_StateName`)
) ;


# add default table records:
# 
insert into users (us_Username,us_Name,us_Password,us_AuthLevel) values ('nimrod@nimrod.co.nz','Nimrod Admin User','258497f62679c89a7ac952b27c2d2c6040cf8da412b8dd044d11156db0986b55',9);
insert into events (ev_DeviceNo,ev_Value) values(-2,108);

# nimrod and nimrod_user are changed to 
GRANT insert,select,update,delete,create,drop,alter,lock tables on nimrod.* to nimrod_user@'localhost' identified by 'passw0rd.23';
GRANT insert,select,update,delete,create,drop,alter,lock tables on nimrod.* to nimrod_user@'%' identified by 'passw0rd.23';
