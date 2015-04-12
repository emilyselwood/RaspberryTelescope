Raspberry Telescope
===================

A project to allow remote control of a digital camera through a web browser.

Features
--------

* Live preview
* Remote Triggering
* Adjust Camera settings (iso, shutter speed, etc)
* Capture to server or return immediately to browser
* Burst Mode
* Controlled through a web browser
* Browse captured images
* Raw support (If you have a cannon with CR2 raw)

Road Map
-------

* Time laps (Sort of working, time between shots is a little erratic however)
* Capture video (May be impossible with current LibGPhoto2)


Building and Running
--------------------

To get started on a Raspberry PI you will need to run the following commands in sequence. This assumes you are in the home directory of a fresh raspbian distro with an active internet connection.

    sudo apt-get update
    sudo apt-get install git libgphoto2-2-dev libconfig-dev
    git clone git://github.com/wselwood/RaspberryTelescope.git
    cd RaspberryTelescope
    make dependencies raspberrytelescope
    ./raspberytelescope

Connect your camera and turn it on. In a web browser connect to http://raspberrypi:8080/ and you should see the live preview from your camera.

On a pi the live preview can be a bit low frame rate. Half second or so refresh.

Make Targets:
* raspberrytelescope : Builds every thing at the moment.
* clean : clean
* dist-clean : Clean out the JQuery-UI folder
* dependencies : Download the JQuery, JQuery-UI and Theme files.

At the moment there a few warnings in the mongoose.c file and one in telescopecamera.c about a pointer being cast
to an integer if you are building on a 64bit machine.

To run the web server simply execute the raspberrytelescope command. To exit press enter in the shell thats running the webserver. If a time lapse is running it will ask you to confirm by pressing enter again.

Dependencies
---------

We need libgphoto2 and libconfig which can be got from you favorite package manager.

JQuery-UI and JQuery are downloaded by using the dependencies target on make. The version number is included in the top of the make file.

This also uses Mongoose  but the needed files are included here. The original Mongoose files can be found here : https://github.com/valenok/mongoose

The JQuery files are included in the build rather than pulled from the CDN urls in the html as this needs to work with out an internet connection, for instance where the Pi is working as a wi-fi hot spot. However we do require an internet connection to set up the Pi (You have to download it from some where after all).

Notes
-----

The reason I built this is so I can remotely view my camera while it is attached to my telescope (hence the project name) The mount for my telescope is extreamly wobbly, so the less I touch it the better. It is also very hard to focus with the small screen on the back or looking through the view finder.

Why not just use a laptop and the software that comes with your camera? Well that would be too easy. It would also mean I had a working laptop. The end goal of this is to get it working on my Raspberry Pi and remotely view the images on my iPad. So the only wire I will eventually need is the one between the Pi and the camera.

I have tested this with a RaspberryPi and my linux desktop and it works reasonably well. The set up was with a Pi wired into my network, Cannon 550D connected by USB cable and power provided from the mains. I have not hooked it up to my telescope yet, maybe next time there is a clear night.

There is a list of compatible cameras on the [gphoto2 website] (http://www.gphoto.org/doc/remote/)
Cameras tested with so far:
Cannon 550D
Cannon G10 (Preview image is tiny and the settings use weird enums but they do work)

If you have been able to use this with any other cameras please let us know.

Early versions of this code had a dependency on Lua which has been removed.

Project Structure
-----------------

* webserver.c contains the main function along with all the code for dealing with the Mongoose webserver.
* telescopecamera.c / .h contains all the code for interfacing with the camera.
* timelapse.c / .h contains all the code for dealing with timelapses.
* stringutils.c / .h some string helper functions.
* fileutils.c /.h code for dealing with the file system.
* mongoose.c / .h all the code for the mongoose web server.
* telescope.c / .h code for talking to a telescope. Experimental.

Services
--------

* /preview - generates a preview image (used for the live preview on the index page)
* /capture - actually captures an image and returns it to the browser.
* /summary - returns the summary information from the camera. This can be useful for debugging whats going on.
* /settings - returns a JSON feed of the settings currently in the camera.
* /setsetting - takes a key and value and updates a setting on the camera.
* /listimages - returns a json file listing of all the images in the save folder. Does not return hidden files. Result is inode ordered. Sort client side.
* /image - takes the name of an image and returns it to the requester. Abstracts away the path images are saved to.
* /timelapse - starts a time lapse sequence.
* /tlstatus - returns the status of a time lapse. Number of frames remaining etc.
* /tlcancel - stops a running timelapse.
* /telescope - send messages to move the axis of a telescope.

The /capture service takes several query parameters:
* n=[filename] this alows you to pass the name to save the picture with.
* r=[0|1] return the file to the user or not. If 1 returns the file to the user. If not it doesn't. Either way it will save it in the webRoot/img/ directory
* d=[0|1] Should the file be removed from the camera once it has been downloaded and saved by the server.
* c=[0|1] Should the picture be copied from the camera to the server. (Note with this off and the delete option on you will lose the picture)
* i=[0-9999] capture a number of images, will always capture at least one image even if you send zero. Note the return option only works if this is 0, 1 or not set.

The /setsetting service takes two query parameters:
* k=[setting name] this is the name of the setting you want to update.
* v=[new value] this is the new value for the setting you want to update.

The /image service is restful so that browsers can cache the images returned. Quite simple the next bit of the path after the /image/ is the file name.

The /timelapse service takes several query parameters:
* n=[prefix] this will be the start of all the file names created from this time lapse.
* i=[0-99999] number of images to take.
* t=[0-99999] number of seconds between frames.

The /telescope service takes a single query parameter for the axis that will be passed through to your telescope over the web controller. At the moment this is highly experimental and relies on access to an ultra-scope.
* a=[0-9999] number for the axis.
