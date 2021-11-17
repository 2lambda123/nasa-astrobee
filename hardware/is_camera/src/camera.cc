/* Copyright (c) 2017, United States Government, as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * 
 * All rights reserved.
 * 
 * The Astrobee platform is licensed under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with the
 * License. You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
// To create symbolic link to device: udevadm info -q all -a -n /dev/video0
// Create rule in /etc/udev/rules.d, e.g.:
// SUBSYSTEM=="video4linux", ATTRS{serial}=="02510265",SYMLINK+="nav_cam"

#include <is_camera/camera.h>
#include <camera/camera_params.h>

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/imgproc/imgproc.hpp>

#include <fcntl.h>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

namespace is_camera {

  static void xioctl(int fh, int request, void *arg) {
    int r = -1;
    int tries = 0;

    do {
      r = v4l2_ioctl(fh, request, arg);
      if (r == 1) {
        ROS_ERROR("error... %d %d %d", errno, errno == EINTR, errno == EAGAIN);
      }
      tries++;
    } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)) && tries <= 3);

    if (r == -1) {
      ROS_FATAL("ioctl error %d, %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  struct V4LStruct {
   public:
    struct v4l2_format              fmt;
    struct v4l2_buffer              buf;
    struct v4l2_requestbuffers      req;
    struct v4l2_streamparm          parm;
    enum v4l2_buf_type              type;
    fd_set                          fds;
    int fd;
    struct timeval                  tv;
    struct buffer *buffers;

    explicit V4LStruct(std::string const& device_name, int camera_gain, int camera_exposure) {
      // Open the device
      fd = v4l2_open(device_name.c_str(), O_RDWR | O_NONBLOCK, 0);
      if (fd < 0) {
        ROS_FATAL("Cannot open device.");
        exit(EXIT_FAILURE);
      }

      // Initialize the v4l2 fmt
      memset(&fmt, 0, sizeof(fmt));
      fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      fmt.fmt.pix.width       = CameraNodelet::kImageWidth;
      fmt.fmt.pix.height      = CameraNodelet::kImageHeight;
      fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
      fmt.fmt.pix.field       = V4L2_FIELD_NONE;
      xioctl(fd, VIDIOC_S_FMT, &fmt);
      if ((fmt.fmt.pix.width  != CameraNodelet::kImageWidth) ||
          (fmt.fmt.pix.height != CameraNodelet::kImageHeight)) {
        ROS_FATAL("Driver is sending image at %dx%d\n",
                  fmt.fmt.pix.width, fmt.fmt.pix.height);
        exit(EXIT_FAILURE);
      }

      // See what the capture rate is
      //
      parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      xioctl(fd, VIDIOC_G_PARM, &parm);
      parm.parm.output.timeperframe.denominator = 15;
      xioctl(fd, VIDIOC_S_PARM, &parm);
      xioctl(fd, VIDIOC_G_PARM, &parm);

      // Initialize the v4l2 buffers
      memset(&req, 0, sizeof(fmt));
      req.count = 2;
      req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      req.memory = V4L2_MEMORY_MMAP;
      xioctl(fd, VIDIOC_REQBUFS, &req);

      // Attach the buffers to an mmap
      buffers = static_cast<buffer*>(calloc(req.count, sizeof(*buffers)));
      for (size_t n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        memset(&buf, 0, sizeof(buf));

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = n_buffers;

        xioctl(fd, VIDIOC_QUERYBUF, &buf);

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start = v4l2_mmap(NULL, buf.length,
            PROT_READ | PROT_WRITE, MAP_SHARED,
            fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start) {
          ROS_FATAL("Unable to mmap.");
          exit(EXIT_FAILURE);
        }
      }

      memset(&buf, 0, sizeof(buf));
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = 0;
      xioctl(fd, VIDIOC_QBUF, &buf);
      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      xioctl(fd, VIDIOC_STREAMON, &type);

      SetParameters(camera_gain, camera_exposure);
    }

    ~V4LStruct() {
      // Stop the streaming
      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      xioctl(fd, VIDIOC_STREAMOFF, &type);
      for (size_t i = 0; i < req.count; ++i)
        v4l2_munmap(buffers[i].start, buffers[i].length);

      // Close the device
      v4l2_close(fd);
    }

    void SetParameters(int gain, int exposure) {
      // Set the camera gain
      struct v4l2_control ctrl;
      memset(&ctrl, 0, sizeof(ctrl));
      ctrl.id = V4L2_CID_GAIN;
      ctrl.value = gain;
      ioctl(fd, VIDIOC_S_CTRL, &ctrl);
      // Set the exposure
      memset(&ctrl, 0, sizeof(ctrl));
      ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
      ctrl.value = exposure;
      ioctl(fd, VIDIOC_S_CTRL, &ctrl);
    }
  };

  CameraNodelet::CameraNodelet() : ff_util::FreeFlyerNodelet(),
    img_msg_buffer_idx_(0), thread_running_(false), camera_topic_(""), calibration_mode_(false)
    {}

  CameraNodelet::~CameraNodelet() {
    thread_running_ = false;
    thread_.join();
    v4l_.reset();
  }

  void CameraNodelet::Initialize(ros::NodeHandle* nh) {
    calibration_mode_ = false;
    config_name_ = GetName();
    if (GetName() == "nav_cam") {
      camera_topic_ = TOPIC_HARDWARE_NAV_CAM;
    } else if (GetName() == "dock_cam") {
      camera_topic_ = TOPIC_HARDWARE_DOCK_CAM;
    } else if (GetName() == "calibration_nav_cam") {
      camera_topic_ = TOPIC_HARDWARE_NAV_CAM;
      config_name_ = "nav_cam";
      calibration_mode_ = true;
    } else if (GetName() == "calibration_dock_cam") {
      camera_topic_ = TOPIC_HARDWARE_DOCK_CAM;
      config_name_ = "dock_cam";
      calibration_mode_ = true;
    } else {
      FF_FATAL("Unknown camera driver name.");
    }

    config_.AddFile("cameras.config");
    ReadParams();
    config_timer_ = GetPrivateHandle()->createTimer(ros::Duration(1), [this](ros::TimerEvent e) {
      config_.CheckFilesUpdated(std::bind(&CameraNodelet::ReadParams, this));}, false, true);

    pub_ = nh->advertise<sensor_msgs::Image>(camera_topic_, 1);
    info_pub_ = nh->advertise<sensor_msgs::CameraInfo>(camera_topic_ + "/camera_info", 1);

    // Allocate space for our output msg buffer
    for (size_t i = 0; i < kImageMsgBuffer; i++) {
      img_msg_buffer_[i].reset(new sensor_msgs::Image());
      img_msg_buffer_[i]->width  = kImageWidth;
      img_msg_buffer_[i]->height = kImageHeight;
      img_msg_buffer_[i]->encoding = "mono8";
      img_msg_buffer_[i]->step   = kImageWidth;
      img_msg_buffer_[i]->data.resize(kImageWidth * kImageHeight);
    }

    v4l_.reset(new V4LStruct(camera_device_, camera_gain_, camera_exposure_));
    thread_running_ = true;
    thread_ = std::thread(&CameraNodelet::PublishLoop, this);
  }

  void CameraNodelet::ReadParams(void) {
    if (!config_.ReadFiles()) {
      ROS_ERROR("Failed to read config files.");
      return;
    }

    config_reader::ConfigReader::Table camera;
    if (!config_.GetTable(config_name_.c_str(), &camera)) {
      FF_FATAL("Device config not found in LUA.");
      exit(EXIT_FAILURE);
    }
    if (!camera.GetStr("device", &camera_device_)) {
      FF_FATAL("Camera device not specified.");
      exit(EXIT_FAILURE);
    }

    if (!camera.GetInt("gain", &camera_gain_)) {
      FF_FATAL("Gain not specified.");
      exit(EXIT_FAILURE);
    }

    if (!camera.GetInt("exposure", &camera_exposure_)) {
      FF_FATAL("Gain not specified.");
      exit(EXIT_FAILURE);
    }

    // overwrite gain and exposure in calibration mode
    if (calibration_mode_) {
      if (!camera.GetInt("calibration_gain", &camera_gain_)) {
        FF_FATAL("Calibration gain not specified.");
        exit(EXIT_FAILURE);
      }

      if (!camera.GetInt("calibration_exposure", &camera_exposure_)) {
        FF_FATAL("Calibration exposure not specified.");
        exit(EXIT_FAILURE);
      }
    }

    LoadCameraInfo();

    if (thread_running_) {
      v4l_->SetParameters(camera_gain_, camera_exposure_);
    }
  }

  void CameraNodelet::LoadCameraInfo() {
    camera::CameraParameters cam_params(&config_, config_name_.c_str());

    // Load parameters
    Eigen::Matrix3d K = cam_params.GetIntrinsicMatrix<camera::RAW>();
    Eigen::VectorXd D = cam_params.GetDistortion();
    Eigen::Vector2i size = cam_params.GetDistortedSize();

    // Populate message
    info_msg_.width = size[0];
    info_msg_.height = size[1];
    info_msg_.K = {K(0,0), K(0,1), K(0,2),
                   K(1,0), K(1,1), K(1,2),
                   K(2,0), K(2,1), K(2,2)};
    if (D.size() == 1) {
      info_msg_.distortion_model = "fov";
      info_msg_.D = {D[0]};
    } else if (D.size() == 4 || D.size() == 5) {
      info_msg_.distortion_model = "plumb_bob";
      info_msg_.D = {D[0], D[1], D[2], D[3], 0};
      if (D.size() > 4) {
        info_msg_.D[4] = D[4];
      }
    }

    // Don't use projection or rotation, but populate
    info_msg_.P = {1, 0, 0, 0,
                   0, 1, 0, 0,
                   0, 0, 1, 0};
    info_msg_.R = {1, 0, 0,
                   0, 1, 0,
                   0, 0, 1};
  }

  void CameraNodelet::PublishLoop() {
    bool camera_running = true;

    while (thread_running_) {
      if (!camera_running) {
        while ((pub_.getNumSubscribers() == 0) && thread_running_)
          usleep(100000);
        if (!thread_running_)
          break;
        memset(&v4l_->buf, 0, sizeof(v4l_->buf));
        v4l_->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4l_->buf.memory = V4L2_MEMORY_MMAP;
        v4l_->buf.index = 0;
        xioctl(v4l_->fd, VIDIOC_QBUF, &v4l_->buf);
        camera_running = true;
      }
      int r = -1;
      do {
        FD_ZERO(&v4l_->fds);
        FD_SET(v4l_->fd, &v4l_->fds);

        /* Timeout. */
        v4l_->tv.tv_sec = 2;
        v4l_->tv.tv_usec = 0;

        r = select(v4l_->fd + 1, &v4l_->fds, NULL, NULL, &v4l_->tv);
      } while ((r == -1 && (errno = EINTR)));
      if (r == -1) {
        FF_FATAL("Select failed.");
        exit(EXIT_FAILURE);
      }

      memset(&v4l_->buf, 0, sizeof(v4l_->buf));
      v4l_->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      v4l_->buf.memory = V4L2_MEMORY_MMAP;
      xioctl(v4l_->fd, VIDIOC_DQBUF, &v4l_->buf);
      bool failed = (v4l_->buf.bytesused != v4l_->buf.length);
      if (failed) {
        ROS_ERROR("Dropped frame: %d Requested: %d Flags: %x", v4l_->buf.bytesused, v4l_->buf.length, v4l_->buf.flags);
      }
      int last_buf = v4l_->buf.index;

      v4l_->buf.index = (last_buf + 1) % v4l_->req.count;
      if (pub_.getNumSubscribers() != 0)
        xioctl(v4l_->fd, VIDIOC_QBUF, &v4l_->buf);
      else
        camera_running = false;
      ros::Time timestamp = ros::Time::now();

      // Select our output msg buffer
      img_msg_buffer_idx_ = (img_msg_buffer_idx_ + 1) % kImageMsgBuffer;

      if (!failed) {
        // Wrap the buffer with cv::Mat so we can manipulate it.
        cv::Mat wrapped(v4l_->fmt.fmt.pix.height,
            v4l_->fmt.fmt.pix.width,
            cv::DataType<uint8_t>::type,
            v4l_->buffers[last_buf].start,
            v4l_->fmt.fmt.pix.width);  // does not copy
        cv::Mat owrapped(kImageHeight, kImageWidth,
            cv::DataType<uint8_t>::type,
            &(img_msg_buffer_[img_msg_buffer_idx_]->data[0]),
            kImageWidth);
        cv::cvtColor(wrapped, owrapped,
            CV_BayerGR2GRAY);

        // Attach the time
        img_msg_buffer_[img_msg_buffer_idx_]->header = std_msgs::Header();
        img_msg_buffer_[img_msg_buffer_idx_]->header.stamp = timestamp;

        info_msg_.header = img_msg_buffer_[img_msg_buffer_idx_]->header;
        pub_.publish(img_msg_buffer_[img_msg_buffer_idx_]);
        info_pub_.publish(info_msg_);
      }

      ros::spinOnce();
    }
  }
}  // end namespace is_camera

// Register nodelet
#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(is_camera::CameraNodelet, nodelet::Nodelet);

