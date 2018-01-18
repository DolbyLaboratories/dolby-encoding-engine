#include <string>
#include <vector>
#include <deque>
#include <list>
#include <thread>
#include <memory>
#include <mutex>
#include "hevc_dec_api.h"
#include "NamedPipe.h"

#define READ_BUFFER_SIZE 1024 * 1024

struct BufferBlob
{
    char*   data;
    size_t  data_size;

    BufferBlob(const void* data_to_copy, size_t size);
    ~BufferBlob();
};

typedef struct
{
    int                         output_bitdepth;
    int                         width;
    int                         height;
    hevc_dec_color_space_t      chroma_format;
    hevc_dec_frame_rate_t       frame_rate;
    hevc_dec_frame_rate_t       frame_rate_ext;
    std::string                 output_format;
    int                         transfer_characteristics;
    int                         matrix_coeffs;
    bool                        use_sps_frame_rate;
    std::string                 msg;

    std::deque<hevc_dec_picture_t> decoded_pictures;
    size_t                      decoded_pictures_to_discard;

    std::thread                 writer_thread;
    std::thread                 reader_thread;
    std::thread                 ffmpeg_thread;
    std::list<BufferBlob*>      in_buffer;
    std::list<BufferBlob*>      out_buffer;
    std::mutex                  in_buffer_mutex;
    std::mutex                  out_buffer_mutex;
    bool                        stop_writing_thread;
    bool                        stop_reading_thread;
    bool                        force_stop_writing_thread;
    bool                        force_stop_reading_thread;
    int                         ffmpeg_ret_code;

    std::vector<std::string>    temp_file;
    std::string                 ffmpeg_bin;
    std::string                 interpreter;
    std::string                 cmd_gen;

    NamedPipe                   in_pipe;
    NamedPipe                   out_pipe;

    // Benchamrk
    unsigned long long missed_calls;
    unsigned long long calls;

} hevc_dec_ffmpeg_data_t;

void remove_pictures(hevc_dec_ffmpeg_data_t* data);

hevc_dec_frame_rate_t string_to_fr(const std::string& str);

bool bin_exists(const std::string& bin, const std::string& arg);

void run_cmd_thread_func(std::string cmd, hevc_dec_ffmpeg_data_t* decoding_data);

void writer_thread_func(hevc_dec_ffmpeg_data_t* decoding_data);

void reader_thread_func(hevc_dec_ffmpeg_data_t* decoding_data);

int extract_bytes(std::list<BufferBlob*>& blob_list, size_t byte_num, char* buffer);

size_t get_buf_size(std::list<BufferBlob*>& blob_list);

int extract_pictures_from_buffer(hevc_dec_ffmpeg_data_t* data);

bool write_cfg_file(hevc_dec_ffmpeg_data_t* data, const std::string& file);

std::string run_cmd_get_output(std::string cmd);