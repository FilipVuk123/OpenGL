#ifndef ORQA_CAMERA_H
#define ORQA_CAMERA_H

#include "camera_cfg.h"
#include <stdint.h>

#include <linux/videodev2.h>

//========================================================

// Function is designed to take no arguments.
#define ORQA_NOARGS

// Marks a pure input argument.
// The given argument must not be modified.
// The given argument must be a valid instance.
// Argument should be marked as const.
#define ORQA_IN

// Marks an input argument that will possibly be changed.
// The given argument may or may not be modified.
// The given argument must be a valid instance.
// Argument shouldn't be marked as const.
#define ORQA_REF

// Marks a pure output argument that will possibly be changed.
// The given argument may or may not be modified.
// The given argument may or may not be a valid instance
// Argument shouldn't be marked as const.
#define ORQA_OUT

//========================================================

typedef enum orqa_error{
    ORQA_OK                         = 0,
    ORQA_CAM_OPEN_ERROR             = -1,
    ORQA_CAM_FPS_ERROR              = -2,
    ORQA_CAM_FORMAT_ERROR           = -3,
    ORQA_CAM_ERROR                  = -4,
    ORQA_DEVICE_OPEN_ERROR          = -1,
    ORQA_NOT_VIDEO_DEVICE           = -2,
    ORQA_VIDEO_DEVICE_CLOSE_ERROR   = -3,
    ORQA_DEVICE_CLOSE_ERROR         = -4,
    ORQA_CFG_ERROR                  = -5,
    ORQA_V4L2_CAP_ERROR             = -6,
    ORQA_DEVICE_NO_USERPTR          = -7,
};

// This struct stores all relevant device settings and is used to apply them with ioctl calls.
struct orqa_camera_t
{
  int fd_v4l;
  char device_name[32];

  int v4l2_capture_mode;
  int v4l2_capture_fmt;
  int enc_color_format;

  struct
  {
    volatile uint64_t frame_size;
    volatile unsigned int width;
    volatile unsigned int height;

    unsigned int yuv_bytes;

    uint16_t fps;
    uint16_t fps_denominator;
    uint16_t fps_numerator;
  } out;

  // struct orqa_hh264_encoder_t *enc;
};

//================= Camera configuration API ===============

// Allocates the camera config struct and sets the device name. Does NOT call any configuration functions for the created instance.
// Returns a valid pointer on success, returns NULL on failure.
struct orqa_camera_t* orqa_camera_create(
  ORQA_IN char const *const device_name,
  ORQA_IN unsigned int const image_width,
  ORQA_IN unsigned int const image_height,
  ORQA_IN unsigned int const slices_per_frame,
  ORQA_IN unsigned int const gop,
  ORQA_IN unsigned int const bitrate);

// Creates and configures a camera configuration instance and applies all default settings from <orqa/common.h>
// Returns a valid pointer on success, returns NULL on failure.
struct orqa_camera_t* orqa_camera_create_default(
  ORQA_NOARGS void);

// Frees the camera config and its members. Closes the associated file descriptor for used device.
enum orqa_error orqa_camera_destroy(
  ORQA_REF void *const camera_ptr);

//=================== Setter functions =====================

/**
 * Setter functions make NO CHANGES/CALLS WITH THE CAMERA DEVICE until 'orqa_cam_apply_v4l2_settings()' is executed. Set all the options you want then call the apply settings function once.
 */

// Sets the image dimensions without making any device driver calls.
// Use this function before calling the apply settings function.
void orqa_cam_set_image_dimensions(
  ORQA_REF struct orqa_camera_t *const camera,
  ORQA_IN unsigned int const width,
  ORQA_IN unsigned int const height);

// Sets the fps for the configuration. Fps is calculated as '(int)(denominator / numerator)'. The resulting value should be in range [ORQA_CAM_FPS_MIN, ORQA_CAM_FPS_MAX].
// Use this function before calling the apply settings function.
// Returns ORQA_OK on success, returns ORQA_CAM_FPS_ERROR on failure.
enum orqa_error orqa_cam_set_fps(
  ORQA_REF struct orqa_camera_t *const camera,
  ORQA_IN int const denominator,
  ORQA_IN int const numerator);

// Sets the capture mode. Apparently changing the capture mode does nothing at the moment, we'll have to see about it with some other camera driver.
// Use this function before calling the apply settings function.
// Always returns ORQA_OK.
void orqa_cam_set_capture_mode(
  ORQA_REF struct orqa_camera_t *const camera,
  ORQA_IN int const capture_mode);

// Sets the capture format. Use a value from the OrqaCaptureFormat enum.
// Use this function before calling the apply settings function.
// Returns ORQA_OK on success, returns ORQA_CAM_FORMAT_ERROR on failure.
enum orqa_error orqa_cam_set_capture_format(
  ORQA_REF struct orqa_camera_t *const camera,
  ORQA_IN int const capture_fmt);

//========================================================

// Attemps to apply the settings with V4L2 and the device.
// Returns ORQA_OK on success, returns ORQA_CAM_ERROR on failure.
enum orqa_error orqa_cam_apply_v4l2_settings(
  ORQA_REF struct orqa_camera_t *const camera);

// Prints [normal print] the capture format that was returned by the camera while applying settings
void orqa_cam_print_pixelformat_n(
  ORQA_IN int const val);

// Prints [verbose print] the capture format that was returned by the camera while applying settings
void orqa_cam_print_pixelformat_v(
  ORQA_IN int const val);

// Creates and initializes the encoder instance, passing the current configuration parameters to the encoder.
// Returns ORQA_OK on success, returns ORQA_CAM_ENC_INIT_ERROR on failure.
enum orqa_error orqa_cam_create_encoder(
  ORQA_IN struct orqa_camera_t *const camera,
  ORQA_IN unsigned int const slices_per_frame,
  ORQA_IN unsigned int const gop,
  ORQA_IN unsigned int const bitrate);

//========================================================

#endif // ORQA_CAMERA_H