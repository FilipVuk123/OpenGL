#ifndef ORQA_CAMERA_CFG
#define ORQA_CAMERA_CFG

//=========================================================

enum orqa_color_format
{
  ORQA_COLOR_FORMAT_YUYV    = 1, // caCamera
  ORQA_COLOR_FORMAT_UYVY    = 2  // ovCamera
};

//=========================================================

#define ORQA_CAM_VIDEO_DEVICE "/dev/video0"

#define ORQA_CAM_OUT_WIDTH   1280
#define ORQA_CAM_OUT_HEIGHT   720
#define ORQA_CAM_IMAGE_PIXELS (ORQA_CAM_OUT_WIDTH * ORQA_CAM_OUT_HEIGHT)
#define ORQA_CAM_YUV_IMAGE_PIXELS (ORQA_CAM_IMAGE_PIXELS * 1.5)

#define ORQA_CAM_CAPTURE_MODE 3 // 1280x720p - No idea if this actually changes anything. I took the comment from old code that "just works". Might be driver-specific

#define ORQA_CAM_FPS_MAX   60
#define ORQA_CAM_FPS_MIN   15
#define ORQA_CAM_FPS_DENOM 60
#define ORQA_CAM_FPS_NUM    1
#define ORQA_CAM_FPS (int)(ORQA_FPS_DENOM / ORQA_FPS_NUM)

// This macro applies changes both to the camera and the encoder
// It is intended to be the unified way of changing the format so feel free to change it here
#define ORQA_CAM_CAPTURE_FMT ORQA_COLOR_FORMAT_UYVY

// I don't know what exactly will happen if you change these values. Changing the buffer count to 2 will slow down the camera device, setting it to 3 will make the entire Linux system misbehave. 4 seems to be the best choice 
#define ORQA_CAM_USERPTR_BUFFERS_COUNT 4
#define ORQA_CAM_STREAM_MEMORY_TYPE V4L2_MEMORY_USERPTR // Don't change this. Other methods not implemented

//=========================================================

// Don't change these two values. They are used as fallback values for V4L2 API when no sane values are provided by the user or the device
#define ORQA_CAM_WIDTH_DEFAULT   1280 // Don't change this
#define ORQA_CAM_HEIGHT_DEFAULT   720 // Don't change this

//=========================================================

#define ORQA_HH264ENC_SLICES_PER_FRAME 40
#define ORQA_HH264ENC_GOP 30
#define ORQA_HH264ENC_BITRATE 10000
#define ORQA_HH264DEC_ENC_COLOR_FORMAT 3

#endif // ORQA_CAMERA_CFG