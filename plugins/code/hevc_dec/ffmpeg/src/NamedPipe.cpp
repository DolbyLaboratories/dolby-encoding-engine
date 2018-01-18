#include <thread>
#include <chrono>
#include "NamedPipe.h"

#define PIPE_TIMEOUT 30000

struct thread_data
{
#ifdef WIN32
    HANDLE      pipe_handle;
#else
    int         pipe_handle;
#endif
    std::string pipe_name;
    char*       buffer;
    size_t      buffer_size;
    size_t      bytes_written_or_read;
    bool        is_working;
    int         ret_code;
};

static
void connect_thread_func(thread_data* data)
{
    data->is_working = true;
    data->ret_code = 0;

#ifdef WIN32  
    if (ConnectNamedPipe(data->pipe_handle, NULL) == NULL)
    {
        data->ret_code = GetLastError();
    }
#else
    if ((data->pipe_handle = open(data->pipe_name.c_str(), O_RDWR)) == -1)
    {
        data->ret_code = -1;
    }
#endif

    data->is_working = false;
}

static
void read_thread_func(thread_data* data)
{
    data->is_working = true; 
    data->ret_code = 0;

#ifdef WIN32
    DWORD bytes_read_dword;
    if (ReadFile(data->pipe_handle, data->buffer, (DWORD)data->buffer_size, &bytes_read_dword, NULL) == NULL)
    {
        data->ret_code = GetLastError();
    }
    data->bytes_written_or_read = (size_t)bytes_read_dword;
#else
    data->bytes_written_or_read = read(data->pipe_handle, data->buffer, data->buffer_size);
#endif

    
    data->is_working = false;
}

static
void write_thread_func(thread_data* data)
{
    data->is_working = true;
    data->ret_code = 0;

#ifdef WIN32
    DWORD bytes_written_dword = 0;
    if (WriteFile(data->pipe_handle, data->buffer, (DWORD)data->buffer_size, &bytes_written_dword, NULL) != NULL)
    {
        data->ret_code = GetLastError();
    }
    data->bytes_written_or_read = (size_t)bytes_written_dword;
#else
    data->bytes_written_or_read = write(data->pipe_handle, data->buffer, data->buffer_size);
#endif

    data->is_working = false;
}

NamedPipe::NamedPipe()
{
#ifdef WIN32
    handle = INVALID_HANDLE_VALUE;
#else
    handle = 0;
#endif
    name = "";
}

NamedPipe::~NamedPipe()
{
#ifdef WIN32    
    if (handle != INVALID_HANDLE_VALUE)
    {
        DisconnectNamedPipe(handle);
    }
#endif
    // no need to close pipes on linux, they are just temp files removed later by DEE
}

int NamedPipe::createPipe(std::string file)
{
    // this function assumes DEE will pass path to an existing temp file that will be removed later
#ifdef WIN32
    // on windows we only use the name of the temp file to create the named pipe
    name = std::string("\\\\.\\pipe\\") + file.substr(file.rfind("\\") + 1);

    handle = CreateNamedPipe(name.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_WAIT | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
        1,
        PIPE_BUFFER_SIZE,
        PIPE_BUFFER_SIZE,
        PIPE_TIMEOUT,
        NULL);

    if (handle == INVALID_HANDLE_VALUE)
    {
        return -1;
    }
#else
    // on linux we remove the original temp file and create a pipe in it's place
    name = file;
    remove(file.c_str());
    if (mkfifo(file.c_str(), S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH) != 0)
    {
        return -1;
    }
#endif

    return 0;
}

std::string NamedPipe::getPath()
{
    return name;
}

int NamedPipe::connectPipe()
{
    thread_data data;
    data.is_working = true;
    data.ret_code = 0;
    data.pipe_handle = handle;
    data.pipe_name = name;

    std::thread connect_thread = std::thread(connect_thread_func, &data);

    auto start = std::chrono::high_resolution_clock::now();
    while (data.is_working == true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = now - start;
        if (elapsed.count() > PIPE_TIMEOUT)
        {
#ifdef WIN32
            CancelIoEx(handle, NULL);
#else        
            pthread_t thread_id = connect_thread.native_handle();
            pthread_cancel(thread_id);
#endif
            data.ret_code = -1;
            break;
        }
    }

    if (connect_thread.joinable())
    {
        connect_thread.join();
    }
    
    handle = data.pipe_handle;

    return data.ret_code;
}

void NamedPipe::closePipe()
{
#ifdef WIN32
    CloseHandle(handle);
#else
    if (handle != 0)
    {
        close(handle);
    }
#endif
}

int NamedPipe::writeToPipe(char* data_to_write, size_t data_size, size_t* bytes_written)
{
    thread_data data;
    data.is_working = true;
    data.ret_code = 0;
    data.pipe_handle = handle;
    data.pipe_name = name;
    data.buffer = data_to_write;
    data.buffer_size = data_size;

    std::thread write_thread = std::thread(write_thread_func, &data);

    auto start = std::chrono::high_resolution_clock::now();
    while (data.is_working == true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = now - start;
        if (elapsed.count() > PIPE_TIMEOUT)
        {
#ifdef WIN32
            CancelIoEx(handle, NULL);
#else        
            pthread_t thread_id = write_thread.native_handle();
            pthread_cancel(thread_id);
#endif
            data.ret_code = -1;
            break;
        }
    }

    if (write_thread.joinable())
    {
        write_thread.join();
    }

    if (data.ret_code == 0)
    {
        *bytes_written = data.bytes_written_or_read;
    }
    else
    {
        *bytes_written = 0;
    }

    return data.ret_code;
}

int NamedPipe::readFromPipe(char* buffer, size_t buffer_size, size_t* bytes_read)
{
    thread_data data;
    data.is_working = true;
    data.ret_code = 0;
    data.pipe_handle = handle;
    data.pipe_name = name;
    data.buffer = buffer;
    data.buffer_size = buffer_size;

    std::thread read_thread = std::thread(read_thread_func, &data);

    auto start = std::chrono::high_resolution_clock::now();
    while (data.is_working == true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = now - start;
        if (elapsed.count() > PIPE_TIMEOUT)
        {
#ifdef WIN32
            CancelIoEx(handle, NULL);
#else        
            pthread_t thread_id = read_thread.native_handle();
            pthread_cancel(thread_id);
#endif
            data.ret_code = -1;
            break;
        }
    }

    if (read_thread.joinable())
    {
        read_thread.join();
    }

    if (data.ret_code == 0)
    {
        *bytes_read = data.bytes_written_or_read;
    }
    else
    {
        *bytes_read = 0;
    }

    return data.ret_code;
}
