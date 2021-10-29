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

class video_frame
{
public:
	video_frame();
	virtual ~video_frame();

protected:
	VC_RECT_T m_output;
	uint32_t m_w, m_h, m_macro_w, m_macro_h;
	bool m_good;

	VC_DISPMANX_ALPHA_T			m_alpha_desc;

//	DISPMANX_ELEMENT_HANDLE_T   element;
//	void                       *image;

};

class video_frame_uyvy: public video_frame
{
public:
	video_frame_uyvy();
	video_frame_uyvy(const char* file, uint32_t w, uint32_t h);
	void capture_file(const char * file, uint32_t w, uint32_t h);
private:
	DISPMANX_RESOURCE_HANDLE_T  m_pixels;
	uint32_t 					m_ptr_pixels;
//	uint32_t m_ptr_alpha;

};

#endif /* SRC_VIDEO_FRAME_H_ */
