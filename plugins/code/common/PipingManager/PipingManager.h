#include <string>

struct PipingManagerData;

typedef enum
{
    PIPE_MGR_OK = 0,
    PIPE_MGR_TIMEOUT,
    PIPE_MGR_CONNECT_ERROR,
    PIPE_MGR_READ_ERROR,
    PIPE_MGR_WRITE_ERROR,
    PIPE_MGR_CREATE_ERROR,
    PIPE_MGR_PIPE_NOT_FOUND,
    PIPE_MGR_PIPE_CLOSED,
    PIPE_MGR_UNKNOWN_ERROR,
    PIPE_MGR_BUFFER_FULL
} piping_status_t;

typedef enum
{
    INPUT_PIPE,
    OUTPUT_PIPE,
    DUPLEX_PIPE
} pipe_type_t;

class PipingManager
{
public:
    PipingManager();
    ~PipingManager();
    void setTimeout(int timeout);
    void setMaxbuf(size_t maxbuf);
    void setGlobalTimeout(bool global_timeout);
    int createNamedPipe(std::string path, pipe_type_t type);
    piping_status_t destroyNamedPipe(int pipe_id);
    piping_status_t closePipe(int pipe_id);
    piping_status_t getPipePath(int pipe_id, std::string& path);
    piping_status_t writeToPipe(int pipe_id, void* buffer, size_t data_size, size_t& bytes_written);
    piping_status_t readFromPipe(int pipe_id, void* buffer, size_t buffer_size, size_t& bytes_read);
    piping_status_t pipeDataReady(int pipe_id, size_t& bytes_ready);
    piping_status_t getPipeStatus(int pipe_id);
private:
    PipingManagerData* mData;
};
