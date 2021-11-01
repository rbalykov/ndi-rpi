/*
 * video_frame.cpp
 *
 *  Created on: 28 окт. 2021 г.
 *      Author: rbalykov
 */

#include "video_frame.h"
#include "video_monitor.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <cstddef>

#include <interface/vctypes/vc_image_types.h>
#include <vc_dispmanx.h>

video_frame::video_frame()
{
	m_w = m_h = m_macro_w = m_macro_h = 1;

	vc_dispmanx_rect_set(&m_src_rect, 0, 0, m_w << 16, m_h << 16);
	vc_dispmanx_rect_set(&m_dst_rect, 0, 0, m_w, m_h);

	m_alpha_desc.flags = DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS;
	m_alpha_desc.opacity = ~0;
	m_alpha_desc.mask = 0;

	m_good = false;
}

video_frame::~video_frame()
{
	// TODO Auto-generated destructor stub
}

video_frame_uyvy::video_frame_uyvy()
	:video_frame()
{
	m_pixels 		= DISPMANX_NO_HANDLE;
	m_ptr_pixels 	= DISPMANX_NO_HANDLE;
	m_layer 		= DISPMANX_NO_HANDLE;
	m_element 		= DISPMANX_NO_HANDLE;
}

video_frame_uyvy::video_frame_uyvy(video_monitor& m, uint32_t layer)
	:video_frame()
{
	m_w = m.mode()->width;
	m_h = m.mode()->height;
	m_macro_w = (m_w + 1)/2;
	m_macro_h = m_h;

	vc_dispmanx_rect_set(&m_src_rect, 0, 0, m_w << 16, m_h << 16);
	vc_dispmanx_rect_set(&m_dst_rect, 0, 0, m_w, m_h);

	m_pixels = vc_dispmanx_resource_create( VC_IMAGE_YUV422UYVY,
			m_w, m_h, &m_ptr_pixels);
	m_element = vc_dispmanx_element_add(m.update_handle(), m.display(),
            m_layer, &m_dst_rect, m_pixels,
            &m_src_rect, DISPMANX_PROTECTION_NONE,
			&m_alpha_desc,
            NULL, DISPMANX_NO_ROTATE );
	m_layer = layer;
	m_good =  ((m_pixels  != DISPMANX_NO_HANDLE)
			&& (m_element != DISPMANX_NO_HANDLE));
}



video_frame_uyvy::video_frame_uyvy(const char* file, uint32_t w, uint32_t h)
	:video_frame()
{
	if (m_good)
		capture_file(file, w, h);
}


void video_frame_uyvy::capture_file(const char * file, uint32_t w, uint32_t h)
{
	int result;

	m_h = h; m_w = w;
	m_macro_w = (m_w+1)/2;
	m_macro_h = m_h;

	if (m_pixels == DISPMANX_NO_HANDLE)
	{
		m_pixels = vc_dispmanx_resource_create( VC_IMAGE_YUV422UYVY,
				m_w, m_h, &m_ptr_pixels);
	}
	assert(m_pixels);

    std::ifstream input( file, std::ios::binary );
    if (!input.good())
    {
    	std::cerr << "can't open file: " << file << std::endl;
    	m_good = false;
    	return;
    }

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
    if (buffer.size() != m_macro_w * m_h * 4)
    {
    	std::cerr << "wrong stub data for file " << file
    			<<", expected "<<  m_macro_w * m_h * 4
    			<< ", got " << buffer.size() << std::endl; // @suppress("Invalid overload")
    	m_good = false;
    	return;
    }
    input.close();

    vc_dispmanx_rect_set( &m_dst_rect, 0, 0, m_w, m_h);
    result = vc_dispmanx_resource_write_data(m_pixels, VC_IMAGE_YUV422UYVY,
    		m_macro_w * 4, buffer.data(), &m_dst_rect);

    m_good = (result == 0);
}

void video_frame_uyvy::capture_ndi(ndi_receiver::ndi_video_t* vf)
{
	assert(vf);
	assert(vf->p_data);
//	assert(m_pixels);

	int result;
	VC_RECT_T frame_size;

	m_w = vf->xres;
	m_h = vf->yres;
	m_macro_w = vf->line_stride_in_bytes/4;
	m_macro_h = vf->yres;

    vc_dispmanx_rect_set( &frame_size, 0, 0, m_w, m_h);
    result = vc_dispmanx_resource_write_data(m_pixels, VC_IMAGE_YUV422UYVY,
    		vf->line_stride_in_bytes, vf->p_data, &frame_size);


    m_good = (result == 0);
}



