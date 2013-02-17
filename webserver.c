// need this for open_memstream
// this is only going to work on linux for now (windows on a Raspberry Pi? We'll leave that to others.)
#define _GNU_SOURCE

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <libconfig.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mongoose.h"
#include "telescopecamera.h"
#include "stringutils.h"
#include "fileutils.h"

#define SUMMARY_URL "/summary"
#define SUMMARY_URL_LENGTH 8

#define CAPTURE_URL "/capture"
#define CAPTURE_URL_LENGTH 8

#define PREVIEW_URL "/preview"
#define PREVIEW_URL_LENGTH 8

#define SETTINGS_URL "/settings"
#define SETTINGS_URL_LENGTH 9

#define SET_SETTING_URL "/setsetting"
#define SET_SETTING_URL_LENGTH 11

#define LIST_IMAGES_URL "/listimages"
#define LIST_IMAGES_URL_LENGTH 11

#define IMAGE_URL "/image"
#define IMAGE_URL_LENGTH 6

// global configuration as this will be accessed in the call backs.
config_t cfg;

// helper to return short messages
void *returnResult(struct mg_connection *conn, const int status, const char * statusMessage, const char * content, const int content_length) {
	mg_printf(conn,
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %d\r\n" // Always set Content-Length
			"Cache-Control: no-store\n\n" // make sure the browser actually redownloads the image. safari is annoying.
            "\r\n"
            "%s",
            status, statusMessage, content_length+2, content);
	return "";
}

void * returnError(struct mg_connection *conn, const int status, const char * statusMessage) {
	return returnResult(conn, status, statusMessage, statusMessage, n_strlen(statusMessage));
}

void *processSummary(struct mg_connection *conn, const struct mg_request_info *request_info) {
	printf("Getting Summary info\n");
	char content[4084];
	int content_length = tc_get_summary(content, sizeof(content));

	return returnResult(conn, 200, "OK", content, content_length );
}

void *processCapture(struct mg_connection *conn, const struct mg_request_info *request_info) {

	printf("Capturing image\n");

	int captureCount = int_query_param(request_info, "i");
	char resultFileName[500];

	char timeStampedName[19];
	time_t t = time(NULL);
	if(captureCount > 1) {
		strftime(timeStampedName, 19, "%Y%m%d%H%M%S", localtime(&t));
	}
	else {
		strftime(timeStampedName, 19, "%Y%m%d%H%M%S.jpg", localtime(&t));
	}
	str_query_param_def(request_info, "n", timeStampedName, resultFileName, 500);

	if(contains_path_chars(resultFileName)) {
		printf("Invalid file name %s\n", resultFileName);
		return returnError(conn, 400, "Invalid file name");
	}
	
	bool shouldSendBack = bool_query_param(request_info, "r");
	bool shouldDelete = bool_query_param(request_info, "d");

	const char *path;
	config_lookup_string(&cfg, "capture.save_path", &path);
	
	char outputPath[500];
	sprintf(outputPath, "%s%s", path, resultFileName);

	int res;
	if(captureCount > 1) {
		printf("about to try and capture %d pictures with name %s\n", captureCount, outputPath);
		res = tc_take_n_pictures(captureCount, outputPath, "jpg", shouldDelete);
	}
	else {
		res = tc_take_picture(outputPath, shouldDelete);
	}
	
	if(res > 0) {
		if(shouldSendBack && (captureCount <= 1)) {
			mg_send_file(conn, outputPath);
		}
		else {
			mg_printf(conn,
				"HTTP/1.1 307 Temporary Redirect\r\n"
				"Location: /?%s\r\n"
				"content_length: 0\r\n"
				"Content-Type: text/html\r\n\r\n",
				request_info->query_string 
			);
		}
	}
	else {
		returnError(conn, 503, "Failed to capture image");
	}

	return "";
}

void *processPreview(struct mg_connection *conn, const struct mg_request_info *request_info) {
	const char * resultFileName = "preview.jpg";

	const char *path;
	config_lookup_string(&cfg, "capture.preview_path", &path);

	char outputPath[500];
	sprintf(outputPath, "%s/%s", path, resultFileName);

	if(tc_preview(outputPath) == 0) {
		mg_send_file(conn, outputPath);
	}
	else {
		returnError(conn, 503, "Failed to capture preview");
	}

	return "";
}

void * processSettings(struct mg_connection *conn, const struct mg_request_info *request_info) {
	char * buffer;
    size_t size;
	FILE * stream = open_memstream(&buffer, &size);
	
	tc_settings(stream);
	
	fclose(stream);
	returnResult(conn, 200, "OK", buffer, size);
	free(buffer);
	return "";
}

void * processSetSetting(struct mg_connection *conn, const struct mg_request_info *request_info) {
	char key[50];
	char value[50];

	int keyLength = str_query_param(request_info, "k", key, 50);
	int valueLength = str_query_param(request_info, "v", value, 50);
	
	if(keyLength <= 0 || valueLength <= 0) {
		return returnError(conn, 400, "Missing setting information");
	}
	else {
		int res = tc_set_setting(key, value);
		if(res == 0) {
			return returnResult(conn, 200, "OK", "", 0);	
		}
		else {
			return returnError(conn, 503, "Failed to change setting");
		}
	}
}

void * processListImages(struct mg_connection *conn, const struct mg_request_info *request_info) {

	char * buffer;
    size_t size;
	FILE * stream = open_memstream(&buffer, &size);

	const char *path;
	config_lookup_string(&cfg, "capture.save_path", &path);
	
	list_img_dir(path, stream);

	fclose(stream);
	returnResult(conn, 200, "OK", buffer, size);
	free(buffer);
	return "";
}


void * processImage(struct mg_connection *conn, const struct mg_request_info *request_info) {
	char name[100];

	int nameLength = str_query_param(request_info, "n", name, 100);
	
	if(nameLength <= 0) {
		return returnError(conn, 400, "Missing image name");
	}
	
	if(contains_path_chars(name)) {
		return returnError(conn, 400, "Invalid image name");
	}
	
	const char * path;
	config_lookup_string(&cfg, "capture.save_path", &path);
	
	int fullLength = n_strlen(path) + nameLength + 1;
	
	char * fullpath = (char*)malloc(sizeof(char) * fullLength + 1);
	
	snprintf(fullpath, fullLength, "%s%s", path, name); 
	
	printf("sending image: %s\n", fullpath);
	mg_send_file(conn, fullpath);
	
	free(fullpath);
	return "";
	
}

static void *callback(enum mg_event event, struct mg_connection *conn) {


	if (event == MG_NEW_REQUEST) {
		const struct mg_request_info *request_info = mg_get_request_info(conn);
		// before loading up the index page check for existence of a camera.
		if(strcmp(request_info->uri, "/") == 0) {
			if(tc_connected()) {
				return NULL; // return null so it goes to the normal index page.
			}
			else {
				mg_printf(conn,
					"HTTP/1.1 307 Temporary Redirect\r\n"
					"Location: /nocamera.lp\r\n"
					"content_length: 0\r\n"
					"Content-Type: text/html\r\n\r\n" 
				);
				return "";
			}
		}
		// check for internal urls and then if we don't find one of our magic ones fall back.
		if( strcmp(request_info->uri, SUMMARY_URL) == 0 ){
			return processSummary(conn, request_info);
		}
		else if( strcmp(request_info->uri, CAPTURE_URL) == 0 ) {
			return processCapture(conn, request_info);
		}
		else if( strncmp(request_info->uri, PREVIEW_URL, PREVIEW_URL_LENGTH) == 0 ) { // strncmp so we look for preview but ignore cache breaker.
			return processPreview(conn, request_info);
		}
		else if( strcmp(request_info->uri, SETTINGS_URL) == 0) {
			return processSettings(conn, request_info);
		}
		else if( strncmp(request_info->uri, SET_SETTING_URL, SET_SETTING_URL_LENGTH) == 0) {
			return processSetSetting(conn, request_info);
		}
		else if( strncmp(request_info->uri, LIST_IMAGES_URL, LIST_IMAGES_URL_LENGTH) == 0) {
			return processListImages(conn, request_info);
		}
		else if(strncmp(request_info->uri, IMAGE_URL, IMAGE_URL_LENGTH) == 0) {
			return processImage(conn, request_info);
		}
		else {
			// if we return null it falls out the end of this handler and tries the document_root to see if a file exists there.
			return NULL;
		}
	}
	else {
		return NULL;
	}
}

void clean_up() {
	config_destroy(&cfg);
	tc_reset();
}

// signal handler so we actually get some idea where the hell this thing crashes.
// very helpful with gphoto2 being completely non thread safe.
void bt_sighandler(int sig) {

	void *trace[16];
	char **messages = (char **)NULL;
	int i, trace_size = 0;

	printf("Got signal %d\n", sig);

	trace_size = backtrace(trace, 16);

	messages = backtrace_symbols(trace, trace_size);
	/* skip first stack frame (points here) */
	printf("[bt] Execution path:\n");
	for (i=1; i<trace_size; ++i)
	{
		printf("[bt] #%d %s\n", i, messages[i]);

		char syscom[256];
		sprintf(syscom,"addr2line %p -e raspberrytelescope", trace[i]); //last parameter is the name of this app
		system(syscom);
	}

	exit(0);
}

int main(void) {

	signal(SIGSEGV, bt_sighandler);   // install our handler

	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "config.cfg"))
	{
		fprintf(stderr, "Error reading config file:\nLine %d - %s\n",
				config_error_line(&cfg), config_error_text(&cfg));
		clean_up();
		return -1;
	}

	// check that the preview directory exists.
	const char *path;
	config_lookup_string(&cfg, "capture.preview_path", &path);
	if(!dir_exits(path)) {
		fprintf(stderr, "Error preview path doesn't seem to exist. %s\n", path);
		clean_up();
		return -2;
	}
	
	const char *savePath;
	config_lookup_string(&cfg, "capture.save_path", &savePath);
	if(!dir_exits(savePath)) {
		fprintf(stderr, "Error save path doesn't seem to exist. %s\n", savePath);
		clean_up();
		return -2;
	}
	
	const char *port;
	config_lookup_string(&cfg, "webserver.port", &port);
	
	struct mg_context *ctx;
	const char *options[] = {
		"listening_ports", port, //"8080",
		"document_root", "./webRoot",
		"index_files", "index.lp,index.html,index.htm",
		"enable_directory_listing", "no",
		NULL
	};

	// this actually starts the web server.
	ctx = mg_start(&callback, NULL, options);
	getchar(); // Wait until user hits "enter"
	mg_stop(ctx);

	clean_up();
	return 0;
}