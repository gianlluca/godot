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

#include "movie_writer.h"


String MovieWriter::zeros_str(uint32_t p_index) {
	char zeros[MAX_TRAILING_ZEROS + 1];
	for (uint32_t i = 0; i < MAX_TRAILING_ZEROS; i++) {
		uint32_t idx = MAX_TRAILING_ZEROS - i - 1;
		uint32_t digit = (p_index / uint32_t(Math::pow(double(10), double(idx)))) % 10;
		zeros[i] = '0' + digit;
	}
	zeros[MAX_TRAILING_ZEROS] = 0;
	return zeros;
}

int MovieWriter::get_target_fps() {
    return fps;
}

void MovieWriter::set_is_enabled(bool p_enabled) {
    enabled = p_enabled;
}

bool MovieWriter::is_enabled() {
    return enabled;
}

void MovieWriter::_bind_methods() {
	// ClassDB::bind_static_method("MovieWriter");

	// GDVIRTUAL_BIND(write_frame, "frame_image")
	// GDVIRTUAL_BIND(write_end)

	// GLOBAL_DEF(PropertyInfo(Variant::BOOL, "editor/movie_writer/enabled"), false);
	// ClassDB::bind_method(D_METHOD("editor", "movie_writer", "enabled"), false);
    
	// // GLOBAL_DEF_BASIC("editor/movie_writer/movie_dir", "");
	// ClassDB::bind_method(D_METHOD("editor", "movie_writer", "movie_dir"), "");

	// // GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "editor/movie_writer/jpeg_quality", PROPERTY_HINT_RANGE, "0.01,1.0,0.01"), 0.75);
	// ClassDB::bind_method(D_METHOD("editor", "movie_writer", "jpeg_quality"), 0.75);

	// // GLOBAL_DEF(PropertyInfo(Variant::INT, "editor/movie_writer/thread_count", PROPERTY_HINT_RANGE, "2,128,1,suffix:Threads"), 8);
	// ClassDB::bind_method(D_METHOD("editor", "movie_writer", "thread_count"), 8);
}

// OLD
// void MovieWriter::write_frame(const Ref<Image> &p_image) {

// 	// UPDATE WINDOW TITLE
// 	const int movie_time_seconds = Engine::get_singleton()->get_frames_drawn() / fps;
// 	const String movie_time = vformat("%s:%s:%s",
// 			String::num(movie_time_seconds / 3600).pad_zeros(2),
// 			String::num((movie_time_seconds % 3600) / 60).pad_zeros(2),
// 			String::num(movie_time_seconds % 60).pad_zeros(2));

// 	OS::get_singleton()->set_window_title(vformat("MovieWriter: Frame %d (time: %s) - %s (DEBUG)", Engine::get_singleton()->get_frames_drawn(), movie_time, project_name));
    

// 	FrameData* fd = new FrameData();
	
// 	// RID main_vp_rid = RenderingServer::get_singleton()->viewport_find_from_screen_attachment(DisplayServer::MAIN_WINDOW_ID);
// 	// RID main_vp_texture = RenderingServer::get_singleton()->viewport_get_texture(main_vp_rid);
// 	// Ref<Image> vp_tex = RenderingServer::get_singleton()->texture_2d_get(main_vp_texture);

// 	fd->image = p_image;
// 	fd->image_path = base_path + zeros_str(frame_count) + ".jpg";
// 	fd->jpeg_quality = jpeg_quality;

// 	threads[thread_turn].wait_to_finish();
// 	threads[thread_turn].start(_thread_func, fd);

// 	thread_turn++;

// 	if(thread_turn == thread_count)
// 		thread_turn = 0;

// 	frame_count++;    
// }


void MovieWriter::write_frame(const Ref<Image> &p_image) {

	// UPDATE WINDOW TITLE
	const int movie_time_seconds = Engine::get_singleton()->get_frames_drawn() / fps;
	const String movie_time = vformat("%s:%s:%s",
			String::num(movie_time_seconds / 3600).pad_zeros(2),
			String::num((movie_time_seconds % 3600) / 60).pad_zeros(2),
			String::num(movie_time_seconds % 60).pad_zeros(2));

	OS::get_singleton()->set_window_title(vformat("MovieWriter: Frame %d (time: %s) - %s (DEBUG)", Engine::get_singleton()->get_frames_drawn(), movie_time, project_name));
    

	FrameData* fd = new FrameData();
	fd->image = p_image;
	fd->image_path = base_dir.get_basename() + "/" + base_file.get_basename() + zeros_str(frame_count) + "." + base_file.get_extension();
	fd->jpeg_quality = jpeg_quality;

	threads[thread_turn].wait_to_finish();
	threads[thread_turn].start(_thread_func, fd);

	thread_turn++;

	if(thread_turn == thread_count)
		thread_turn = 0;

	frame_count++;
}

void MovieWriter::_thread_func(void *p_userdata) {
	FrameData *fd = (FrameData *)p_userdata;

	fd->image->flip_y();
	fd->image->save_jpg(fd->image_path, fd->jpeg_quality);

	delete fd;
}


void MovieWriter::write_end() {
	for(int i = 0; i < thread_count; i++) {
		threads[i].wait_to_finish();
	}

  	delete [] threads;
}



MovieWriter::MovieWriter() {
	project_name = GLOBAL_GET("application/config/name");

    enabled = GLOBAL_DEF("movie_writer/enable", false);

    base_dir = String(GLOBAL_DEF("movie_writer/output_dir", String())).strip_edges();
	ProjectSettings::get_singleton()->set_custom_property_info("movie_writer/output_dir", PropertyInfo(Variant::STRING, "movie_writer/output_dir", PROPERTY_HINT_GLOBAL_DIR, ""));

    base_file = String(GLOBAL_DEF("movie_writer/output_file", "out.jpg")).strip_edges();
	ProjectSettings::get_singleton()->set_custom_property_info("movie_writer/output_file", PropertyInfo(Variant::STRING, "movie_writer/output_file", PROPERTY_HINT_NONE, ""));

	fps = GLOBAL_DEF("movie_writer/movie_fps", 60);
	ProjectSettings::get_singleton()->set_custom_property_info("movie_writer/movie_fps", PropertyInfo(Variant::INT, "movie_writer/movie_fps", PROPERTY_HINT_RANGE, "1,240,1"));

	jpeg_quality = GLOBAL_DEF("movie_writer/jpeg_quality", 0.75);
	ProjectSettings::get_singleton()->set_custom_property_info("movie_writer/jpeg_quality", PropertyInfo(Variant::REAL, "movie_writer/jpeg_quality", PROPERTY_HINT_RANGE, "0.01,1.0,0.01"));

	thread_count = (int) GLOBAL_DEF("movie_writer/thread_count", 8);
	ProjectSettings::get_singleton()->set_custom_property_info("movie_writer/thread_count", PropertyInfo(Variant::INT, "movie_writer/thread_count", PROPERTY_HINT_RANGE, "1,128,1"));
	threads = new Thread[thread_count];

	print_line(vformat("Movie Maker mode enabled, recording movie using %d threads...", thread_count));
}