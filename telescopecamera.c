#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "stringutils.h"
#include "telescopecamera.h"


Camera *camera = NULL;
GPContext *context = NULL;

bool initFailed = false;
bool resetNextTime = false;

int crudeLock = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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
	* This function looks up a label or key entry of
	* a configuration widget.
	* The functions descend recursively, so you can just
	* specify the last component.
	*/
int _lookup_widget(CameraWidget*widget, const char *key, CameraWidget **child) {
	int ret = gp_widget_get_child_by_name (widget, key, child);
	if (ret < GP_OK) {
		ret = gp_widget_get_child_by_label (widget, key, child);
	}
	return ret;
}
	
int getWidget(CameraWidget ** widget, CameraWidget ** child, const char * setting) {
	CameraWidgetType type;

	int ret = gp_camera_get_config (camera, widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}

	ret = _lookup_widget (*widget, setting, child);
	if (ret < GP_OK || child == NULL) {
		fprintf (stderr, "l lookup widget %s failed: %d\n", setting, ret);
		gp_widget_free (*widget);
		return ret;
	}

	/* This type check is optional, if you know what type the label
	 * has already. If you are not sure, better check. */
	ret = gp_widget_get_type (*child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "l widget get type failed: %d\n", ret);
		gp_widget_free (*widget);
		return ret;
	}

	return GP_OK;
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

	ret = getWidget(&widget, &child, "capture");
	if (ret < GP_OK || child == NULL) {
		fprintf (stderr, "x lookup widget failed: %d\n", ret);
		return ret;
	}

	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "x widget get type failed: %d\n", ret);
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
			canon_enable_capture(camera, 1, context);
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

int waitForCaptureComplete() {
	pthread_mutex_lock(&lock);
	
	int waitTime = 2000;
	CameraEventType type;
	void *data;

	while(1) {
		if(gp_camera_wait_for_event(camera, waitTime, &type, &data, context) == GP_ERROR_NOT_SUPPORTED) {
			printf("camera doesn't support wait.");
			sleep(500); // hopefully this will be enough.
			break;
		}
		if(type == GP_EVENT_TIMEOUT) {
			break;
		}
		else if (type == GP_EVENT_CAPTURE_COMPLETE) {
			waitTime = 100;
		}
		else if (type != GP_EVENT_UNKNOWN) {
			printf("Unexpected event received from camera: %d\n", (int)type);
		}
	}
	pthread_mutex_unlock(&lock);
	return 0;
	
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

int takePicture(const char * fileName, const bool deleteFromCamera) {

	pthread_mutex_lock(&lock);
	
	initCamaraAndContext();

	if( initFailed ) {
		pthread_mutex_unlock(&lock);
		return -1;
	}
	else {
		CameraFile *canonfile;
		CameraFilePath camera_file_path;

		/* NOP: This gets overridden in the library to /capt0000.jpg */
		strcpy(camera_file_path.folder, "/");
		strcpy(camera_file_path.name, fileName);

		gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_file_path, context);

		int fd = open(fileName, O_CREAT | O_WRONLY, 0644);
		gp_file_new_from_fd(&canonfile, fd);
		gp_camera_file_get(camera, camera_file_path.folder, camera_file_path.name, GP_FILE_TYPE_NORMAL, canonfile, context);

		if(deleteFromCamera) {
			gp_camera_file_delete(camera, camera_file_path.folder, camera_file_path.name, context);
		}

		gp_file_free(canonfile);
		pthread_mutex_unlock(&lock);
		return fd;
	}
}

int takeNPictures(const int n, const char * fileName, const char * postfix, const bool deleteFromCamera) {
	int res;
	for(int i = 0; i < n; i++) {
		char entryName[100];
		snprintf(entryName, 100, "%s-%d.%s", fileName, i, postfix);
		res = takePicture(entryName, deleteFromCamera);
		if(res < 0) {
			return -1;
		}
	}
	return 1;
}

int capturePreview(const char * fileName) {

	pthread_mutex_lock(&lock);
	
	initCamaraAndContext();
	
	if( initFailed ) {
		pthread_mutex_unlock(&lock);
		return -1;
	}
	else {
		CameraFile *file;
		
		int retval = gp_file_new(&file);
		if (retval != GP_OK) {
			fprintf(stderr,"gp_file_new: %d\n", retval);
			resetCamera();
			pthread_mutex_unlock(&lock);
			return -1;
		}

		retval = gp_camera_capture_preview(camera, file, context);
		if (retval != GP_OK) {
			fprintf(stderr,"gp_camera_capture_preview: %d\n", retval);
			resetNextTime = true;
			pthread_mutex_unlock(&lock);
			return -1;
		}

		retval = gp_file_save(file, fileName);
		if (retval != GP_OK) {
			fprintf(stderr,"gp_file_save: %d\n", retval);
			gp_file_unref(file);
			resetCamera();
			pthread_mutex_unlock(&lock);
			return -1;
		}
		gp_file_unref(file);
		pthread_mutex_unlock(&lock);
		return 0;
	}
}

int getSetting(const char * setting, char * result, size_t resultLength) {

	pthread_mutex_lock(&lock);
	
	initCamaraAndContext();
	
	if( initFailed ) {
		pthread_mutex_unlock(&lock);
		return -1;
	}
	else {
		CameraWidget * widget = NULL; // will hold the root config entry
		CameraWidget * child  = NULL; // will hold the actual config entry from the tree

		int ret = getWidget(&widget, &child, setting);
		if (ret < GP_OK) {
			fprintf (stderr, "camera_get_config failed: %d\n", ret);
			gp_widget_free (widget);
			pthread_mutex_unlock(&lock);
			return ret;
		}

		/* This is the actual query call. Note that we just
		* a pointer reference to the string, not a copy... */
		char *val;
		ret = gp_widget_get_value (child, &val);
		if (ret < GP_OK) {
			fprintf (stderr, "could not query widget value: %d\n", ret);
			gp_widget_free (widget);
			pthread_mutex_unlock(&lock);
			return ret;
		}

		/* Create a new copy for our caller. */
		snprintf(result, resultLength, "%s", val);

		gp_widget_free (widget);
		pthread_mutex_unlock(&lock);
		return ret;
	}
}

int setSetting(const char * setting, const char * newValue) {
	
	pthread_mutex_lock(&lock);
	initCamaraAndContext();

	if( initFailed ) {
		pthread_mutex_unlock(&lock);
		return -1;
	}
	else {
		CameraWidget * widget = NULL; // will hold the root config entry
		CameraWidget * child  = NULL; // will hold the actual config entry from the tree

		int ret = getWidget(&widget, &child, setting);
		if (ret < GP_OK) {
			fprintf (stderr, "camera_get_config failed: %d\n", ret);
			pthread_mutex_unlock(&lock);
			return ret;
		}
		
		ret = gp_widget_set_value(child, newValue);
		if (ret < GP_OK) {
			fprintf (stderr, "could not set widget value: %d\n", ret);
			gp_widget_free (widget);
			pthread_mutex_unlock(&lock);
			return ret;
		}
		
		/* This stores it on the camera again */
		ret = gp_camera_set_config (camera, widget, context);
		if (ret < GP_OK) {
			fprintf (stderr, "camera_set_config failed: %d\n", ret);
			gp_widget_free (widget);
			pthread_mutex_unlock(&lock);
			return ret;
		}
		
		gp_widget_free (widget);
		pthread_mutex_unlock(&lock);
		return 0;
	}
}

int enumerateSettings(FILE * outputStream) {
	pthread_mutex_lock(&lock);
	initCamaraAndContext();

	if( initFailed ) {
		pthread_mutex_unlock(&lock);
		return -1;
	}
	else {
		CameraWidget * widget = NULL; // will hold the root config entry
		int ret = gp_camera_get_config (camera, &widget, context);
		if (ret < GP_OK) {
			fprintf (stderr, "camera_get_config failed: %d\n", ret);
			gp_widget_free (widget);
			pthread_mutex_unlock(&lock);
			return -1;
		}
		
		void printValue(CameraWidget * widget, const int depth, const char *name, const char * value) {
			int choicesCount;
			
			fprintf(outputStream, "\"%s\" : {\n", name);
			indent(outputStream, depth+1);
			fprintf(outputStream, "\"value\" : \"%s\"", value);
			
			choicesCount = gp_widget_count_choices(widget);
			if(choicesCount > 0) {
				fprintf(outputStream, ",\n");
				indent(outputStream, depth + 1);
				fprintf(outputStream, "\"choices\" : [");
				for(int i = 0; i < choicesCount; i++) {
					const char * choice;
					ret = gp_widget_get_choice(widget, i, &choice);
					if(ret == GP_OK) {
						fprintf(outputStream, "\"%s\"", choice);
					}
					if(i < (choicesCount -1)) {
						fprintf(outputStream, ",");
					}
				}
				fprintf(outputStream, "]\n");
			}
			else {
				fprintf(outputStream, "\n");
			}
			indent(outputStream, depth);
			fprintf(outputStream, "}");
		}
		
		// internal tree walking function
		int displayChildren(CameraWidget * widget, int depth, bool last) {
			const char *name;
			void *value;

			int ret = gp_widget_get_name(widget, &name);
			if(ret < GP_OK) {
				return -1;
			}
			CameraWidgetType type;
			indent(outputStream, depth);
			
			ret = gp_widget_get_type (widget, &type);
			switch (type) {
				case GP_WIDGET_WINDOW:
				case GP_WIDGET_SECTION:
					fprintf(outputStream, "\"%s\"", name);
					
					int count = gp_widget_count_children(widget);
					if(count > 0) {
						fprintf(outputStream, " : {\n");
					}
					int newDepth = depth + 1;
					for (int i = 0; i < count; i++) {
						CameraWidget * child = NULL;
						gp_widget_get_child(widget, i, &child);
						
						displayChildren(child, newDepth, (i == (count - 1)) );
					}
					
					if(count > 0) {
						indent(outputStream, depth);
						fprintf(outputStream, "}");
					}
				break;
				case GP_WIDGET_MENU:
				case GP_WIDGET_RADIO:
				case GP_WIDGET_TEXT:
					gp_widget_get_value(widget, &value);
					printValue(widget, depth, name, (char *) value);
				break;	
				case GP_WIDGET_TOGGLE:	// with these the value of the pointer is the actual value.
				case GP_WIDGET_DATE:
					gp_widget_get_value(widget, &value);
					char str[15];
					snprintf(str, 15, "%d", ((int)value));
					
					printValue(widget, depth, name, str);
				break;
			default:
				fprintf(outputStream, "\"%s\" : \"has bad type %d\"", name, type);
			}
			
			if(!last) {
				fprintf(outputStream, ",");
			}
			fprintf(outputStream, "\n");
			
			return 0;
		}
		
		fprintf(outputStream, "{\n");
		ret = displayChildren(widget, 1, true);
		fprintf(outputStream, "}\n");
		
		fflush(outputStream);
		
		gp_widget_free(widget);
		pthread_mutex_unlock(&lock);
		return ret;
	}
}
