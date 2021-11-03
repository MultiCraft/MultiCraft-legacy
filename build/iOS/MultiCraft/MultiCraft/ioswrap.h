#pragma once

#ifdef OFFICIAL
#import "ads.h"
#else
#define ZIPPWD @"1" // zip password
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum {
	PATH_DOCUMENTS,
	PATH_LIBRARY_SUPPORT,
	PATH_LIBRARY_CACHE,
};

void ioswrap_init();

void ioswrap_paths(int type, char *dest, size_t destlen);

void ioswrap_assets(void); // extracts assets.zip to PATH_LIBRARY_SUPPORT

float ioswrap_scale();

void ioswrap_show_dialog(void *uiviewcontroller, const char *accept, const char *hint, const char *current, int type);
int ioswrap_get_dialog(const char **text);

void ioswrap_init_viewc(void *uiviewcontroller);

void ioswrap_events(int event);
void ioswrap_server_connect(bool multiplayer);
void ioswrap_exit_game();

void ioswrap_open_url(const char *url);

#ifdef __cplusplus
}
#endif
