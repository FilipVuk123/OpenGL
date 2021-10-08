#include "display.h"


#include <stddef.h>

#define TRUE 1
#define FALSE 0

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

struct window;
struct seat;
struct wl_compositor *compositor = NULL;
struct wl_surface *surface;
struct wl_egl_window *egl_window;
struct wl_region *region;
struct wl_shell *shell;
struct wl_shell_surface *shell_surface;

struct _escontext ESContext =
    {
        .native_display = NULL,
        .window_width = 0,
        .window_height = 0,
        .native_window = 0,
        .display = NULL,
        .context = NULL,
        .surface = NULL};

EGLBoolean CreateEGLContext();
EGLBoolean CreateWindowWithEGLContext(char *title, int width, int height);
void RefreshWindow();
void CreateNativeWindow(char *title, int width, int height);

void CreateNativeWindow(char *title, int width, int height)
{

  region = wl_compositor_create_region(compositor);

  wl_region_add(region, 0, 0, width, height);
  wl_surface_set_opaque_region(surface, region);

  struct wl_egl_window *egl_window =
      wl_egl_window_create(surface, width, height);

  if (egl_window == EGL_NO_SURFACE)
    exit(1);

  else
    printf("Window created !\n");
  ESContext.window_width = width;
  ESContext.window_height = height;
  ESContext.native_window = egl_window;
}

DISReturn CreateEGLContext()
{
  EGLint numConfigs=0;
  EGLint majorVersion;
  EGLint minorVersion;
  EGLContext context;
  EGLSurface surface;
  EGLConfig config;
  EGLint fbAttribs[] =
  {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_NONE
  };
  EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE};
  EGLDisplay display = eglGetDisplay(ESContext.native_display);

  if (display == EGL_NO_DISPLAY)
  {
    printf("No EGL Display...\n");
    return DIS_EGL_FAIL;
  }

  // Initialize EGL
  if (!eglInitialize(display, &majorVersion, &minorVersion))
  {
    printf("No Initialisation...\n");
    return DIS_EGL_FAIL;
  }

  if(!eglBindAPI(EGL_OPENGL_ES_API))
  {
    printf("OpenGLES2 API not set...\n");
    return DIS_EGL_FAIL;
  }

  // Get configs
  if ((eglGetConfigs(display, NULL, 0, &numConfigs) != EGL_TRUE) || (numConfigs == 0))
  {
    printf("No configuration...\n");
    return DIS_EGL_FAIL;
  }

  // Choose config
  if ((eglChooseConfig(display, fbAttribs, &config, 1, &numConfigs) != EGL_TRUE) || (numConfigs != 1))
  {
    printf("No configuration...\n");
    return DIS_EGL_FAIL;
  }

  // Create a surface
  surface = eglCreateWindowSurface(display, config, ESContext.native_window, NULL);
  if (surface == EGL_NO_SURFACE)
  {
    printf("No surface...\n");
    return DIS_EGL_FAIL;
  }

  // Create a GL context
  context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
  if (context == EGL_NO_CONTEXT)
  {
    printf("No context... %x\n", eglGetError());
    return DIS_EGL_FAIL;
  }

  // Make the context current
  if (!eglMakeCurrent(display, surface, surface, context))
  {
    printf("Could not make the current window current !\n");
    return DIS_EGL_FAIL;
  }
  
  ESContext.display = display;
  ESContext.surface = surface;
  ESContext.context = context;
  return DIS_EGL_OK;
}

void shell_surface_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial)
{
  wl_shell_surface_pong(shell_surface, serial);
}

void shell_surface_configure(void *data, struct wl_shell_surface *shell_surface, uint32_t edges,
                             int32_t width, int32_t height)
{
  struct window *window = data;
  wl_egl_window_resize(ESContext.native_window, width, height, 0, 0);
}

void shell_surface_popup_done(void *data, struct wl_shell_surface *shell_surface)
{
}

static struct wl_shell_surface_listener shell_surface_listener =
    {
        &shell_surface_ping,
        &shell_surface_configure,
        &shell_surface_popup_done};

DISReturn CreateWindowWithEGLContext(char *title, int width, int height)
{
  CreateNativeWindow(title, width, height);
  return CreateEGLContext();
}

void RefreshWindow() { eglSwapBuffers(ESContext.display, ESContext.surface); }

static void global_registry_handler(void *data, struct wl_registry *registry, uint32_t id,
                                    const char *interface, uint32_t version)
{
  printf("Got a registry event for %s id %d\n", interface, id);
  if (strcmp(interface, "wl_compositor") == 0)
    compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);

  else if (strcmp(interface, "wl_shell") == 0)
    shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
}

static void global_registry_remover(void *data, struct wl_registry *registry, uint32_t id)
{
  printf("Got a registry losing event for %d\n", id);
}

const struct wl_registry_listener listener =
    {
        global_registry_handler,
        global_registry_remover};

static DISReturn
get_server_references()
{

  struct wl_display *display = wl_display_connect(NULL);
  if (display == NULL)
  {
    printf("Can't connect to wayland display !?\n");
    exit(1);
  }

  struct wl_registry *wl_registry =
      wl_display_get_registry(display);
  wl_registry_add_listener(wl_registry, &listener, NULL);

  // This call the attached listener global_registry_handler
  wl_display_dispatch(display);
  wl_display_roundtrip(display);

  // If at this point, global_registry_handler didn't set the
  // compositor, nor the shell, bailout !
  if (compositor == NULL || shell == NULL)
    return DIS_WAYLAND_FAIL;
  else
    ESContext.native_display = display;
}

void destroy_window()
{
  wl_surface_destroy(surface);
  wl_shell_surface_destroy(shell_surface);
  wl_egl_window_destroy(ESContext.native_window);
  wl_display_disconnect(ESContext.native_display);
  eglDestroySurface(ESContext.display, ESContext.surface);
  eglDestroyContext(ESContext.display, ESContext.context);
}

void initialize_display(int screen_width, int screen_height)
{
  get_server_references();

  surface = wl_compositor_create_surface(compositor);
  if (surface == NULL)
    exit(1);

  shell_surface = wl_shell_get_shell_surface(shell, surface);
  wl_shell_surface_set_toplevel(shell_surface);

  CreateWindowWithEGLContext("Nya", screen_width, screen_height);
}

void display_dispatch()
{
  wl_display_dispatch_pending(ESContext.native_display);
}