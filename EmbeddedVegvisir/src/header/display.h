#ifndef _DISPLAY_H
#define _DISPLAY_H


#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <signal.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-server.h>
#include <wayland-client-protocol.h>

#include <EGL/egl.h>
#include <EGL/eglplatform.h>

typedef enum DISReturn
{
	DIS_WAYLAND_FAIL,
	DIS_WAYLAND_OK,
	DIS_EGL_FAIL,
	DIS_EGL_OK,
	DIS_GLES2_FAIL,
	DIS_GLES2_OK,
	DIS_OK
} DISReturn;


struct _escontext
{
  /// Native System informations
  EGLNativeDisplayType native_display;
  EGLNativeWindowType native_window;
  uint16_t window_width, window_height;
  /// EGL display
  EGLDisplay  display;
  /// EGL context
  EGLContext  context;
  /// EGL surface
  EGLSurface  surface;

};

struct display 
{
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_shell *shell;
	struct wl_seat *seat;
	struct wl_pointer *pointer;
	struct wl_keyboard *keyboard;
	struct wl_shm *shm;
	struct wl_cursor_theme *cursor_theme;
	struct wl_cursor *default_cursor;
	struct wl_surface *cursor_surface;
	struct 
    {
		EGLDisplay dpy;
		EGLContext ctx;
		EGLConfig conf;
	} egl;
	struct window *window;
};

struct geometry 
{
	int width, height;
};

struct window 
{
	struct display *display;
	struct geometry geometry, window_size;
	struct wl_egl_window *native;
	struct wl_surface *surface;
	struct wl_shell_surface *shell_surface;
	EGLSurface egl_surface;
	struct wl_callback *callback;
	int fullscreen, configured, opaque;
};

void initialize_display(int screen_width, int screen_height);
void display_dispatch();
void RefreshWindow();
DISReturn CreateEGLContext();
void destroy_window();

#endif