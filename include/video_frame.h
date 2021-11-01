/*
 * video_frame.h
 *
 *  Created on: 28 окт. 2021 г.
 *      Author: rbalykov
 */

#ifndef SRC_VIDEO_FRAME_H_
#define SRC_VIDEO_FRAME_H_

#include <stdint.h>
#include <vc_dispmanx.h>
#include "ndi_receiver.h"

class video_monitor;
class video_frame
{
public:
	video_frame();
	virtual ~video_frame();

	uint32_t w() { return m_w; };
	uint32_t h() { return m_h; };
	uint32_t macro_w() { return m_macro_w; };
	uint32_t macro_h() { return m_macro_h; };
	bool good() { return m_good; };

	VC_DISPMANX_ALPHA_T* alpha() {return &m_alpha_desc; }

protected:
	uint32_t m_w, m_h, m_macro_w, m_macro_h;
	bool m_good;

	VC_RECT_T m_src_rect;
	VC_RECT_T m_dst_rect;
	VC_DISPMANX_ALPHA_T			m_alpha_desc;

//	DISPMANX_ELEMENT_HANDLE_T   element;
//	void                       *image;

};

class video_frame_uyvy: public video_frame
{
public:
	video_frame_uyvy	();
	video_frame_uyvy	(video_monitor& m, uint32_t layer);
	video_frame_uyvy	(const char* file, uint32_t w, uint32_t h);

	void capture_file	(const char * file, uint32_t w, uint32_t h);
	void capture_ndi	(ndi_receiver::ndi_video_t* vf);

	DISPMANX_RESOURCE_HANDLE_T pixels()  { return m_pixels; };
	DISPMANX_ELEMENT_HANDLE_T  element() { return m_element; };
	uint32_t layer() { return m_layer; };

private:
	DISPMANX_RESOURCE_HANDLE_T  m_pixels;
	DISPMANX_ELEMENT_HANDLE_T 	m_element;

	uint32_t	m_layer;
	uint32_t 	m_ptr_pixels;
};

#endif /* SRC_VIDEO_FRAME_H_ */
