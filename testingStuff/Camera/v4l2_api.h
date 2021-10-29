#ifndef ORQA_V4L2_API_H
#define ORQA_V4L2_API_H

//========================================================

#include <linux/videodev2.h>
#include "camera_api.h"

//========================================================

// Opens the given device using the name set in camera->device_name.
// After the device is opened a call to "orqa_v4l2_ensure_video_capture_cap" is made to ensure that the device supports video capturing.
// Returns ORQA_OK on success, returns ORQA_DEVICE_OPEN_ERROR on device opening error and ORQA_NOT_VIDEO_DEVICE if the device doesn't support video capture.
enum orqa_error orqa_v4l2_open_video_device(struct orqa_camera_t *camera);

// Closes the device associated with the file descriptor index.
// Returns ORQA_OK on success and ORQA_DEVICE_CLOSE_ERROR on failure.
enum orqa_error orqa_v4l2_close_device(int const fd_v4l);

// Fills the given capability struct through a call to VIDIOC_QUERYCAP.
// Returns ORQA_OK on success and ORQA_V4L2_CAP_ERROR on failure.
enum orqa_error orqa_v4l2_get_device_cap(int const fd_v4l, struct v4l2_capability *cap);

// Checks if the given device supports video capture.
// Returns ORQA_OK if support is reported by the device, returns ORQA_NOT_VIDEO_DEVICE otherwise.
enum orqa_error orqa_v4l2_ensure_video_capture_cap(int const fd_v4l);

// Checks if the given device supports userptr streaming.
// Returns ORQA_OK on success and ORQA_DEVICE_NO_USERPTR on failure.
enum orqa_error orqa_v4l2_ensure_userptr_cap(int const fd_v4l);

// Attempts to apply all the previously set settings with the device.
// Returns ORQA_OK on success, returns ORQA_V4L2_ERROR on failure.
enum orqa_error orqa_v4l2_apply_settings(struct orqa_camera_t *camera);

//========================================================

#endif // ORQA_V4L2_API_H