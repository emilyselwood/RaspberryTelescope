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

// big camera lock. Used around all the public functions to prevent multiple threads trying to access this at the same time.
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
		tc_reset();
		resetNextTime = false;
	}
	
	if( context == NULL && camera == NULL ) {
		context = create_context ();
		gp_camera_new (&camera);
		
		if( gp_camera_init (camera, context) < GP_OK ) {
			initFailed = true;
			tc_reset();
		}
		else {
			canon_enable_capture(camera, 1, context);
			initFailed = false;
		}
	}
}

void tc_reset() {
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
int tc_get_summary(char * content, const int size_of) {

	CameraText text;
	int content_length = 0;
	int ret;

	initCamaraAndContext();

	if( initFailed ) {
		content_length = snprintf(content, size_of, "No camera detected");
	}
	else {
		ret = gp_camera_get_summary (camera, &text, context);
		if (ret < GP_OK) {
			// here we will try and reset the camara so next time some one tries to use it hopefully it will re init.
			tc_reset();
			content_length = snprintf(content, size_of, "Camera failed retrieving summary.");
		}
		else {
			content_length = snprintf(content, size_of, "Summary info from camara:\n %s",text.text);
		}
	}

	return content_length;
}

int tc_take_picture(const char * name, const bool delete) {

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
		strcpy(camera_file_path.name, name);

		gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_file_path, context);

		int fd = open(name, O_CREAT | O_WRONLY, 0644);
		gp_file_new_from_fd(&canonfile, fd);
		gp_camera_file_get(camera, camera_file_path.folder, camera_file_path.name, GP_FILE_TYPE_NORMAL, canonfile, context);

		if(delete) {
			gp_camera_file_delete(camera, camera_file_path.folder, camera_file_path.name, context);
		}

		gp_file_free(canonfile);
		pthread_mutex_unlock(&lock);
		return fd;
	}
}

int tc_take_n_pictures(const int n, const char * name, const char * postfix, const bool delete) {
	int res;
	for(int i = 0; i < n; i++) {
		char entryName[100];
		snprintf(entryName, 100, "%s-%d.%s", name, i, postfix);
		res = tc_take_picture(entryName, delete);
		if(res < 0) {
			return -1;
		}
	}
	return 1;
}

int tc_preview(const char * name) {

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
			tc_reset();
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

		retval = gp_file_save(file, name);
		if (retval != GP_OK) {
			fprintf(stderr,"gp_file_save: %d\n", retval);
			gp_file_unref(file);
			tc_reset();
			pthread_mutex_unlock(&lock);
			return -1;
		}
		gp_file_unref(file);
		pthread_mutex_unlock(&lock);
		return 0;
	}
}

bool tc_connected() {
	pthread_mutex_lock(&lock);
	// if we can init the camera we assume its connected.
	initCamaraAndContext();
	
	if( initFailed ) {
		pthread_mutex_unlock(&lock);
		return false;
	}
	pthread_mutex_unlock(&lock);
	return true;
}
	

int tc_get_setting(const char * setting, char * result, size_t size_of) {

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
		snprintf(result, size_of, "%s", val);

		gp_widget_free (widget);
		pthread_mutex_unlock(&lock);
		return ret;
	}
}

int tc_set_setting(const char * setting, const char * value) {
	
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
		
		ret = gp_widget_set_value(child, value);
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

int tc_settings(FILE * output) {
	pthread_mutex_lock(&lock);
	initCamaraAndContext();

	if( initFailed ) {
		pthread_mutex_unlock(&lock);
		return -1;
	}
	else {
		CameraWidget * widget = NULL; // will hold the root config entry
			// the config entries make a tree structure.
			// each node has differnt types. Only some can be branches
			// others are always leaves.
		int ret = gp_camera_get_config (camera, &widget, context);
		if (ret < GP_OK) {
			fprintf (stderr, "camera_get_config failed: %d\n", ret);
			gp_widget_free (widget);
			pthread_mutex_unlock(&lock);
			return -1;
		}
		
		// Helper function to print an entry out, needs to be passed the value as a string.
		void printValue(CameraWidget * widget, const int depth, const char *name, const char * value) {
			int choicesCount;
			
			fprintf(output, "\"%s\" : {\n", name);
			indent(output, depth+1);
			fprintf(output, "\"value\" : \"%s\"", value);
			
			// If this setting has a fixed list of options we will return those as well.
			// For instance iso can be one of "Auto","100","200","400","800","1600","3200","6400" on a Cannon 550D
			choicesCount = gp_widget_count_choices(widget);
			if(choicesCount > 0) {
				fprintf(output, ",\n");
				indent(output, depth + 1);
				fprintf(output, "\"choices\" : [");
				for(int i = 0; i < choicesCount; i++) {
					const char * choice;
					ret = gp_widget_get_choice(widget, i, &choice);
					if(ret == GP_OK) {
						fprintf(output, "\"%s\"", choice);
					}
					if(i < (choicesCount -1)) {
						fprintf(output, ",");
					}
				}
				fprintf(output, "]\n");
			}
			else {
				fprintf(output, "\n");
			}
			indent(output, depth);
			fprintf(output, "}");
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
			indent(output, depth);
			
			ret = gp_widget_get_type (widget, &type);
			switch (type) {
				case GP_WIDGET_WINDOW:
				case GP_WIDGET_SECTION: // These are branch nodes.
					fprintf(output, "\"%s\"", name);
					
					int count = gp_widget_count_children(widget);
					if(count > 0) {
						fprintf(output, " : {\n");
					}
					int newDepth = depth + 1;
					for (int i = 0; i < count; i++) {
						CameraWidget * child = NULL;
						gp_widget_get_child(widget, i, &child);
						
						displayChildren(child, newDepth, (i == (count - 1)) );
					}
					
					if(count > 0) {
						indent(output, depth);
						fprintf(output, "}");
					}
				break;
				case GP_WIDGET_MENU:
				case GP_WIDGET_RADIO:
				case GP_WIDGET_TEXT: // these are all easy text nodes
					gp_widget_get_value(widget, &value);
					printValue(widget, depth, name, (char *) value);
				break;	
				case GP_WIDGET_TOGGLE:	
				case GP_WIDGET_DATE: // with these the pointer returned is the value rather than a pointer.
					gp_widget_get_value(widget, &value);
					char str[15];
					snprintf(str, 15, "%d", ((int)value));
					
					printValue(widget, depth, name, str);
				break;
			default:
				fprintf(output, "\"%s\" : \"has bad type %d\"", name, type);
			}
			
			// json output, no commas after the last object
			if(!last) {
				fprintf(output, ",");
			}
			fprintf(output, "\n");
			
			return 0;
		}
		
		// the main bit of code for this function.
		fprintf(output, "{\n");
		ret = displayChildren(widget, 1, true);
		fprintf(output, "}\n");
		
		fflush(output);
		
		gp_widget_free(widget);
		pthread_mutex_unlock(&lock);
		return ret;
	}
}
