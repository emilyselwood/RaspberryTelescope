Raspberry Telescope
===================

This a simple project to allow remote control of a digital camara through a web browser.

Features
--------

* Live preview
* Remote Triggering
* Controlled through a web browser.

Roadmap
-------

* Capture to server
* Browse captured images
* Adjust Camera settings (iso, shutter speed, etc)
* Time laps 
* Capture video

Depencies
---------

We need lua and libgphoto2

`sudo apt-get install libgphoto2-2-dev`

That should get you libgphoto2 if your using a debian based system

lua can be got from www.lua.org/ftp/ Ive used version 5.2.1
Download and compile. 
At the top of the make file there is a path to where you downloaded and built lua. Defaults to "lua-5.2.1/"

This also uses Mongoose but the needed files are included here, the origional ones can be found here : https://github.com/valenok/mongoose

Building
--------
Run make

Targets:
* all : Builds every thing
* raspberrytelescope : Builds every thing at the moment.
* clean : clean

At the moment there is one warning generated in the mongoose.c file about an int being cast to a pointer.

Notes
-----

The reason I built this is so I can remotely view my camera while it is attached to my telescope (hence the project name) The mount for my telescope is extreamly wobbly, so the less I touch it the better. It is also very hard to focus with the small screen on the back or looking through the view finder.

Why not just use a laptop and the software that comes with your camera? Well that would be too easy. It would also mean I had a working laptop. The end goal of this is to get it working on my Raspberry Pi and remotely view the images on my ipad. So the only wire I will eventually need is the one between the Pi and the camera.

As yet I haven't tested this on the pi. So far it has only been running on my linux desktop, with a Cannon 550D There is a list of compatible cameras on the [gphoto2 website] (http://www.gphoto.org/doc/remote/)

Project Structure
-----------------

The main function is in webserver.c along with all the code for dealing with the Mongoose webserver. There are three "services" 
* /preview - generates a preview image (used for the live preview on the index page)
* /capture - actually captures an image and returns it to the browser.
* /summary - returns the summary information from the camera. This can be useful for debugging whats going on.

telescopecamera.c / .h contains all the code for interfacing with the camera.

mongoose.c / .h all the code for the mongoose web server.


