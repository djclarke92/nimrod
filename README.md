# Nimrod Home Automation

A home automation project based on the Raspberry Pi, Modbus IO devices and NodeMCU modules.  The control software is written in C++ and the web configuration is in PHP7.  Relies on 
a MySQL database running on a separate host.  The current software is compiled for Pi model B running the Rasbian buster release.

Why call it Nimrod ?  It needed a name and "Nimrod" was the name of a neanderthal butler in an early Doctor Who episode. The original purpose was to develop something to turn on my
motorbike glove heater 30 minutes before my morning alarm went off if the outside temperature was below 15 deg C.

The Nimrod software is compiled on your standard linux desktop PC, then cross compiled for the Pi version your are using.  In theory you could compile everything on the Pi and 
do away with the cross compiler step.

Uses dygraph.js from http://dygraphs.com for displaying graphs.

What can you do with it ?
* Input events can be switches, temperature values, voltage levels or time of day
* Turn on one or more output ports when an input event occurs
* Input switch events can be a click, double click or long click, each can trigger different output actions
* Input events can be chained together, e.g. if time is 6:30am and temperature sensor #2 is < 15 deg then turn on output #7 for 30 minutes

Communication with each modbus device is polled.  All digital inputs are active low, i.e. connect to 0V / Gnd to activate.  All digital outputs are active low  open collector outputs.

The Nimrod web page also supports displaying CCTV camera images and video streams, to do this you need the mount the CCTV image storage filesystem onto the Pi (edit /etc/fstab and 
create /etc/.smbpwd).  The camera definitions are in the file/site_config.conf file.
1.	/etc/fstab example
	//<SERVER_IP_ADDRESS>/cctv  /cctv   cifs    auto,gid=users,file_mode=0660,dir_mode=0660,iocharset=utf8,credentials=/etc/.smbpwd  0 0
2.	/etc/.smbpwd example, mode must be 0600
	username=<some_user_name>
	password=<some_password>
	domain=YOURSMBWORKGROUP

Note that for each server using a guest account, you may wish to specify an additional share in /etc/samba/smb.conf
thus;

[cctv]
        comment = cctv
        inherit acls = Yes
        path = /cctv
        read only = No
        vfs objects =
[cctv2]
        comment = cctv2
        inherit acls = Yes
        path = /cctv
        read only = No
        vfs objects =

Add a share for each additional server that will connect using the same account. In /etc/fstab, specifiy the new share name for each additional server
//<SERVER_IP_ADDRESS/cctv2, etc

This works around some Windows/samba peculiarities and allows conflict-free file/directory access.

Alternatively, you could use NFS instead, if you aren't using Windows PC's to access the camera files.

## Devices, Device Info and IO Links

Each Modbus interface device is called a "Device" in Nimrod speak.  So a "Device" could be a 16 digital input unit, 8 relay output unit, 8 analog voltage input unit, etc.

Once you have define a device you need to tell Nimrod what each of it's ports is connected to, this is called the "Device Info".  So for a 16DI unit you would setup input channel 1 is 
"Kitchen switch 1", input channel 2 is "Lounge switch 1", etc. The actual connections are determined by the physical cabling between the switch and the Modbus device.  Output devices 
are similarly defined, e.g. 8RO device channel #4 is "Carport Lights".

Now that you have defined the inputs and outputs you need to tell Nimrod how to connect them together.  This is called "IO Links".  E.g. "Kitchen switch 1" links to 
"Carport Lights", and you can also specify if the output is on until the switch is pressed again or automatically turns off after a set time.

## Supported Modbus devices

Wellpro modbus devices

![WP8028ADAM device](/images/WP8028ADAM.jpg)

* WP3066ADAM		8DS18B20 - 8x DS18B20 temperature inputs
* WP3082ADAM		8AI - 8x 0-20mA / 4-20mA analog inputs
* WP3084ADAM		8VI - 8x 0-10V DC analog inputs
* WP8025ADAM		8RO - 8x relay outputs
* WP8026ADAM		16DI - 16x digital inputs
* WP8027ADAM		16DO - 16x digital outputs
* WP8028ADAM		8DI/8DO - 8x digital outputs, 8x digital inputs
* PD3064			8KT - 8 K Thermocouples

## Supported WiFi devices

* NodeMCU (ESP8266)
	modules - adc crypto enduser_setup file gpio http net node pwm tmr uart websocket wifi + TLS


# Nimrod package file structure
* ./LICENSE							GNU v3 license
* ./README.md						This file
* ./RELEASE-NOTES.TXT				A history of changes to Nimrod
* ./index.php						Nimrod web portal home page
* ./favicon.ico						Favicon file
* ./bin/nimrod						Nimrod server executable (amd64)
* ./bin/nimrod-arm7					Nimrod server executable (arm7)
* ./bin/set-address					Set the Modbus device index (amd64)
* ./bin/set-address-arm7			Set the Modbus device index (arm7)
* ./files/cameras.php				Shows CCTV camera snapshots on the Nimrod web console
* ./files/class.email.php
* ./files/common.php				Base configuration and some common functions
* ./files/commonhtml.php			Common functions which include HTML output.
* ./files/deviceinfo.php
* ./files/devices.php
* ./files/dygraph.js				Javascript graphing lib
* ./files/dygraph.css				Javascript graphing lib
* ./files/dygraph.min.js			Javascript graphing lib
* ./files/events.php				ToDo
* ./files/home.php
* ./files/intro.php					What is Nimrod
* ./files/monitor.php				Full screen monitor pages 1 and 2
* ./files/iolinks.php
* ./files/site_config.php
* ./files/site_config.php.sample	Example site config file
* ./files/styles.css
* ./images/*						Various image files to make Nimrod look good
* ./scripts/tables.sql				MySQL table creation sql script
* ./scripts/build-num.sh			Update the build number automatically each time makerelease.sh is run
* ./scripts/makerelease.sh			Create a new nimrod package
* ./scripts/nimrod.service			Exampe Nimrod service file of the Pi
* ./scripts/archive_cctv.sh			Script to delete old cctv files
* ./sounds/alerts.mp3
* ./sounds/warning1.mp3


# Raspberry Pi Model 2 B+

## Create Pi bootable SD card

We recommend 16GB SD cards are used, an 8GB card is likely to run out of space in the future.

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

- Remove the default index.html file
> sudo rm /var/www/html/index.html

## Create the nimrod user on the Pi

It is recommended that you use the same uid/gid for each user/group across your installation. The useradd/groupadd 
commands have options for this.

1.	> adduser nimrod
	- Set password to something
2.	Add nimrod user to the dialout group to give acces to the /dev/ttyUSB* devices
	> adduser nimrod dialout
3.	Add any public ssh keys to $HOME/.ssh/authorized_keys
4.	Create pub/priv key pair
	> ssh-keygen -t rsa
	- copy the public key to nimrod@<database_server>:~/.ssh/authorized_keys

## Setup ssmtp mail on the Pi

> sudo vi /etc/ssmtp/ssmtp.conf
	rewriteDomain=<your_domain_name>
	mailhub=<your_mail_server>:<port>
	hostname=nimrod@<your_domain_name>
	AuthUser=nimrod@<your_domain_name>
	AuthPass=PASSWORD
	useSTARTTLS=YES
	
You also need to create the 'nimrod' email account on your mail server.

Note: On Jessie 4.4.38-v7+ logging in with ssh will send a "Security" email. Something to do with having
set a password for the nimrod user. See Google.

## Create a certificate so NodeMCU (ESP8266) WiFi devices can connect

> ssh nimrod@nimrod
> openssl req -x509 -nodes -days 3650 -newkey rsa:2048 -keyout nimrod-cert.pem -out nimrod-cert.pem


# Setup the cross compiler environment on your linux desktop PC

On your linux desktop PC:
> cd $HOME
> mkdir raspberrypi; 
> cd raspberrypi
> rsync -rl --delete-after --copy-unsafe-links --progress pi@nimrod:/{lib,usr} $HOME/raspberrypi/rootfs

# Download the raspberrypi tools from github

cd $HOME/raspberrypi

> git clone https://github.com/raspberrypi/tools

# Download the raspberrypi documentation from github 

> git clone https://github.com/raspberrypi/documentation

- add to .bashrc
export PATH=$PATH:$HOME/raspberrypi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin
	
Add gcc/g++ compiler packages to your desktop including 32bit gcc support packages, lib32gcc-6-dev, lib32z1
	
	
# MySQL Database server

The MySQL database will contain the nimrod configuration and record events and monitoring data.
1.	Create nimrod user with password
2.	Create pub/priv key
	> ssh-keygen -t rsa
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

## Setup basic auth for apache on the nimrod host

> ssh pi@nimrod
> sudo htpasswd -c /etc/apache2/.htpasswd nimrod@nimrod.co.nz
	Replace nimrod@nimrod.co.nz with your own email
> sudo htpasswd /etc/apache2/.htpasswd another_user

> sudo vi /etc/apache2/apache2.conf
	<Directory "/var/www/html">
        Options +FollowSymLinks +Multiviews +Indexes
        AllowOverride None
        AuthType basic
        AuthName "Nimrod - restricted site"
        AuthUserFile /etc/apache2/.htpasswd
        Require valid-user
	</Directory>


# Installing Nimrod on the Pi into /var/www/html for the first time

1.	Copy the install tar file from your PC to the Pi
> scp nimrod-x.y.z.tgz pi@nimrod:/var/www/html/.
		
2.	Set permissions and untar the package as the nimrod user
> ssh nimrod@nimrod 
> sudo chown -R nimrod:nimrod /var/www/html
> cd /var/www/html
> tar xvzf nimrod.x.y.z.tgz
> cp files/site_config.php.sample files/site_config.php
> vi site_config.php
	- set REMOTE_MYSQL_HOST and DB_PASSWORD
	
3.	Establish the SSH tennel to the database host to accept the host key as the nimrod user
> ssh nimrod@REMOTE_DB_HOST
> yes
> ctrl-d		
			
4.	Set nimrod to start automatically as the pi user
> ssh pi@nimrod
> cd /var/www/html
> sudo cp scripts/nimrod.service /lib/systemd/system/.
> sudo chown root:root /lib/systemd/system/nimrod.service
> sudo systemctl enable nimrod.service
> sudo systemctl start nimrod

5.	Nimrod should now be running

See the log in /home/nimrod/nimrod.log.  Point your browser to https://nimrod/index.php
	
	
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
	
# Development and debugging

1.	A fairly verbose log file is written to /home/nimrod/nimrod.log
2.	Check the apache2 logs at /var/log/apache/error.log
3.	The nimrod application will automatically upgrade itself if it sees a nimrod.x.y.x.tgz package file in the /var/www/html directory
	To turn off the automatic upgrade feature, touch the file /var/www/html/no.upgrade
4.	The nimrod application will attempt to create an SSH tunnel to the remote MySQL database host on startup.
	To disable the SSH tunnel, touch the file /var/www/html/no.tunnel
5.	Each modbus device must be set with a unique address (1-254) using the set-address (or set-addres-arm7) tool. 
	A maximum of 254 devices are currently supported across any number of Pi's.  
	The baud rate for each modbus device should be set to 19200, the default is 9600, using the set-address tool.
6.	To build the software run the following command
	> ./scripts/makerelease.sh x.y.z ["nimrod_host1 [nimrod_host2]"] [copy]
	where "x.y.z" is the version number you are building,
	 	"nimrod_host1 nimrod_host2" is an optional space separated list of hosts to scp the successful package file onto,
	 	"copy" will just scp the existing package file, skipping the compile step

