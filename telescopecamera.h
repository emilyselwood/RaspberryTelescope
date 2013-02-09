
#ifndef __TELESCOPE_CAMARA_H
#define __TELESCOPE_CAMARA_H
#include <gphoto2/gphoto2-camera.h>
#include <stdio.h>

/**
 * return summary information from the camera
 * Takes a buffer to put the resulting text in, and the size of the buffer.
 */
int getCamaraSummary(char * content, int sizeOfContent);

/**
 * capture an actual picture.
 * Saves the picture to the local disk with the fileName provided.
 * deletes the copy of the picture on the camera
 */
int takePicture(char * fileName, bool deleteFromCamera);

/**
 * Captures a preview frame. Saves to the provided fileName/
 * Note this leaves the shutter open.
 */
int capturePreview(char * fileName);

/**
 * cleans up the camera context. Should be used when we are done with it,
 */
void resetCamera();

#endif