/*
 * ndi_receiver.h
 *
 *  Created on: 29 окт. 2021 г.
 *      Author: rbalykov
 */

#ifndef SRC_NDI_RECEIVER_H_
#define SRC_NDI_RECEIVER_H_

#include <thread>
#include <mutex>
#include "Processing.NDI.Advanced.h"

class ndi_receiver
{
public:
	ndi_receiver();
	virtual ~ndi_receiver();

	void start();
	void stop();

	typedef NDIlib_video_frame_v2_t 	ndi_video_t;
	typedef NDIlib_audio_frame_v3_t		ndi_audio_t;
	typedef NDIlib_find_instance_t  	ndi_find_t;
	typedef NDIlib_recv_create_v3_t 	ndi_recv_desc_t;
	typedef NDIlib_recv_instance_t 		ndi_recv_t;
	typedef NDIlib_framesync_instance_t	ndi_fsync_t;
	typedef NDIlib_avsync_instance_t	ndi_avsync_t;
	typedef NDIlib_source_t				ndi_src_t;

	ndi_video_t* capture_video();
	ndi_audio_t* capture_audio();

	void free_video(ndi_video_t* vf);
	void free_audio(ndi_audio_t* af);

private:
	void poll();
	void pend_video(ndi_video_t* vf);
	void pend_audio(ndi_audio_t* af);

	void discover_any_source();
	void disconnect();
	void cleanup();

	static void poll_thread (ndi_receiver &ndi);
	std::thread poller;

	ndi_find_t		m_pfinder;
	ndi_recv_desc_t m_rx_desc;
	ndi_recv_t 		m_precv;
	ndi_avsync_t 	m_pavsync;

	ndi_video_t* m_pending_video;
	ndi_audio_t* m_pending_audio;

	uint32_t m_v_received, m_v_captured, m_v_dropped;
	uint32_t m_a_received, m_a_captured, m_a_dropped;
	std::chrono::high_resolution_clock::time_point m_last_data;

	bool m_run;
	bool m_quit;
	bool m_connected;

	std::mutex m_discovery_lock;
	std::mutex m_video_lock;
	std::mutex m_audio_lock;

	static uint32_t timeout_ms;

};

#endif /* SRC_NDI_RECEIVER_H_ */
