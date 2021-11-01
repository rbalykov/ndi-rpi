# RPI NDI monitor
Newtek NDI monitor based on DispmanX.

# Requires
- Raspberry Pi 3 w/ Raspbian OS
- Legacy GL Driver turned on

# Issues
- Although Newtek benchmark tool states Rpi capable to drive 1.84 streams of 1080p60 using 4 cores, library uses single core and no way found to run it multicore. On practice it leads to frame dropping (approx 4 of 5 frames are dropped) and giant latency (more that second). This disaster repeats whenever DispmanX or OpenGLES used to output frames and nothing promices Rpi3 be capable driving 1080p60 using NDI library v5.
- Idea to capture fastest UYVY frames has failed, so code captures BGRX/BGRA and treats it as BGRX.

# Building
- make && ./runner_arm7l

