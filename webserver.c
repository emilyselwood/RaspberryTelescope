 
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "mongoose.h"
#include "telescopecamera.h"
#include "stringutils.h"

#define SUMMARY_URL "/summary"
#define SUMMARY_URL_LENGTH 8

#define CAPTURE_URL "/capture"
#define CAPTURE_URL_LENGTH 8

#define PREVIEW_URL "/preview"
#define PREVIEW_URL_LENGTH 8 


// helper to return short messages
void *returnResult(struct mg_connection *conn, const int status, const char * statusMessage, const char * content, const int content_length) {
	mg_printf(conn,
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %d\r\n" // Always set Content-Length
			"Cache-Control: no-store\n\n" // make sure the browser actually redownloads the image. safari is annoying.
            "\r\n"
            "%s",
            status, statusMessage, content_length, content);
	return "";
}

void *processSummary(struct mg_connection *conn, const struct mg_request_info *request_info) {
	printf("Getting Summary info\n");
	char content[4084];
	int content_length = getCamaraSummary(content, sizeof(content));

	return returnResult(conn, 200, "OK", content, content_length );
}

void *processCapture(struct mg_connection *conn, const struct mg_request_info *request_info) {

	printf("Capturing image\n");

	char resultFileName[500];
	
	char timeStampedName[19];
	time_t t = time(NULL);
	strftime(timeStampedName, 19, "%Y%m%d%H%M%S.jpg", localtime(&t));
	extractStringQueryParamDefault(request_info, "n", timeStampedName, resultFileName, 500);
	
	bool shouldSendBack = extractBoolQueryParam(request_info, "r");

	char outputPath[500];
	sprintf(outputPath, "webRoot/img/%s", resultFileName);

	if(takePicture(outputPath) > 0) {
		if(shouldSendBack) {
			mg_send_file(conn, outputPath);
		}
		else {
			mg_printf(conn,
				"HTTP/1.1 307 Temporary Redirect\r\n"
				"Location: /\r\n"
				"content_length: 0\r\n"
				"Content-Type: text/html\r\n\r\n"
			);
		}
	}
	else {
		const char * message = "Failed to capture image";
		returnResult(conn, 503, message, message, strlen(message));
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
		const char * message = "Failed to capture preview";
		returnResult(conn, 503, message, message, strlen(message));
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