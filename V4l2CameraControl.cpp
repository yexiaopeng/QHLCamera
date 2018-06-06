#include "V4l2CameraControl.h"
#include "QDateTime"







//
int V4l2GetJpg(char *camserId, char * jpgpatch, int pictureCcount)
{
    int fd_video;
    FILE* fp_jpg;
    struct buffer *buffers;

    fd_video = open(camserId,O_RDWR);
    if(fd_video < 0 ){
        printf("\r\n # 1 error: %s open error ",camserId);
        return -1;
    }

    //设置视频的格式，720P JPEG
    struct v4l2_format s_fmt;
    s_fmt.fmt.pix.width = 1280;
    s_fmt.fmt.pix.height = 960;
    s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    printf("s_fmt.fmt.pix.pixelformat:%d\n",s_fmt.fmt.pix.pixelformat);
    s_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int flag= ioctl(fd_video,VIDIOC_S_FMT,&s_fmt);
    if(flag != 0)
    {
        printf("set format error\n");
        return -1;
    }

    //creat 3 picture
    for(int i = 0; i < pictureCcount;i++){
        //QDateTime current_date_time =QDateTime::currentDateTime();
        //QString current_date =current_date_time.toString("yyyy.MM.dd_hh:mm:ss.zzz");
        QString current_date = QString::number(i)+".jpg";
        fp_jpg = fopen(current_date.toLatin1().data() ,"wb+");



        //申请1个缓冲区
        struct v4l2_requestbuffers req;
        req.count=1;
        req.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory=V4L2_MEMORY_MMAP;
        ioctl(fd_video,VIDIOC_REQBUFS,&req);
        //缓冲区与应用程序关联

        //申请1个struct buffer空间
        buffers = (struct buffer*)calloc (req.count, sizeof (struct buffer));
        if (!buffers)
        {
            perror ("Out of memory");
            exit (EXIT_FAILURE);
        }

        unsigned int n_buffers;
        for (n_buffers = 0; n_buffers < req.count; n_buffers++)
        {
            struct v4l2_buffer buf;
            memset(&buf,0,sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = n_buffers;
            if (-1 == ioctl (fd_video, VIDIOC_QUERYBUF, &buf))
                exit(-1);
            buffers[n_buffers].length = buf.length;
            buffers[n_buffers].start = mmap (NULL,
                                             buf.length,PROT_READ | PROT_WRITE ,MAP_SHARED,fd_video, buf.m.offset);
            if (MAP_FAILED == buffers[n_buffers].start)
                exit(-1);
        }

        enum v4l2_buf_type type;
        for (n_buffers = 0; n_buffers < req.count; n_buffers++)
        {
            struct v4l2_buffer buf;
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = n_buffers;
            ioctl (fd_video, VIDIOC_QBUF, &buf);
        }


        //开始捕获图像
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl (fd_video, VIDIOC_STREAMON, &type);


        struct v4l2_buffer buf;
        memset(&(buf), 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        //取出图像数据
        ioctl (fd_video, VIDIOC_DQBUF, &buf);
        //保存图像
        fwrite(buffers[buf.index].start,1,buffers[buf.index].length,fp_jpg);//mjpeg
        fflush(fp_jpg);
        //放回缓冲区
        ioctl (fd_video,VIDIOC_QBUF,&buf);

        for (n_buffers = 0; n_buffers < req.count; n_buffers++)
            munmap(buffers[n_buffers].start, buffers[n_buffers].length);
        free(buffers);
        fclose(fp_jpg);

    }


    close(fd_video);

    printf("capture jpg finish..\n");
}
