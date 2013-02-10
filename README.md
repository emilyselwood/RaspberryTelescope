Raspberry Telescope
===================

This a simple project to allow remote control of a digital camara through a web browser.

Features
--------

* Live preview
* Remote Triggering
* Capture to server or return immedatly to browser
* Controlled through a web browser.

Roadmap
-------

* Browse captured images
* Adjust Camera settings (iso, shutter speed, etc)
* Time laps 
* Capture video


Building and Running
--------------------

To get started on a Raspbery PI you will need to run the following commands in squence. This assumes you are in the home directory of a fresh wheezy-raspbian distro. 

    sudo apt-get update
    sudo apt-get install git libgphoto2-2-dev libconfig-dev libreadline-dev libncurses-dev
    git clone git://github.com/wselwood/RaspberryTelescope.git
    cd RaspberryTelescope
    wget http://www.lua.org/ftp/lua-5.2.1.tar.gz -O lua-5.2.1.tar.gz
    tar xvzf lua-5.2.1.tar.gz
    cd lua-5.2.1
    make linux
    cd ..
    make
    ./raspberytelescope
    
Connect your camera and turn it on. In a web browser connect to http://raspberrypi:8080/ and you should see the live preview from your camera. 

On a pi the live preview can be pretty slow and not exactly live. I think a faster memory card will probably help, or a configuration system that allows for the preview images to be writen to a ram disk.

Make Targets:
* all : Builds every thing
* raspberrytelescope : Builds every thing at the moment.
* clean : clean

At the moment there two warnings one in the mongoose.c file and one in telescopecamera.c about a pointer being cast 
to an integer if you are building on a 64bit machine. This doesn't happen on a Pi. 
These are both due to using void * to store int data.

To run the web server simply execude the raspberrytelescope command. To exit press enter in the shell thats running the webserver.

Depencies
---------

We need lua, libgphoto2 and libconfig

libgphoto2 and libconfig can be got from you favourte package manager.

lua can be got from www.lua.org/ftp/ Ive used version 5.2.1 Download and compile. You may need to install libncurses and libreadline

At the top of the make file there is a path to where you downloaded and built lua. Defaults to "lua-5.2.1/"

This also uses Mongoose and JQuery-UI but the needed files are included here. 

* The origional Mongoose files can be found here : https://github.com/valenok/mongoose
* The origional JQuery-UI files can be found at http://jqueryui.com

The JQuery files are included in the build rather than pulled from the internet as this needs to work with out an internet connection, for instance where the Pi is working as a wi-fi hot spot.

Notes
-----

The reason I built this is so I can remotely view my camera while it is attached to my telescope (hence the project name) The mount for my telescope is extreamly wobbly, so the less I touch it the better. It is also very hard to focus with the small screen on the back or looking through the view finder.

Why not just use a laptop and the software that comes with your camera? Well that would be too easy. It would also mean I had a working laptop. The end goal of this is to get it working on my Raspberry Pi and remotely view the images on my ipad. So the only wire I will eventually need is the one between the Pi and the camera.

I have tested this with a RaspberryPi and my linux desktop and it works reasonably well. The set up was with a Pi wired into my network, Cannon 550D connected by USB cable and power provided from the mains. I have not hooked it up to my telescope yet, maybe next time there is a clear night.

There is a list of compatible cameras on the [gphoto2 website] (http://www.gphoto.org/doc/remote/)

Project Structure
-----------------

* webserver.c contains the main function along with all the code for dealing with the Mongoose webserver. There are three "services" 
    * /preview - generates a preview image (used for the live preview on the index page)
    * /capture - actually captures an image and returns it to the browser.
    * /summary - returns the summary information from the camera. This can be useful for debugging whats going on.
    * /settings - returns a JSON feed of the settings currently in the camera.
* telescopecamera.c / .h contains all the code for interfacing with the camera.
* stringutils.c / .h some string helper functions.
* mongoose.c / .h all the code for the mongoose web server.

The /capture service takes several query parameters:
* n=[filename] this alows you to pass the name to save the picture with.
* r=[0|1] return the file to the user or not. If 1 returns the file to the user. if not it doesn't and just saves it in the webRoot/img/ directory



