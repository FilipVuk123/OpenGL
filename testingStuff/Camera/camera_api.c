#include "camera_cfg.h"
#include "camera_api.h"

#include "v4l2_api.h"

#include <stdio.h>  // printf
#include <stdlib.h> // calloc
#include <string.h> // memcpy

#include <linux/videodev2.h>

#include <sys/ioctl.h> // ioctl()

//============================= Static functions =============================

// Opens the assigned device and checks for relevant capabilities.
// Returns ORQA_OK on success, returns ORQA_CAM_OPEN_ERROR on failure.
static enum orqa_error orqa_open_cam_device(
  ORQA_REF struct orqa_camera_t *const camera);

//==========================================================

struct orqa_camera_t* orqa_camera_create(
  ORQA_IN char const *const device_name,
  ORQA_IN unsigned int const image_width,
  ORQA_IN unsigned int const image_height,
  ORQA_IN unsigned int const slices_per_frame,
  ORQA_IN unsigned int const gop,
  ORQA_IN unsigned int const bitrate)
{
  // int report_inits = orqa_hm_get_int(ORQA_ARGS_HM, "report-inits");
  // if (report_inits) { oprint_va("[CAM  II] | Creating a cam cfg instance for device \"%s\"...\n", device_name); }

  struct orqa_camera_t *const camera = orqa_alloc(sizeof *camera);
  if (camera == NULL) return NULL;

  strcpy(camera->device_name, device_name);

  // Open video capture device. All relevant device capabilities are checked
  if (orqa_open_cam_device(camera)) goto error;

  orqa_cam_set_image_dimensions(camera, image_width, image_height);
  orqa_cam_set_capture_mode(camera, ORQA_CAM_CAPTURE_MODE);

  if (orqa_cam_set_capture_format(camera, ORQA_CAM_CAPTURE_FMT)) goto error;
  if (orqa_cam_set_fps(camera, ORQA_CAM_FPS_DENOM, ORQA_CAM_FPS_NUM)) 
  return NULL;

  // Apply all of the settings that were set above
  if (orqa_cam_apply_v4l2_settings(camera)) goto error;

  // Initialize the h264 encoder
  if (orqa_cam_create_encoder(camera, slices_per_frame, gop, bitrate)) goto error;

  // if (report_inits) { oprint_va("[CAM  OK] | Succesfully initialized \"%s\" as a camera device\n", device_name); }
  return camera;

error:
  // oreport_ea("cam ", "Failed to create camera instance for device %s\n", camera->device_name);

  free(camera);

  return NULL;
}

struct orqa_camera_t *orqa_camera_create_default(
  ORQA_NOARGS void)
{
  return orqa_camera_create(ORQA_CAM_VIDEO_DEVICE, ORQA_CAM_OUT_WIDTH, ORQA_CAM_OUT_HEIGHT, ORQA_HH264ENC_SLICES_PER_FRAME, ORQA_HH264ENC_GOP, ORQA_HH264ENC_BITRATE);
}

enum orqa_error orqa_camera_destroy(
  ORQA_REF void *const camera_ptr)
{
  if (camera_ptr != NULL)
  {
    // int report_destroys = orqa_hm_get_int(ORQA_ARGS_HM, "report-destroys");
    struct orqa_camera_t *const camera = camera_ptr;

    // if (report_destroys) {
      // oprint_v("[cam  II] | Destroying camera instance\n");
      // oprint_va("----------| Device name \"%s\", id %d...\n", camera->device_name, camera->fd_v4l);
    // }

    orqa_v4l2_close_device(camera->fd_v4l);
    // orqa_hh264enc_destroy(camera->enc);

    // if (report_destroys) { oprint_v("[cam  OK] | Camera instance destroyed succesfully\n"); }
  }

  return ORQA_OK;
}

enum orqa_error orqa_open_cam_device(
  ORQA_REF struct orqa_camera_t *const camera)
{
  // Open the device, ensuring it has video capture capabilities
  if (orqa_v4l2_open_video_device(camera) != ORQA_OK) return ORQA_CAM_OPEN_ERROR;

  // Ensure the device supports userptr streaming
  if (orqa_v4l2_ensure_userptr_cap(camera->fd_v4l) !=  ORQA_OK) return ORQA_CAM_OPEN_ERROR;

  return ORQA_OK;
}

void orqa_cam_set_image_dimensions(
  ORQA_REF struct orqa_camera_t *const camera,
  ORQA_IN unsigned int const width,
  ORQA_IN unsigned int const height)
{
  // int report_details = orqa_hm_get_int(ORQA_ARGS_HM, "report-camera-init");
  // if (report_details) { oprint_va("[cam  II] | Setting image dimensions to %dx%d\n", width, height); }

  camera->out.width = width;
  camera->out.height = height;
  camera->out.yuv_bytes = width * height * 1.5;
}

enum orqa_error orqa_cam_set_fps(
  ORQA_REF struct orqa_camera_t *const camera,
  ORQA_IN int const denominator,
  ORQA_IN int const numerator)
{
  int fps = denominator / numerator;

  if ((fps <= ORQA_CAM_FPS_MAX) && (fps >= ORQA_CAM_FPS_MIN))
  {
    camera->out.fps = fps;
    camera->out.fps_denominator = denominator;
    camera->out.fps_numerator = numerator;

    // int report_details = orqa_hm_get_int(ORQA_ARGS_HM, "report-camera-init");
    // if (report_details) { oprintf_va("[cam  OK] | Camera fps set to %d\n", fps); }

    return ORQA_OK;
  }
  else
  {
    // oprint_ea("[cam  EE] [%s:%d] Error: Failed to set camera fps. Given fps is %d, denom. is %d, num. is %d. Fps should be in range [%d, %d]\n", __FILE__, __LINE__, fps, denominator, numerator, ORQA_CAM_FPS_MIN, ORQA_CAM_FPS_MAX);
    return ORQA_CAM_FPS_ERROR;
  }
}

void orqa_cam_set_capture_mode(
  ORQA_REF struct orqa_camera_t *const camera,
  ORQA_IN int const capture_mode)
{
  camera->v4l2_capture_mode = capture_mode;
}

enum orqa_error orqa_cam_set_capture_format(
  ORQA_REF struct orqa_camera_t *const camera,
  ORQA_IN int const capture_fmt)
{
  switch(capture_fmt)
  {
    case ORQA_COLOR_FORMAT_YUYV:
      camera->v4l2_capture_fmt = V4L2_PIX_FMT_YUYV;
      camera->enc_color_format = ORQA_HH264DEC_ENC_COLOR_FORMAT;
      break;

    case ORQA_COLOR_FORMAT_UYVY:
      camera->v4l2_capture_fmt = V4L2_PIX_FMT_UYVY;
      camera->enc_color_format = ORQA_HH264DEC_ENC_COLOR_FORMAT;
      break;

    default:
      // oprint_ea("[cam  EE] [%s:%d] Error: Unknown color format %d\n", __FILE__, __LINE__, capture_fmt);
      return ORQA_CAM_FORMAT_ERROR;
      break;
  }

  return ORQA_OK;
}

enum orqa_error orqa_cam_apply_v4l2_settings(
  ORQA_REF struct orqa_camera_t *const camera)
{
  if (orqa_v4l2_apply_settings(camera)) return ORQA_CAM_ERROR;

  return ORQA_OK;
}

void orqa_cam_print_pixelformat_n(int val)
{
  // oprint_na("[cam  II] | Pixelformat: %c%c%c%c (returned by camera device)\n", val & 0xff, (val >> 8) & 0xff,(val >> 16) & 0xff, (val >> 24) & 0xff);
}
 
void orqa_cam_print_pixelformat_v(int val)
{
  // oprint_va("[cam  II] | Pixelformat: %c%c%c%c (returned by camera device)\n", val & 0xff, (val >> 8) & 0xff,(val >> 16) & 0xff, (val >> 24) & 0xff);
}
 
enum orqa_error orqa_cam_create_encoder(
  ORQA_IN struct orqa_camera_t *const camera,
  ORQA_IN unsigned int const slices_per_frame,
  ORQA_IN unsigned int const gop,
  ORQA_IN unsigned int const bitrate)
{
  // Hantro encoder:
#if 0
  camera->enc = orqa_hh264enc_init(camera->out.width, camera->out.height, camera->out.fps, slices_per_frame, gop, bitrate, camera->enc_color_format);

  if (camera->enc == NULL)
  {
    // oprint_ea("[cam  EE] [%s:%d] Error: Failed to create an encoder instance\n", __FILE__, __LINE__);
    return ORQA_CAM_ENC_INIT_ERROR;
  }

  camera->enc->cfg.fps = camera->out.fps;
#endif

  return ORQA_OK;
}