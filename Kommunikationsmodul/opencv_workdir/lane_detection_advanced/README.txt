

# COMMAND TO READ GSTREAM FROM RASPBERRY AND SAVE IN MP4 FORMAT
gst-launch-1.0 -e udpsrc port=5000 caps = "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! h264parse ! mp4mux ! filesink location=res/camera-test2.mp4

# COMMAND TO READ GSTREAM FROM RASPBERRY LIVE
gst-launch-1.0 -e udpsrc port=5000 caps = "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! autovideosink