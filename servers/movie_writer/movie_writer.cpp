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


void MovieWriter::write_frame(const Ref<Image> &p_image) {

	// UPDATE WINDOW TITLE
	const int movie_time_seconds = Engine::get_singleton()->get_frames_drawn() / fps;
	const String movie_time = vformat("%s:%s:%s",
			String::num(movie_time_seconds / 3600).pad_zeros(2),
			String::num((movie_time_seconds % 3600) / 60).pad_zeros(2),
			String::num(movie_time_seconds % 60).pad_zeros(2));

	OS::get_singleton()->set_window_title(vformat("MovieWriter: Frame %d (time: %s) - %s (DEBUG)", Engine::get_singleton()->get_frames_drawn(), movie_time, project_name));
    
	// SAVE IMAGE DATA IN A STRUCT TO SEND TO THREAD FUCNTION
	FrameData* fd = new FrameData();
	fd->image = p_image;
	fd->image_path = base_dir.get_basename() + "/" + base_file.get_basename() + String::num_int64(frame_count).pad_zeros(8) + "." + base_file.get_extension();
	fd->jpeg_quality = jpeg_quality;

	// WAIT CURRENT TURN THREAD FINISH AND SEND THE NEW FRAME TO SAVE
	threads[thread_turn].wait_to_finish();
	threads[thread_turn].start(_thread_func, fd);

	// LOOP THE THREAD TURN AROUN THREAD COUNT
	// MOST LIKELY THAT WHEN THE TURN ROLLBACK THE THREAD IS ALREADY FINISHED
	thread_turn++;
	if(thread_turn == thread_count)
		thread_turn = 0;

	// INCREASE FRAME COUNT
	frame_count++;	
}

void MovieWriter::_thread_func(void *p_userdata) {
	// SAVE THE FRAME
	FrameData *fd = (FrameData *)p_userdata;

	fd->image->flip_y();
	fd->image->save_jpg(fd->image_path, fd->jpeg_quality);

	delete fd;
}


void MovieWriter::write_end() {
	// WAIT ALL THREADS FINISH AND FREE THE MEMORY
	for(int i = 0; i < thread_count; i++) {
		threads[i].wait_to_finish();
	}
  	delete [] threads;


	// PRINT HOW MUCH TIME WAS SPEND ON RENDER
	const int movie_time_seconds = Engine::get_singleton()->get_frames_drawn() / fps;
	const String movie_time = vformat("%s:%s:%s",
			String::num(movie_time_seconds / 3600).pad_zeros(2),
			String::num((movie_time_seconds % 3600) / 60).pad_zeros(2),
			String::num(movie_time_seconds % 60).pad_zeros(2));

	const int real_time_seconds = Time::get_singleton()->get_ticks_msec() / 1000;
	const String real_time = vformat("%s:%s:%s",
			String::num(real_time_seconds / 3600).pad_zeros(2),
			String::num((real_time_seconds % 3600) / 60).pad_zeros(2),
			String::num(real_time_seconds % 60).pad_zeros(2));

	print_line(vformat("%d frames at %d FPS (movie length: %s), recorded in %s (%d%% of real-time speed).", Engine::get_singleton()->get_frames_drawn(), fps, movie_time, real_time, (float(movie_time_seconds) / real_time_seconds) * 100));
	print_line("----------------");
}



MovieWriter::MovieWriter() {
	// EDITOR SETTINGS
	
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

	if(enabled)
		print_line(vformat("Movie Maker mode enabled, recording movie using %d threads...", thread_count));
}