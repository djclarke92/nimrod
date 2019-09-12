# Nimrod Home Automation

A home automation project based on the Raspberry Pi, Modbus IO devices and NodeMCU modules.  The control software is written in C++ and the web configuration is in PHP7.  Relies on 
a MySQL database running on a separate host.  The current software is compiled for Pi model B running the Rasbian buster release.

Why call it Nimrod ?  It needed a name and "Nimrod" was the name of a neanderthal butler in an early Doctor Who episode.

The Nimrod software is compiled on your standard linux desktop PC, then cross compiled for the Pi version your are using.  In theory you could compile everything on the Pi and 
do away with the cross compiler step.

Uses PHPGraphLib https://github.com/elliottb/phpgraphlib for displaying graphs.

## Supported Modbus devices

Wellpro modbus devices

WP3066ADAM		8DS18B20 - 8x DS18B20 temperature inputs
WP3082ADAM		8AI - 8x 0-20mA / 4-20mA analog inputs
WP3084ADAM		8VI - 8x 0-10V DC analog inputs
WP8025ADAM		8RO - 8x relay outputs
WP8026ADAM		16DI - 16x digital inputs
WP8027ADAM		16DO - 16x digital outputs
WP8028ADAM		8DI/8DO - 8x digital outputs, 8x digital inputs


# Nimrod package file structure
./LICENSE						GNU v3 license
./README.md						This file
./RELEASE-NOTES.TXT				A history of changes to Nimrod
./index.php						Nimrod web portal home page
./favicon.ico					Favicon file
./bin/nimrod					Nimrod server executable (amd64)
./bin/nimrod-arm7				Nimrod server executable (arm7)
./bin/set-address				Set the Modbus device index (amd64)
./bin/set-address-arm7			Set the Modbus device index (arm7)
./files/cameras.php				Shows CCTV camera snapshots on the Nimrod web console
./files/class.email.php
./files/common.php				Base configuration and some common functions
./files/commonhtml.php			Common functions which include HTML output.
./files/deviceinfo.php
./files/devices.php
./files/home.php
./files/intro.php
./files/iolinks.php
./files/phpgraphlib.php			PHP graphing lib
./files/site_config.php
./files/site_config.php.sample	Example site config file
./files/styles.css
./images/*						Various image files to make Nimrod look good
./scripts/tables.sql			MySQL table creation sql script
./scripts/build-num.sh			update the build number automatically each time makerelease.sh is run
./scripts/makerelease.sh		create a new nimrod package
./scripts/nimrod.service		Exampe Nimrod service file of the Pi


# Raspberry Pi Model 2 B+

## Create Pi bootable SD card

	1.	umount /dev/sdd1
	2.	dd bs=4M if=2019-07-10-raspbian-buster.img of=/dev/sdd
	3.	boot the pi
	
## Initial Setup of the Pi

	- Menu - Pi Configuration
		- System Tab
			- Expand Filesystem
			- Change password for pi user to something new (default is raspberry, nimrod default is passw0rd.23)
			- set hostname to nimrod (or nimrod2, nimrod3, etc)
			- Disable auto login 
		- Interfaces Tab
			- Enable SSH
		- Localization
			- Set Locale to EN / NZ
			- Set Timezone to Pacific/Auckland
			- Set Keyboard to United States / English International
			- Set WiFi Country to NZ
	- Reboot
	- Set password for root > sudu su; passwd
	- Set static IP address
		- sudo vi /etc/dhcpcd.conf (add to the end of the file)
			interface eth0
 		  		static ip_address=192.168.1.199/24
		  		static routers=<your_gateway_ip_address>	
		  		static domain_name_servers=<dns_ip_addres_1> <dns_ip_address_2>
		  			(NZ InspireNet DNS servers are 203.114.168.2 203.114.128.2)
	- Reboot	
	- apt-get update
	- apt-get upgrade
	- Reboot

## Install additional packages required by Nimrod on the Pi

	- cmd line: sudo apt-get update (update list of available packages)
	- gui: sudo aptitude
	- example to install curl
		- apt-cache search curl
		- apt-get install curl

	Raspbian buster
	- apache2 libapache2-mod-php php php-mysql php-gd
	- libmodbus5 libmodbus-dev
	- libssl-dev libusb-dev ntpddate
	- mariadb-client libmariadb-dev
	- (old mysql-client libmysqlclient-dev)
	- ssmtp mailutils mpack 
	
	- Enable mod-ssl in apache using a self signed certificate
	> sudo a2enmod ssl
	> sudo a2ensite default-ssl
	> sudo systemctl restart apache2

## Create the nimrod user on the Pi

	1.	> adduser nimrod
		- Set password to something
	2.	Add nimrod user to the dialout group to give acces to the /dev/ttyUSB* devices
		> adduser nimrod dialout
	3.	Add any public ssh keys to $HOME/.ssh/authorized_keys
	4.	Create pub/priv key pair
		> ssh-keygen -t rsa
		- copy the public key to nimrod@<database_server>:~/.ssh/authorized_keys

## Setup ssmtp mail on the Pi

	- sudo vi /etc/ssmtp/ssmtp.conf
	rewriteDomain=<your_domain_name>
	mailhub=<your_mail_server>:<port>
	hostname=nimrod@<your_domain_name>
	AuthUser=nimrod@<your_domain_name>
	AuthPass=PASSWORD
	useSTARTTLS=YES
	
	You also need to create the 'nimrod' email account on your mail server.

	Note: On Jessie 4.4.38-v7+ logging in with ssh will send a "Security" email. Something to do with having
	set a password for the nimrod user. See Google.


# Setup the cross compiler environment on your linux desktop PC

	On your linux desktop PC:
	- cd $HOME
	- mkdir raspberrypi; 
	- cd raspberrypi
	- rsync -rl --delete-after --copy-unsafe-links --progress pi@nimrod:/{lib,usr} $HOME/raspberrypi/rootfs

	- add to .bashrc
	export PATH=$PATH:$HOME/raspberrypi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin
	
	Add gcc/g++ compiler packages to your desktop including 32bit gcc support packages, lib32gcc-6-dev, lib32z1
	
	
# MySQL Database server

	The MySQL database will contain the nimrod configuration and record events and monitoring data.
	1.	Create nimrod user with password
	2.	Create pub/priv key
		ssh-keygen -t rsa
	3.	Copy pub key to nimrod@pi
	4.	Create nimrod database
	5.	Create the nimrod tables
	
## Database username and password

	If you want to change the default database username or password you need to edit the tables.sql and files/site_config.php files
	to specify your required username and password before you create the tables. 
	File:	./files/site_config.php			defines DB_USER_NAME and DB_PASSWORD
	File:	./scripts/tables.sql			grant select,insert,update,delete on .......
	
## Database ssh tunnel

	Nimrod sets up a tunnel to a remote mysql server as per the IP address in the site_config.php file. On the remote mysql host you need to add nimrod as a user and add 
	the ssh keys for all nimrod hosts

	

# Installing Nimrod on the Pi into /var/www/html for the first time

	1.	Copy the install tar file from your PC to the Pi
		> scp nimrod-x.y.z.tgz pi@nimrod:/var/www/html/.
		
	2.	Set permissions and untar the package as the nimrod user
		# ssh nimrod@nimrod 
		> sudo chown -R nimrod:nimrod /var/www/html
		> cd /var/www/html/files
		> tar xvzf nimrod.x.y.z.tgz
		> cp files/site_config.php.sample files/site_config.php
		> vi site_config.php
			- set REMOTE_MYSQL_HOST and DB_PASSWORD
	
	3.	Establish the SSH tennel to the database host to accept the host key as the nimrod user
		> ssh nimrod@REMOTE_DB_HOST
		> yes
		# ctrl-d		
			
	4.	Set nimrod to start automatically as the pi user
		# ssh pi@nimrod
		> cd /var/www/html
		> sudo cp scripts/nimrod.service /lib/systemd/system/.
		> sudo chown root:root /lib/systemd/system/nimrod.service
		> sudo systemctl enable nimrod.service
		> sudo systemctl start nimrod

	Nimrod should now be running, see the log in /home/nimrod/nimrod.log.  Point your browser to https://nimrod/index.php
	
	
# Setting up a new Pi

	1. 	Use dd to write the nimrod image file onto a new micro flash card
	2.	Boot the Pi using the new flash card
	3.	Set the correct hostname
		> sudo vi /etc/hostname
		> sudo vi /etc/hosts
	4.	Set the correct IP address
		> sudo vi /etc/dhcpcd.conf
	5.	Reboot
		> sudo reboot
	

