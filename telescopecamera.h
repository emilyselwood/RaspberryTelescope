
#ifndef __TELESCOPE_CAMARA_H
#define __TELESCOPE_CAMARA_H
#include <gphoto2/gphoto2-camera.h>
#include <stdio.h>

/**
 * return summary information from the camera
 * Takes a buffer to put the resulting text in, and the size of the buffer.
 */
int tc_get_summary(char * content, const int size_of);

/**
 * capture an actual picture.
 * Saves the picture to the local disk with the fileName provided.
 * deletes the copy of the picture on the camera
 */
int tc_take_picture(const char * name, const bool delete);


int tc_take_n_pictures(const int n, const char * name, const char * postfix, const bool delete);

/**
 * Captures a preview frame. Saves to the provided fileName/
 * Note this leaves the shutter open.
 */
int tc_preview(const char * name);

/**
 * Cleans up the camera context. Should be used when we are done with it,
 */
void tc_reset();

/**
 * Set a setting in the camera
 */
int tc_set_setting(const char * setting, const char * value);

/**
 * Get a setting value from the camera
 */
int tc_get_setting(const char * setting, char * result, size_t size_of);

/**
 * Writes all the camera settings to the output in JSON format.
 */
int tc_settings(FILE * output);

/**
 * returns true if there appears to be a camera connected,
 */
bool tc_connected();


#endif