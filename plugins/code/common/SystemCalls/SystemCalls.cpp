/*
* BSD 3-Clause License
*
* Copyright (c) 2018-2019, Dolby Laboratories
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice, this
*   list of conditions and the following disclaimer.
*
* * Redistributions in binary form must reproduce the above copyright notice,
*   this list of conditions and the following disclaimer in the documentation
*   and/or other materials provided with the distribution.
*
* * Neither the name of the copyright holder nor the names of its
*   contributors may be used to endorse or promote products derived from
*   this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <SystemCalls.h>
#include <vector>
#include <sstream>
#include <memory>
#include <iterator>

#ifdef WIN32

#include <Windows.h>
#include <WinBase.h>

struct ProcessData
{
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;
};

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <list>

struct ProcessData
{
    pid_t pid;
};

#endif

// check threads every 100 miliseconds
#define LOOP_WAIT 100
#define TEMP_BUF 1024

static
inline
std::vector<std::string> splitQuotedString(const std::string &s, char delim)
{
    std::stringstream item;
    std::vector<std::string> elems;

    for (unsigned int i = 0; i < s.length(); i++)
    {
        char c = s[i];
        if (c == delim)
        {
            if (item.str() != "")
            {
                elems.push_back(item.str());
                item.str("");
            }
        }
        else if (c == '\"')
        {
            do
            {
                item << s[i];
                i++;
            }
            while (s[i] != '\"');
            
            item << s[i];
            elems.push_back(item.str());
            item.str("");
        }
        else
        {
            item << c;
        }
    }
    
    if (item.str() != "")
        elems.push_back(item.str());

    return elems;
}

// remove leading and trailing whitespaces and quotes
static
inline 
std::string& trim(std::string& s, const char* t = "\"\n\t ")
{
	s.erase(s.find_last_not_of(t) + 1);
    s.erase(0, s.find_first_not_of(t));
    return s;
}

static
system_call_status_t startProcess(std::string command, std::string logfile, ProcessData& processData)
{
#ifdef WIN32

    std::string command_line = trim(command, "\n");

    ZeroMemory(&processData.startupInfo, sizeof(processData.startupInfo));
    processData.startupInfo.cb = sizeof(processData.startupInfo);

    ZeroMemory(&processData.processInfo, sizeof(processData.processInfo));

    HANDLE log_handle = NULL;
    if (!logfile.empty())
    {
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;

        log_handle = CreateFile(logfile.c_str(),
            FILE_APPEND_DATA,
            FILE_SHARE_WRITE | FILE_SHARE_READ,
            &sa,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        processData.startupInfo.dwFlags |= STARTF_USESTDHANDLES;
        processData.startupInfo.hStdInput = NULL;
        processData.startupInfo.hStdOutput = log_handle;
        processData.startupInfo.hStdError = log_handle;
    }

    BOOL status = CreateProcess(NULL, (LPSTR)(command_line.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &processData.startupInfo, &processData.processInfo);
    // closing unneeded handles
    CloseHandle(processData.processInfo.hThread);
    CloseHandle(processData.startupInfo.hStdInput);
    CloseHandle(log_handle);
    if (status == 0)
    {
        return SYSCALL_STATUS_CALL_ERROR;
    }

    return SYSCALL_STATUS_OK;

#else

    processData.pid = ::fork();
    if (processData.pid < 0)
    {
        return SYSCALL_STATUS_CALL_ERROR;
    }
    else if (processData.pid == 0)
    {
        // parsing the command into binary and a table of arguments
        std::vector<std::string> split_command = splitQuotedString(command, ' ');

        if (split_command.empty()) exit(-1);

        std::string trimmed_command = trim(split_command[0]);

        char** argv = new char*[split_command.size() + 1];
        unsigned int i = 0;
        unsigned int j = 0;
        std::list<std::string> trimmed_args;
        for (; i < split_command.size(); i++)
        {
            std::string trimmed = trim(split_command[i]);

            if (!trimmed.empty())
            {
                trimmed_args.push_back(trimmed);
                argv[j] = const_cast<char*>(trimmed_args.back().c_str());
                j++;
            }
        }
        argv[j] = NULL;

        if (!logfile.empty())
        {
            int logfile_handle = open(logfile.c_str(), O_WRONLY | O_CREAT, S_IRWXU);
            // closing original STDOUT and STDERR handles
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            // replacing STDOUT and STDERR handles with logfile handle in the forked process
            dup2(logfile_handle, STDOUT_FILENO);
            dup2(logfile_handle, STDERR_FILENO);
            // closing the original file handle
            close(logfile_handle);
        }

        execvp(trimmed_command.c_str(), argv);
        exit(-1);
    }

    return SYSCALL_STATUS_OK;

#endif
}

int systemWithTimeout(std::string command, int& return_code, int timeout, std::string logfile)
{
    return_code = -1;

#ifdef WIN32

    ProcessData processData;
    if (startProcess(command, logfile, processData) != SYSCALL_STATUS_OK)
        return SYSCALL_STATUS_CALL_ERROR;

    HANDLE pid = processData.processInfo.hProcess;

    DWORD rc = WaitForSingleObject(pid, timeout);
    if (rc == WAIT_OBJECT_0)
    {
        // state is signalled 
        DWORD exitCode;
        if (GetExitCodeProcess(pid, &exitCode) == 0)
        {
            // error getting exit code
            TerminateProcess(pid, 1);
            return SYSCALL_STATUS_RUNTIME_ERROR;
        }
        return_code = (int)exitCode;
        return SYSCALL_STATUS_OK;
    }
    else if (rc == WAIT_TIMEOUT)
    {
        // timeout elapsed
        TerminateProcess(pid, 1);
        return SYSCALL_STATUS_TIMEOUT;
    }
    else
    {
        // wait failed
        TerminateProcess(pid, 1);
        return SYSCALL_STATUS_RUNTIME_ERROR;
    }

#else

    ProcessData processData;
    if (startProcess(command, logfile, processData) != SYSCALL_STATUS_OK)
        return SYSCALL_STATUS_CALL_ERROR;

    pid_t pid = processData.pid;

    int rc;
    int status;
    int time = 0;

    while (time < timeout) 
    {
        rc = waitpid(pid, &status, WNOHANG | WUNTRACED);
        if (rc == pid)
        {
            if (WIFEXITED(status))
            {
                // process exited
                return_code = WEXITSTATUS(status);
                return SYSCALL_STATUS_OK;
            }
            else if (WIFSIGNALED(status))
            {
                // process interrupted by a signal
                return SYSCALL_STATUS_RUNTIME_ERROR;
            }
        }
        usleep(LOOP_WAIT * 1000);
        time += LOOP_WAIT;
    }

    // timeout elapsed
    kill(pid, SIGKILL);
    return SYSCALL_STATUS_TIMEOUT;

#endif
}

int systemWithKillswitch(std::string command, int& return_code, std::atomic_bool& killswitch, std::string logfile)
{
    return_code = -1;

#ifdef WIN32
    
    ProcessData processData;
    if (startProcess(command, logfile, processData) != SYSCALL_STATUS_OK)
        return SYSCALL_STATUS_CALL_ERROR;

    HANDLE pid = processData.processInfo.hProcess;

    while (killswitch == false)
    {
        DWORD rc = WaitForSingleObject(pid, LOOP_WAIT);
        if (rc == WAIT_OBJECT_0)
        {
            // state is signalled 
            DWORD exitCode;
            if (GetExitCodeProcess(pid, &exitCode) == 0)
            {
                // error getting exit code
                TerminateProcess(pid, 1);
                return_code = -1;
                return SYSCALL_STATUS_RUNTIME_ERROR;
            }
            return_code = (int)exitCode;
            return SYSCALL_STATUS_OK;
        }
        else if (rc == WAIT_TIMEOUT)
        {
            // timeout elapsed, repeat loop
        }
        else
        {
            // wait failed
            TerminateProcess(pid, 1);
            return_code = -1;
            return SYSCALL_STATUS_RUNTIME_ERROR;
        }
    }

    TerminateProcess(pid, 1);
    return_code = -1;
    return SYSCALL_STATUS_KILLED;

#else

    ProcessData processData;
    if (startProcess(command, logfile, processData) != SYSCALL_STATUS_OK)
        return SYSCALL_STATUS_CALL_ERROR;

    pid_t pid = processData.pid;

    int rc;
    int status;
    while (killswitch == false)
    {
        rc = waitpid(pid, &status, WNOHANG | WUNTRACED);
        if (rc == pid)
        {
            if (WIFEXITED(status))
            {
                // process exited
                return_code = WEXITSTATUS(status);
                return SYSCALL_STATUS_OK;
            }
            else if (WIFSIGNALED(status))
            {
                // process interrupted by a signal
                return_code = -1;
                return SYSCALL_STATUS_RUNTIME_ERROR;
            }
        }
        usleep(LOOP_WAIT * 1000);
    }

    // killswitch engaged
    kill(pid, SIGKILL);
    waitpid(pid, &status, WNOHANG | WUNTRACED);
    return_code = -1;
    return SYSCALL_STATUS_KILLED;

#endif
}

int systemWithStdout(std::string cmd, std::string& output)
{
#ifdef WIN32
    // wrap command in extra quotations to ensure windows calls it properly
    cmd = "\"" + cmd + "\"";
#endif

    char buffer[TEMP_BUF];
    std::string result;

#ifdef WIN32
    std::shared_ptr<FILE> pipe(_popen(cmd.c_str(), "r"), _pclose);
#else
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
#endif

    if (!pipe)
    {
        return SYSCALL_STATUS_CALL_ERROR;
    }

    while (!feof(pipe.get()))
    {
        if (fgets(buffer, TEMP_BUF, pipe.get()) != NULL)
            result += buffer;
    }

    output = result;
    return SYSCALL_STATUS_OK;
}

int systemWithStdout(std::string cmd, std::string& output, int& cmdReturnCode)
{
#ifdef WIN32
    // wrap command in extra quotations to ensure windows calls it properly
    cmd = "\"" + cmd + "\"";
#endif

    char buffer[TEMP_BUF];
    std::string result;

    FILE* pipe;
#ifdef WIN32
    pipe = _popen(cmd.c_str(), "r");
#else
    pipe = popen(cmd.c_str(), "r");
#endif

    if (!pipe)
    {
        return SYSCALL_STATUS_CALL_ERROR;
    }

    while (!feof(pipe))
    {
        if (fgets(buffer, TEMP_BUF, pipe) != NULL)
            result += buffer;
    }

#ifdef WIN32
    cmdReturnCode = _pclose(pipe);
#else
    cmdReturnCode = pclose(pipe);
#endif
    output = result;
    return SYSCALL_STATUS_OK;
}
