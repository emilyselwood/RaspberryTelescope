
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
 * Cleans up the camera context. Should be used when we are done with it,
 */
void resetCamera();

/**
 * Set a setting in the camera
 */
int setSetting(const char * setting, const char * newValue);

/**
 * Get a setting value from the camera
 */
int getSetting(const char * setting, char * result, size_t resultLength);

/**
 * Writes all the camera settings to the outputStream in JSON format.
 */
int enumerateSettings(FILE * outputStream);

#endif