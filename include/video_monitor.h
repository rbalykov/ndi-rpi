/*
 * VideoMonitor.h
 *
 *  Created on: 28 окт. 2021 г.
 *      Author: rbalykov
 */

#ifndef SRC_VIDEO_MONITOR_H_
#define SRC_VIDEO_MONITOR_H_

#include "ndi_receiver.h"
#include "video_frame.h"
#include <bcm_host.h>
#include <vc_dispmanx.h>

#include <exception>
#include <string>
#include <mutex>



class video_monitor
{
public:
	video_monitor(bool* run);
	virtual ~video_monitor();

	int run();

private:
	static void vsync_callback(DISPMANX_UPDATE_HANDLE_T u, void* arg);

	void init();
	void loop();
	void cleanup();
	void init_check(bool x, const char* message);

	std::mutex m_lock;
	bool* m_run;

	unsigned m_display_id;

    DISPMANX_DISPLAY_HANDLE_T   m_display;
    DISPMANX_MODEINFO_T         m_modeinfo;
    DISPMANX_UPDATE_HANDLE_T    m_update;

    ndi_receiver m_ndi;


    class init_exception: public std::exception
    {
    public:
    	explicit init_exception(const char *message) : msg_(message) {}
    	explicit init_exception(const std::string &message) : msg_(message) {}
    	virtual ~init_exception() noexcept {}
    	virtual const char* what() const noexcept { return msg_.c_str(); }
    protected:
    	std::string msg_;
    };

};

#endif /* SRC_VIDEO_MONITOR_H_ */
