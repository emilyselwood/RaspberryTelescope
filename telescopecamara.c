#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "telescopecamara.h"


Camera *camera = NULL;
GPContext *context = NULL;

bool initFailed = false;
bool resetNextTime = false;

int crudeLock = 0;

// error logging function for things failing in the gphoto2 library.
// NOTE : Do not try and reset the camera in here. It will cause seg faults.
static void ctx_error_func (GPContext *context, const char *str, void *data)
{
	fprintf(stderr, "\n*** Contexterror ***\n%s\n", str);
	fflush(stderr);
}

// status logger. These seem to be pretty rare.
static void ctx_status_func (GPContext *context, const char *str, void *data)
{
	fprintf(stderr, "status: %s\n", str);
	fflush(stderr);
}

/*
 * This enables/disables the specific canon capture mode.
 * 
 * For non canons this is not required, and will just return
 * with an error (but without negative effects).
 */
int canon_enable_capture (Camera *camera, int onoff, GPContext *context) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int 				ret;

	/*
	 * This function looks up a label or key entry of
	 * a configuration widget.
	 * The functions descend recursively, so you can just
	 * specify the last component.
	 */
	int _lookup_widget(CameraWidget*widget, const char *key, CameraWidget **child) {
		int ret;
		ret = gp_widget_get_child_by_name (widget, key, child);
		if (ret < GP_OK)
			ret = gp_widget_get_child_by_label (widget, key, child);
		return ret;
	}
	
	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = _lookup_widget (widget, "capture", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup widget failed: %d\n", ret);
		gp_widget_free (widget);
		return ret;
	}

	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		gp_widget_free (widget);
		return ret;
	}
	if (type != GP_WIDGET_TOGGLE) {
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		gp_widget_free (widget);
		return ret;
	}
	/* Now set the toggle to the wanted value */
	ret = gp_widget_set_value (child, &onoff);
	if (ret < GP_OK) {
		fprintf (stderr, "toggling Canon capture to %d failed with %d\n", onoff, ret);
		gp_widget_free (widget);
		return ret;
	}
	/* OK */
	ret = gp_camera_set_config (camera, widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_set_config failed: %d\n", ret);
		return ret;
	}

	gp_widget_free (widget);
	return ret;
}

// Create a constext. used in init.
GPContext* create_context() {
	GPContext *context;

	context = gp_context_new();

	// make sure we log errors. helps work out wtf is going on.
	gp_context_set_error_func (context, (GPContextErrorFunc) ctx_error_func, NULL);
	gp_context_set_status_func (context, (GPContextStatusFunc) ctx_status_func, NULL);

	return context;
}


// function to make sure the camara is initalised
void initCamaraAndContext() {
	
	if(resetNextTime) {
		resetCamera();
		resetNextTime = false;
	}
	
	if( context == NULL && camera == NULL ) {
		context = create_context ();
		gp_camera_new (&camera);
		
		if( gp_camera_init (camera, context) < GP_OK ) {
			initFailed = true;
			resetCamera();
		}
		else {
			canon_enable_capture(camera, TRUE, context);
			initFailed = false;
		}
	}
}

void resetCamera() {
	gp_camera_exit(camera, context);
	gp_camera_free (camera);

	camera = NULL;
	context = NULL;
}

// really really crude locking to stop us trying to either take two pictures at once or generate a preview and capture an image.
// TODO: Make this entire file use some kind of actual atomic lock.
bool reallyCrudeLock() {
	int count = 0;
	while(crudeLock && count < 10) {
		sleep(10);
	}
	if(count == 10) {
		return false;
	}
	else {
		crudeLock++;
	}
	
	return true;
}

/**
 * public method to get the summary information for the camara.
 * 
 * takes a string buffer to put the information in and returns the length writen.
 */
int getCamaraSummary(char * content, const int sizeOfContent) {

	CameraText text;
	int content_length = 0;
	int ret;

	initCamaraAndContext();

	if( initFailed ) {
		content_length = snprintf(content, sizeOfContent, "No camera detected");
	}
	else {
		ret = gp_camera_get_summary (camera, &text, context);
		if (ret < GP_OK) {
			// here we will try and reset the camara so next time some one tries to use it hopefully it will re init.
			resetCamera();
			content_length = snprintf(content, sizeOfContent, "Camera failed retrieving summary.");
		}
		else {
			content_length = snprintf(content, sizeOfContent, "Summary info from camara:\n %s",text.text);
		}
	}

	return content_length;
}

int takePicture(char * fileName) {

	// really really crude locking to stop us trying to either take two pictures at once or generate a preview and capture an image.
	if(!reallyCrudeLock()) {
		return -1;
	}
	
	initCamaraAndContext();

	if( initFailed ) {
		crudeLock--;
		return -1;
	}
	else {
		CameraFile *canonfile;
		CameraFilePath camera_file_path;

		/* NOP: This gets overridden in the library to /capt0000.jpg */
		strcpy(camera_file_path.folder, "/");
		strcpy(camera_file_path.name, "foo.jpg");

		gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_file_path, context);

		int fd = open(fileName, O_CREAT | O_WRONLY, 0644);
		gp_file_new_from_fd(&canonfile, fd);
		gp_camera_file_get(camera, camera_file_path.folder, camera_file_path.name, GP_FILE_TYPE_NORMAL, canonfile, context);

		gp_camera_file_delete(camera, camera_file_path.folder, camera_file_path.name, context);

		gp_file_free(canonfile);
		crudeLock--;
		return fd;
	}
}



int capturePreview(char * fileName) {

	// really really crude locking to stop us trying to either take two pictures at once or generate a preview and capture an image.
	if(!reallyCrudeLock()) {
		return -1;
	}
	
	initCamaraAndContext();
	
	if( initFailed ) {
		crudeLock--;
		return -1;
	}
	else {
		CameraFile *file;
		
		int retval = gp_file_new(&file);
		if (retval != GP_OK) {
			fprintf(stderr,"gp_file_new: %d\n", retval);
			resetCamera();
			crudeLock--;
			return -1;
		}

		retval = gp_camera_capture_preview(camera, file, context);
		if (retval != GP_OK) {
			fprintf(stderr,"gp_camera_capture_preview: %d\n", retval);
			resetNextTime = true;
			crudeLock--;
			return -1;
		}

		retval = gp_file_save(file, fileName);
		if (retval != GP_OK) {
			fprintf(stderr,"gp_file_save: %d\n", retval);
			gp_file_unref(file);
			crudeLock--;
			resetCamera();
			return -1;
		}
		gp_file_unref(file);
		crudeLock--;
		return 0;
	}
}
