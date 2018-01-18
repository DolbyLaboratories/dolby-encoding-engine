#ifdef WIN32

#include <windows.h> 
#define PIPE_BUFFER_SIZE 1024 * 1024

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#endif

#include <string>

class NamedPipe
{
#ifdef WIN32
    HANDLE      handle;
#else
    int         handle;
#endif
    std::string name;

public:
    NamedPipe();
    ~NamedPipe();
    int createPipe(std::string name);
    int connectPipe();
    void closePipe();
    int writeToPipe(char* data_to_write, size_t data_size, size_t* bytes_written);
    int readFromPipe(char* buffer, size_t buffer_size, size_t* bytes_read);
    std::string getPath();
};
