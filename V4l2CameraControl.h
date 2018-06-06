#ifndef V4L2CAMERACONTROL_H
#define V4L2CAMERACONTROL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

struct buffer
{
    void* start;
    unsigned int length;
};


extern int V4l2GetJpg(char * camserId, char *jpgpatch, int pictureCcount);

#ifdef __cplusplus
}
#endif
#endif // V4L2CAMERACONTROL_H
