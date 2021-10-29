/*
 * video_frame.cpp
 *
 *  Created on: 28 окт. 2021 г.
 *      Author: rbalykov
 */

#include "video_frame.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <cstddef>

#include <interface/vctypes/vc_image_types.h>
#include <vc_dispmanx.h>

video_frame::video_frame()
{
	// TODO Auto-generated constructor stub
	m_w = m_h = m_macro_w = m_macro_h = 1;
	m_good = false;

	m_output.x = m_output.y = 0;
	m_output.width = m_output.height = 1;

	m_alpha_desc.flags = DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS;
	m_alpha_desc.opacity = ~0;
	m_alpha_desc.mask = 0;
}

video_frame::~video_frame()
{
	// TODO Auto-generated destructor stub
}

video_frame_uyvy::video_frame_uyvy()
	:video_frame()
{
	m_pixels = vc_dispmanx_resource_create( VC_IMAGE_YUV422UYVY,
			m_w, m_h, &m_ptr_pixels);
	m_good = (m_pixels != 0);
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
	assert(m_pixels);

	m_h = h; m_w = w;
	m_macro_w = (m_w+1)/2;
	m_macro_h = m_h;

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

    vc_dispmanx_rect_set( &m_output, 0, 0, m_w, m_h);
    result = vc_dispmanx_resource_write_data(m_pixels, VC_IMAGE_YUV422UYVY,
    		m_macro_w * 4, buffer.data(), &m_output);

    m_good = true;
}


