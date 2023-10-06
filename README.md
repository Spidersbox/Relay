# relay
Control one or more HID API USB relay cards 
<br>
<hr>
<br>
### Usage

relay [-i] card information<br>
relay [relay number] check the state of the relay.<br>
relay [relay number]  [on | off] to set the state of the relay.<br>
relay [relay number]  [pulse] to turn on for 10 seconds, then off.<br>

relay [all] returns the state of all relays.<br>
relay [all]  [on | off] to set the state of all the relays.<br>
relay [all]  [pulse] to turn on all relays for 10 seconds, then off.<br>


### Examples
In these examples, I have two relay cards plug into my Banana PI.<br>
Each card has two relays.<br>

* relay -i
<pre>
 Device Found
   Type: 16c0 05df
   Path: 0004:0002:00
   Number of Relays = 2
   Device ID: X162W
   relay 1 is OFF
   relay 2 is OFF
  <br>
 Device Found
   Type: 16c0 05df
   Path: 0002:0002:00
   Number of Relays = 2
   Device ID: 78J67
   relay 3 is OFF
   relay 4 is OFF
</pre>

* relay all
<pre>
 Relay 1 is OFF
 Relay 2 is OFF
 Relay 3 is OFF
 Relay 4 is OFF
</pre>

* relay 3 on
<pre>
 this does not print out any thing, but if you do 'relay all' again:
 Relay 1 is OFF
 Relay 2 is OFF
 Relay 3 is ON
 Relay 4 is OFF
</pre>

<br>
<hr>
<br>

### Installation
The installation procedure is usually perfomed directly on the target system.<br>
Therefore a C compiler and dependencies should already be installed.<br>
Otherwise a cross compilation environment needs to be setup on a PC<br>
(this is not described here).<br>

* Install dependencies (package names may vary depending on your distribution):
<pre>
    apt-get install build-essential libhidapi-dev libusb-1.0-0 libusb-1.0-0-dev
</pre>

* Build dependencies from source (optional):
On some not so recent Linux distributions (like Debian Weezy)<br>
the HIDAPI library is not yet packaged, so it needs to be build from the source code.<br>
Follow these instructions to do that:
<pre>
    apt-get install libudev-dev libusb-1.0-0-dev
    git clone git://github.com/signal11/hidapi.git
    ./bootstrap
    ./configure
    make
    sudo make install
</pre>

* Clone Relay git repository :  
<pre>
    git clone https://github.com/Spidersbox/Relay
</pre>

* Build :  
<pre>
    cd Relay
    ./build.sh
</pre>
* to 'install'
<pre>
    sudo cp relay /usr/bin/
</pre>
<br>
* to use without sudo:
<pre>
sudo cp 50relays.rules /etc/udev/rules.d/
sudo reboot
</pre>
<br>
<br> 
<hr>
<br>
### Credits
I would like to give thanks to these people for thier work on their respective projects:

* [Darryl Bond](https://github.com/darrylb123), project usbrelay
* [Ondrej Wisniewski](https://github.com/ondrej1024), project crelay

Without thier code, I would still be scratching my head  8]
<br>  
