#include <string>
#include <atomic>

typedef enum
{
    SYSCALL_STATUS_OK = 0,
    SYSCALL_STATUS_TIMEOUT,
    SYSCALL_STATUS_CALL_ERROR,
    SYSCALL_STATUS_RUNTIME_ERROR,
    SYSCALL_STATUS_KILLED
} system_call_status_t;

int systemWithTimeout(std::string command, int& return_code, int timeout);

int systemWithKillswitch(std::string command, int& return_code, std::atomic_bool& killswitch);

int systemWithStdout(std::string cmd, std::string& output);