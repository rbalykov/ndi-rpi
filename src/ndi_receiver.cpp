/*
 * ndi_receiver.cpp
 *
 *  Created on: 29 окт. 2021 г.
 *      Author: rbalykov
 */

#include "ndi_receiver.h"
#include <cstring>
#include <iostream>
#include <cassert>

ndi_receiver::ndi_receiver()
{
	m_pfinder 		= NULL;
	m_precv 		= NULL;
	m_pavsync 		= NULL;
	m_run 		= false;
	m_connected = false;

	m_pending_video = NULL;
	m_pending_audio = NULL;

	m_rx_desc.color_format 		= NDIlib_recv_color_format_fastest;
	m_rx_desc.bandwidth 		= NDIlib_recv_bandwidth_highest;
	m_rx_desc.p_ndi_recv_name 	= "RPI monitor";

	m_v_received = m_v_captured = m_v_dropped = 0;
	m_a_received = m_a_captured = m_a_dropped = 0;
}

ndi_receiver::~ndi_receiver()
{
	if (m_pending_video)
	{
		free_video(m_pending_video);
		m_v_dropped++;
	}
	if (m_pending_audio)
	{
		free_audio(m_pending_audio);
		m_a_dropped++;
	}
	if(m_pavsync) 	NDIlib_avsync_destroy(m_pavsync);
	if(m_precv)		NDIlib_recv_destroy(m_precv);

	std::cout << "Video frames received: " << m_v_received
			  << ", captured: " << m_v_captured
			  << ", dropped: "  << m_v_dropped << std::endl;

	std::cout << "Audio frames received: " << m_a_received
			  << ", captured: " << m_a_captured
			  << ", dropped: "  << m_a_dropped << std::endl;

}

void ndi_receiver::poll()
{
	NDIlib_frame_type_e result;
	ndi_video_t* vf = new ndi_video_t;
	ndi_audio_t* af = new ndi_audio_t;

	if ((!vf) || (!af))
	{
		std::cerr << "Out of memory. NDI polling stopped." << std::endl;
		if (vf) delete vf;
		if (af) delete af;
		stop();
		return;
	}

	result = NDIlib_recv_capture_v3(m_precv, vf, af, NULL, timeout_ms);
	switch(result)
	{
	case NDIlib_frame_type_video:
		delete af;
		pend_video(vf);
		break;

	case NDIlib_frame_type_audio:
		delete vf;
		pend_audio(af);
		break;

	case NDIlib_frame_type_none:
	case NDIlib_frame_type_metadata:
	case NDIlib_frame_type_error:
	case NDIlib_frame_type_status_change:
	default:
		delete vf;
		delete af;
		break;
	}
}

void ndi_receiver::poll_thread (ndi_receiver &ndi)
{
	std::cout << "NDI polling started." << std::endl;
	if (!NDIlib_initialize())
	{
		std::cerr << "Failed to initialize NDIlib." << std:: endl;
		ndi.m_run = false;
	}
	while(ndi.m_run)
	{
		if (!ndi.m_connected)
		{
			ndi.discover_any_source();
		}
		else
		{
			ndi.poll();
		}
	}
	NDIlib_destroy();
	std::cout << "NDI polling stopped." << std::endl;
}

void ndi_receiver::discover_any_source()
{
	uint32_t src_count;
	const ndi_src_t* m_psources;
	m_discovery_lock.lock();

	if (m_connected) goto quit;

	if (!m_pfinder)
	{
		m_pfinder = NDIlib_find_create_v2();
		if (!m_pfinder)
		{
			std::cerr << "Failed to create finder." << std:: endl;
			m_run = false;
			goto quit;
		}
	}
	m_psources = NDIlib_find_get_current_sources(m_pfinder, &src_count);
	if (!src_count)
		goto quit;

	m_precv = NDIlib_recv_create_v3(&m_rx_desc);
	if (!m_precv)
	{
		std::cerr << "Failed to create receiver." << std:: endl;
		m_run = false;
		goto quit;
	}
	std::cout << "NDI source found: " << m_psources[0].p_ndi_name << std::endl;
	NDIlib_recv_connect(m_precv, m_psources + 0);
	NDIlib_find_destroy(m_pfinder);
	m_pfinder = NULL;
	m_connected = true;

	quit:
	m_discovery_lock.unlock();
}

void ndi_receiver::start()
{
	m_discovery_lock.lock();
	if (m_run) goto quit;

	m_run = true;
	poller = std::thread(poll_thread, std::ref(*this)); // @suppress("Symbol is not resolved")
	poller.detach();

	quit:
	m_discovery_lock.unlock();
}

void ndi_receiver::stop()
{
	m_discovery_lock.lock();
	m_run = false;
	m_connected = false;
	m_discovery_lock.unlock();

	if (poller.joinable())
		poller.join();
}

ndi_receiver::ndi_video_t* ndi_receiver::capture_video()
{
	ndi_video_t* result;

	m_video_lock.lock();
	if (m_pending_video == NULL)
	{
		result = NULL;
	}
	else
	{
		result = m_pending_video;
		m_pending_video = NULL;
		m_v_captured++;
	}
	m_video_lock.unlock();

	return result;
}

ndi_receiver::ndi_audio_t* ndi_receiver::capture_audio()
{
	ndi_audio_t* result;

	m_audio_lock.lock();
	if (m_pending_audio == NULL)
	{
		result = NULL;
	}
	else
	{
		result = m_pending_audio;
		m_pending_audio = NULL;
		m_a_captured++;
	}
	m_audio_lock.unlock();

	return result;
}

void ndi_receiver::pend_video(ndi_receiver::ndi_video_t* vf)
{
	assert(vf);

	m_video_lock.lock();
	m_v_received++;
	if (m_pending_video == NULL)
	{
		m_pending_video = vf;
	}
	else
	{
		NDIlib_recv_free_video_v2(m_precv, m_pending_video);
		delete m_pending_video;
		m_pending_video = vf;
		m_v_dropped++;
	}
	m_video_lock.unlock();
}

void ndi_receiver::pend_audio(ndi_receiver::ndi_audio_t* af)
{
	assert(af);

	m_audio_lock.lock();
	m_a_received++;
	if (m_pending_audio == NULL)
	{
		m_pending_audio = af;
	}
	else
	{
		NDIlib_recv_free_audio_v3(m_precv, m_pending_audio);
		delete m_pending_audio;
		m_pending_audio = af;
		m_a_dropped++;
	}
	m_audio_lock.unlock();
}

void ndi_receiver::free_video(ndi_receiver::ndi_video_t* vf)
{
	assert(m_precv);
	assert(vf);

	m_video_lock.lock();
	NDIlib_recv_free_video_v2(m_precv, vf);
	delete vf;
	m_video_lock.unlock();
}

void ndi_receiver::free_audio(ndi_receiver::ndi_audio_t* af)
{
	assert(m_precv);
	assert(af);

	m_audio_lock.lock();
	NDIlib_recv_free_audio_v3(m_precv, af);
	delete af;
	m_audio_lock.unlock();
}



