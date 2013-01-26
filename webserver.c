 
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "mongoose.h"
#include "telescopecamara.h"

#define SUMMARY_URL "/summary"
#define SUMMARY_URL_LENGTH 8

#define CAPTURE_URL "/capture"
#define CAPTURE_URL_LENGTH 8

#define PREVIEW_URL "/preview"
#define PREVIEW_URL_LENGTH 8 



// helper to return short messages
void *returnOkResult(struct mg_connection *conn, const int status, const char * content, const int content_length) {
	mg_printf(conn,
            "HTTP/1.1 %d OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %d\r\n" // Always set Content-Length
			"Cache-Control: no-store\n\n" // make sure the browser actually redownloads the image. safari is annoying.
            "\r\n"
            "%s",
            status, content_length, content);
	return "";
}

void *processSummary(struct mg_connection *conn, const struct mg_request_info *request_info) {
	printf("Getting Summary info\n");
	char content[4084];
	int content_length = getCamaraSummary(content, sizeof(content));

	return returnOkResult(conn, 200, content, content_length );
}

void *processCapture(struct mg_connection *conn, const struct mg_request_info *request_info) {
	printf("Capturing image\n");
	const char * resultFileName = "result.jpg";

	char outputPath[500];
	sprintf(outputPath, "webRoot/%s", resultFileName);

	if(takePicture(outputPath) ) {

		mg_send_file(conn, outputPath);
	}
	else {
		const char * message = "failed to capture image";
		returnOkResult(conn, 503, "failed to capture image", strlen(message));
	}

	return "";
}

void *processPreview(struct mg_connection *conn, const struct mg_request_info *request_info) {
	const char * resultFileName = "preview.jpg";
	
	char outputPath[500];
	sprintf(outputPath, "/tmp/%s", resultFileName);
	
	if(capturePreview(outputPath) == 0) {
		mg_send_file(conn, outputPath);
	}
	else {
		const char * message = "failed to capture preview";
		returnOkResult(conn, 503, "failed to capture preview", strlen(message));
	}
	
	return "";
}


static void *callback(enum mg_event event, struct mg_connection *conn) {


	if (event == MG_NEW_REQUEST) {
		const struct mg_request_info *request_info = mg_get_request_info(conn);
		
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
		else {
			// if we return null it falls out the end of this handler and tries the document_root to see if a file exists there.
			return NULL;
		}
	}
	else {
		return NULL;
	}
}

// signal handler so we actually get some idea where the hell this thing crashes.
// very helpful with gphoto2 failing some time after a request has come through.
void handler(int sig) {
  void *array[20];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 20);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(1);
}


int main(void) {

	signal(SIGSEGV, handler);   // install our handler

	struct mg_context *ctx;
	const char *options[] = {
		"listening_ports", "8080",
		"document_root", "./webRoot",
		"index_files", "index.lp,index.html,index.htm",
		"enable_directory_listing", "no",
		NULL
	};

	// this actually starts the web server.
	ctx = mg_start(&callback, NULL, options);
	getchar(); // Wait until user hits "enter"
	mg_stop(ctx);

	// make sure we reset the camara.
	resetCamera();
	
	return 0;
}