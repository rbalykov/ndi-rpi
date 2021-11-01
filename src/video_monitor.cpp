/*
 * VideoMonitor.cpp
 *
 *  Created on: 28 окт. 2021 г.
 *      Author: rbalykov
 */

#include "video_monitor.h"
#include <vc_dispmanx.h>
#include <iostream>
#include <chrono>
#include <cassert>

static std::mutex m_render_lock;
static std::condition_variable m_render_trigger;
static bool m_render_flag;

#undef PRINT_VSYNC_TIME

video_monitor::video_monitor(bool* run)
{
	assert(run);

	m_run = run;
	m_display_id = 0;
	m_display = 0;
	m_update = 0;

	m_current_frame = NULL;
	m_quit = false;
	m_render_flag = false;
}

video_monitor::~video_monitor()
{
	// TODO Auto-generated destructor stub
	std::cout << "\nProgram quit." << std::endl;
}

void video_monitor::init_check(bool x, const char* message)
{
	if (x) throw init_exception(message);
}


int video_monitor::start()
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
	std::cout << "Init done." << std::endl;

	m_renderer = std::thread(render_thread, std::ref(*this)); // @suppress("Symbol is not resolved")
	m_renderer.detach();

	while(!m_quit)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	cleanup();
	return 0;
}

void video_monitor::render_thread (video_monitor &m)
{
	assert(m.m_display);

	DISPMANX_RESOURCE_HANDLE_T  pixels 	= DISPMANX_NO_HANDLE;
	DISPMANX_ELEMENT_HANDLE_T 	element = DISPMANX_NO_HANDLE;
	DISPMANX_UPDATE_HANDLE_T	update	= DISPMANX_NO_HANDLE;
	VC_RECT_T					frame_size, src_rect, dst_rect;
	uint32_t ptr_pixels;
	int result;
    VC_DISPMANX_ALPHA_T alpha =
    	{  DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 255,  0 };
	std::cout << "Render started." << std::endl;
	std::unique_lock<std::mutex> lock(m_render_lock);

	pixels = vc_dispmanx_resource_create( VC_IMAGE_ARGB8888
			,
			1920, 1080, &ptr_pixels);
	assert(pixels);

	update = vc_dispmanx_update_start( 10 );
	assert(update);

    vc_dispmanx_rect_set( &src_rect, 0, 0, 1920 << 16, 1080 << 16 );
    vc_dispmanx_rect_set( &dst_rect, 0, 0, m.m_modeinfo.width, m.m_modeinfo.height);

    element = vc_dispmanx_element_add(update, m.m_display, 100,
            &dst_rect, pixels, &src_rect, DISPMANX_PROTECTION_NONE,
            &alpha,
            NULL,             // clamp
			DISPMANX_NO_ROTATE );
    assert(element);

	while( *m.m_run )
	{
		m_render_trigger.wait(lock, []{return m_render_flag;});

#ifdef PRINT_VSYNC_TIME
		using namespace std::chrono;
		static auto t1 = high_resolution_clock::now();
		static auto t2 = high_resolution_clock::now();
		static uint32_t i = 0;
#endif
		ndi_receiver::ndi_video_t* vf = m.m_ndi.capture_video();
		if (vf && vf->p_data)
		{
			vc_dispmanx_rect_set( &frame_size, 0, 0, vf->xres, vf->yres);
			result = vc_dispmanx_resource_write_data(pixels, VC_IMAGE_XRGB8888,
		    		vf->line_stride_in_bytes, vf->p_data, &frame_size);
			assert(result == 0);
			m.m_ndi.free_video(vf);

		    vc_dispmanx_update_submit_sync( update );

		}
#ifdef PRINT_VSYNC_TIME
		if (i)
		{
			t2 = high_resolution_clock::now();
			nanoseconds d = (t2 - t1);
			std::cerr << d.count() << std::endl;
		}
		t1 = t2;
		i++;
#endif
		m_render_flag = false;
	}

	std::cout << "Render stopped." << std::endl;
	m.m_quit = true;
}


void video_monitor::vsync_callback(DISPMANX_UPDATE_HANDLE_T u, void* arg)
{
	(void) u;
	(void) arg;

    {
       std::lock_guard<std::mutex> lock(m_render_lock);
       m_render_flag = true;
       m_render_trigger.notify_one();
    }
}

void video_monitor::init()
{
	int result;

	m_current_frame = new video_frame_uyvy;

	bcm_host_init();
	m_display = vc_dispmanx_display_open(m_display_id);
	init_check(  m_display == DISPMANX_NO_HANDLE ,"can't open display");

	result = vc_dispmanx_display_get_info(m_display, &m_modeinfo);
	init_check( result != 0, "can't get display info");

	m_current_frame = new video_frame_uyvy(*this, 100);
	init_check( m_current_frame->good() != true, "can't create video frame");

	vc_dispmanx_vsync_callback(m_display, vsync_callback, this);
	(void) result;

	m_ndi.start();
}

void video_monitor::loop()
{}

void video_monitor::cleanup()
{
	m_ndi.stop();
	vc_dispmanx_vsync_callback(m_display, NULL, NULL);
	vc_dispmanx_display_close(m_display);
	bcm_host_deinit();
}
