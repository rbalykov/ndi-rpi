/*
 * VideoMonitor.cpp
 *
 *  Created on: 28 окт. 2021 г.
 *      Author: rbalykov
 */

#include <iostream>
#include <chrono>
#include "video_monitor.h"
#include <vc_dispmanx.h>

video_monitor::video_monitor()
{
	// TODO Auto-generated constructor stub
	m_run = true;
	m_display_id = 0;
	m_display = 0;
	m_update = 0;
}

video_monitor::~video_monitor()
{
	// TODO Auto-generated destructor stub
	std::cerr << "hello world" << std::endl;
}

void video_monitor::init_check(bool x, const char* message)
{
	if (x) throw init_exception(message);
}


int video_monitor::run()
{
	try
	{
		init();
	}
	catch (const init_exception &e)
	{
		std::cerr << "Init failed: " << e.what() << '.' << std::endl;
		cleanup();
		return -1;
	}

	while(m_run)
	{
		loop();
	}
	cleanup();
	return 0;
}

void do_callback(DISPMANX_UPDATE_HANDLE_T u, void * arg)
{
	std::cout << "------"
			<< std::endl;
}

void video_monitor::init()
{
	int result;
	bcm_host_init();
	m_display = vc_dispmanx_display_open(m_display_id);
	init_check(  m_display == 0 ,"can't open display");

	result = vc_dispmanx_display_get_info(m_display, &m_modeinfo);
	init_check( result != 0, "can't get display info");

	vc_dispmanx_vsync_callback( m_display, do_callback, NULL);

}

void video_monitor::loop()
{}

void video_monitor::cleanup()
{
	vc_dispmanx_display_close(m_display);
	bcm_host_deinit();
}
