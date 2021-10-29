
#include "v4l2_api.h"
#include "camera_cfg.h"
#include "camera_api.h"

#include <stdio.h>  // printf, fprintf
#include <stdlib.h> // calloc
#include <string.h> // memset

#include <linux/fb.h>
#include <linux/videodev2.h>

#include <sys/types.h> // open()
#include <sys/stat.h>  //
#include <fcntl.h>     //

#include <unistd.h>    // close()

#include <sys/ioctl.h> // ioctl()

#define ORQA_ARGS_HM
//==========================================================

enum orqa_error orqa_v4l2_open_video_device(struct orqa_camera_t* camera)
{
  int fd_v4l = open(camera->device_name, O_RDWR, 0);

  if (fd_v4l < 0)
  {
    // oprint_ea("[V4L2 EE] [%s:%d] Error: Failed to open %s for video capture\n", __FILE__, __LINE__, camera->device_name);
    return ORQA_DEVICE_OPEN_ERROR;
  }

  // int report_details = orqa_hm_get_int(ORQA_ARGS_HM, "report-camera-init");
  // if (report_details) { oprint_va("[V4L2 OK] | Successfully opened %s as video capture device with id %d\n", camera->device_name, fd_v4l); }

  // Store the file descriptor ID in the given camera config for later use
  camera->fd_v4l = fd_v4l;

  // if (report_details) { oprint_va("[V4L2 ..] | Checking device %s for video capture capabilities...\n", camera->device_name); }
  int cap_err = orqa_v4l2_ensure_video_capture_cap(fd_v4l);

  if (cap_err != ORQA_OK)
  {
    // oprint_ea("[V4L2 EE] [%s:%d] Error: Device %d capabilities insufficient. Closing the device...", __FILE__, __LINE__, fd_v4l);
    orqa_v4l2_close_device(fd_v4l);
    close(fd_v4l);
    return ORQA_NOT_VIDEO_DEVICE;
  }

  return ORQA_OK;
}

enum orqa_error orqa_v4l2_close_device(int const fd_v4l)
{
  if (fd_v4l != -1)
  {
    // int report_details = orqa_hm_get_int(ORQA_ARGS_HM, "report-camera-init");
    // if (report_details) { oprint_va("[V4L2 II] | Closing device with id %d... ", fd_v4l); }

    if (close(fd_v4l) < 0)
    {
      // if (report_details) { oprint_e("ERROR\n"); }

      // oreport_ea("V4L2", "An error occurred while closing device %d\n", fd_v4l);
      // operror("[V4L2 EE] perror");
      return ORQA_DEVICE_CLOSE_ERROR;
    }

    // if (report_details) { oprint_v("OK\n"); }
  }

  return ORQA_OK;
}

//==========================================================

enum orqa_error orqa_v4l2_get_device_cap(int const fd_v4l, struct v4l2_capability *cap)
{
  if (ioctl(fd_v4l, VIDIOC_QUERYCAP, cap) < 0)
  {
    // oprint_ea("[V4L2 EE] [%s:%d] Error: Failed to query device capabilities. Device id = %d\n", __FILE__, __LINE__, fd_v4l);
    // operror("[V4L2 EE] perror");
    return ORQA_V4L2_CAP_ERROR;
  }

  return ORQA_OK;
}

enum orqa_error orqa_v4l2_ensure_video_capture_cap(int const fd_v4l)
{
  struct v4l2_capability cap;
  int cap_err;

  if ((cap_err = orqa_v4l2_get_device_cap(fd_v4l, &cap)) != ORQA_OK)
  {
    return cap_err;
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
  {
    // oprint_ea("[V4l2 EE] [%s:%d] Error: Device %d doesn't support video capture\n", __FILE__, __LINE__, fd_v4l);

    close(fd_v4l);
    return ORQA_V4L2_CAP_ERROR;
  }

  // int report_details = orqa_hm_get_int(ORQA_ARGS_HM, "report-camera-init");
  // if (report_details) { oprint_va("[V4L2 OK] | Device %d recognized as video capture device\n", fd_v4l); }

  return ORQA_OK;
}

enum orqa_error orqa_v4l2_ensure_userptr_cap(int const fd_v4l)
{
  struct v4l2_capability cap;
  int cap_err;

  if ((cap_err = orqa_v4l2_get_device_cap(fd_v4l, &cap)) != ORQA_OK)
  {
    return cap_err;
  }

  if (!(cap.capabilities & V4L2_CAP_STREAMING))
  {
    // oprint_ea("[V4L2 EE] [%s:%d] Error: Device %d does not support streaming IO. Cannot use userptr streaming\n", __FILE__, __LINE__, fd_v4l);
    close(fd_v4l);
    return ORQA_DEVICE_NO_USERPTR;
  }

  // int report_details = orqa_hm_get_int(ORQA_ARGS_HM, "report-camera-init");
  // if (report_details) { oprint_va("[V4L2 OK] | Device %d supports userptr streaming\n", fd_v4l); }

  return ORQA_OK;
}

enum orqa_error orqa_v4l2_apply_settings(struct orqa_camera_t *camera)
{
  int ioret = 0;

  // int report_details = orqa_hm_get_int(ORQA_ARGS_HM, "report-camera-init");
  // if (report_details) { oprint_va("[V4L2 II] | Applying settings for device %s id %d...\n", camera->device_name, camera->fd_v4l); }

  struct v4l2_streamparm parm;
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  parm.parm.capture.capturemode = camera->v4l2_capture_mode;
  parm.parm.capture.timeperframe.denominator = camera->out.fps_denominator;
  parm.parm.capture.timeperframe.numerator = camera->out.fps_numerator;

  ioret = ioctl(camera->fd_v4l, VIDIOC_S_PARM, &parm);

  if (ioret < 0)
  {
    // oreport_ea("V4L2", "ioctl VIDIOC_S_PARM failed with %d\n", ioret);
    // operror("[V4L2 EE] perror");
    return ORQA_CFG_ERROR;
  }

  struct v4l2_fmtdesc fmtdesc;
  fmtdesc.index = 0;
  fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  ioret = ioctl(camera->fd_v4l, VIDIOC_ENUM_FMT, &fmtdesc);
  if (ioret < 0)
  {
    // oreport_ea("V4L2", "ioctl VIDIOC_ENUM_FMT failed with %d\n", ioret);
    // operror("[V4L2 EE] perror");
    return ORQA_CFG_ERROR;
  }

  // if (report_details) { orqa_cam_print_pixelformat_n(fmtdesc.pixelformat); }
  camera->v4l2_capture_fmt = fmtdesc.pixelformat;

  struct v4l2_frmsizeenum frmsize;
  frmsize.pixel_format = fmtdesc.pixelformat;
  frmsize.index = camera->v4l2_capture_mode;

  ioret = ioctl(camera->fd_v4l, VIDIOC_ENUM_FRAMESIZES, &frmsize);
  if (ioret < 0) // Fatal errors
  {
    // oprint_ea("[V4L2 EE] [%s:%d] Error: ioctl VIDIOC_ENUM_FRAMESIZES failed with %d. Capture mode is %d\n", __FILE__, __LINE__, ioret, camera->v4l2_capture_mode);
    // operror("[V4L2 EE] perror");
    return ORQA_CFG_ERROR;
  }

  struct v4l2_format fmt;

  // if (report_details) { oprint_na("[V4L2 II] | Frame size: WxH = %dx%d (returned by camera device)", frmsize.discrete.width, frmsize.discrete.height); }

  if ((frmsize.discrete.width == 0) || (frmsize.discrete.height == 0))
  {
    // if (report_details) { oprint_n(" - Values ignored\n----------| Device returned degenerate values. Using sane defaults if needed..."); }
    frmsize.discrete.width = ORQA_CAM_WIDTH_DEFAULT;
    frmsize.discrete.height = ORQA_CAM_HEIGHT_DEFAULT;
  }

  // if (report_details) { oprint_n("\n"); }

  if (camera->out.width == 0)
  {
    camera->out.width = frmsize.discrete.width;
    // oprint_va("          | Image width defaulted to %d (provided by device or sane defaults)\n", camera->out.width);
  }

  if (camera->out.height == 0)
  {
    camera->out.height = frmsize.discrete.height;
    // oprint_va("          | Image height defaulted to %d (provided by device or sane defaults)\n", camera->out.height);
  }

  if ((frmsize.discrete.width != camera->out.width) || (frmsize.discrete.height != camera->out.height)) // Warnings only, not fatal
  {
    // if (report_details) { oprint_wa("[V4L2 WW] [%s:%d] Warning: Device returned image dimensions different than what the user provided. User provided %dx%d, device returned %dx%d\n", __FILE__, __LINE__, camera->out.width, camera->out.height, frmsize.discrete.width, frmsize.discrete.height); }

    // if (report_details) { oprint_w("          | Using device-provided values or sane defaults and continuing...\n"); }
    orqa_cam_set_image_dimensions(camera, frmsize.discrete.width, frmsize.discrete.height);
  }

  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.pixelformat = camera->v4l2_capture_fmt;
  fmt.fmt.pix.width = camera->out.width;
  fmt.fmt.pix.height = camera->out.height;

  ioret = ioctl(camera->fd_v4l, VIDIOC_S_FMT, &fmt);
  if (ioret < 0) // Fatal errors
  {
    // oprint_ea("[V4L2 EE] [%s:%d] Error: ioctl VIDIOC_S_FMT failed with %d\n", __FILE__, __LINE__, ioret);
    // operror("[V4L2 EE] perror");
    return ORQA_CFG_ERROR;
  }

  ioret = ioctl(camera->fd_v4l, VIDIOC_G_FMT, &fmt);
  if (ioret < 0) // Fatal errors
  {
    // oprint_ea("[V4L2 EE] [%s:%d] Error: ioctl VIDIOC_G_FMT failed with %d\n", __FILE__, __LINE__, ioret);
    // operror("[V4L2 EE] perror");
    return ORQA_CFG_ERROR;
  }

  memset(&parm, 0, sizeof(parm));
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  ioret = ioctl(camera->fd_v4l, VIDIOC_G_PARM, &parm);

  int fps_denom = parm.parm.capture.timeperframe.denominator;
  int fps_num = parm.parm.capture.timeperframe.numerator;
  int fps = (int)(fps_denom / fps_num);

  // Debug print
  // if (report_details) { oprint_va("[V4L2 ..] | FPS: Device returned denominator = %d, numerator = %d\n", fps_denom, fps_num); }

  if (ioret < 0) // Warnings only, not fatal
  {
    // oprint_wa("[V4L2 WW] [%s:%d] Warning: ioctl VIDIOC_G_PARM failed with %d\n", __FILE__, __LINE__, ioret);
    // oprint_w("[V4L2 WW] perror");
    // oprint_wa("[V4L2 WW] | Falling back to device-returned fps %d. Continuing...\n", fps);

    // orqa_cam_set_fps(camera, fps_denom, fps_num);
  }

  if (fps != camera->out.fps) // Warnings only, not fatal
  {
    // oprint_wa("[V4L2 WW] [%s:%d] Warning: User-specified fps differs from fps returned by the camera device\n", __FILE__, __LINE__);
    // oprint_wa("          | User fps = %d, calculated device fps is %d\n", camera->out.fps, fps);
    // oprint_wa("          | Setting camera config fps to %d and continuing...\n", fps);
    // oprint_w("[V4L2 NN] | Note to developer: Check if the device and its driver support the fps you want them to run with\n");

    // orqa_cam_set_fps(camera, fps_denom, fps_num);
  }

  // if (report_details) { oprint_v("[V4L2 OK] | Settings applied succesfully\n"); }
  // oprint_na("[V4L2 II] | WxH@fps = %dx%d@%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height, parm.parm.capture.timeperframe.denominator);
  // oprint_na("[V4L2 II] | Frame size = %d bytes\n", fmt.fmt.pix.sizeimage);

  camera->out.frame_size = fmt.fmt.pix.sizeimage;

  return ORQA_OK;
}

//==========================================================