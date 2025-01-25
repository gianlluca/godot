/**************************************************************************/
/*  movie_writer.h                                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef MOVIE_WRITER_H
#define MOVIE_WRITER_H

#include "core/os/thread.h"
#include "core/project_settings.h"
#include "core/os/file_access.h"
#include "core/os/dir_access.h"
#include "core/os/time.h"
#include "servers/visual_server.h"
#include "core/os/os.h"


class MovieWriter : public Object {
	GDCLASS(MovieWriter, Object);

    enum {
		MAX_TRAILING_ZEROS = 8 // more than 10 days at 60fps, no hard drive can put up with this anyway :)
	};

	String project_name;

    bool enabled = false;
	String base_dir;
	String base_file;
	float jpeg_quality = 0.75;
	int fps = 0;

    struct FrameData {
		Ref<Image> image;
		String image_path;
		float jpeg_quality;
	};
    
	int thread_count = 8;
	int thread_turn = 0;
	Thread *threads;
	
	uint32_t frame_count = 0;

	String zeros_str(uint32_t p_index);
	static void _thread_func(void *p_userdata);

protected:
	static void _bind_methods();

public:
    int get_target_fps();

    bool is_enabled();
    void set_is_enabled(bool p_enabled);
	
	void write_frame(const Ref<Image> &p_image);
	void write_end();
    
	MovieWriter();
};

#endif // MOVIE_WRITER_H