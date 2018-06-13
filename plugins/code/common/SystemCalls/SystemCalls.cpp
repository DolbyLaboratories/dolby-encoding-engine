/*
* BSD 3-Clause License
*
* Copyright (c) 2018, Dolby Laboratories
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

#ifdef WIN32

#include <Windows.h>
#include <WinBase.h>

typedef HANDLE Pid;

#else

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <list>

typedef pid_t Pid;

#endif

// check threads every 100 miliseconds
#define LOOP_WAIT 100
#define TEMP_BUF 1024

template<typename Out>
static
void splitString(const std::string &s, char delim, Out result)
{
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        *(result++) = item;
    }
}

static
inline
std::vector<std::string> splitString(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    splitString(s, delim, std::back_inserter(elems));
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

int systemWithTimeout(std::string command, int& return_code, int timeout)
{
    return_code = -1;

#ifdef WIN32

    std::string command_line = trim(command, "\n");

    STARTUPINFO startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));

    BOOL status = CreateProcess(NULL, (LPSTR)(command_line.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
    CloseHandle(startupInfo.hStdInput);
    CloseHandle(startupInfo.hStdOutput);
    CloseHandle(startupInfo.hStdError);
    CloseHandle(processInfo.hThread);
    if (status == 0)
    {
        //DWORD error_code = GetLastError();
        return SYSCALL_STATUS_CALL_ERROR;
    }
    
    Pid pid = processInfo.hProcess;

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
        return SYSCALL_STATUS_OK;;
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

    Pid pid = ::fork();
    if (pid < 0) 
    {
        return SYSCALL_STATUS_CALL_ERROR;
    }
    else if (pid == 0)
    {
        std::vector<std::string> split_command = splitString(command, ' ');
        
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
        argv[i] = NULL;

        execvp(trimmed_command.c_str(), argv);
        exit(-1);
    }

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
                return_code = WEXITSTATUS(status);
                return SYSCALL_STATUS_OK;
            }
            else if (WIFSIGNALED(status))
            {
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

int systemWithKillswitch(std::string command, int& return_code, std::atomic_bool& killswitch)
{
    return_code = -1;

#ifdef WIN32

    std::string command_line = trim(command, "\n");

    STARTUPINFO startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));

    BOOL status = CreateProcess(NULL, (LPSTR)(command_line.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo);
    //DWORD error_code = GetLastError();
    CloseHandle(startupInfo.hStdInput);
    CloseHandle(startupInfo.hStdOutput);
    CloseHandle(startupInfo.hStdError);
    CloseHandle(processInfo.hThread);
    if (status == 0)
    {
        return_code = -1;
        return SYSCALL_STATUS_CALL_ERROR;
    }

    Pid pid = processInfo.hProcess;

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
    return_code = 0;
    return SYSCALL_STATUS_KILLED;

#else

    Pid pid = fork();
    if (pid < 0)
    {
        return_code = -1;
        return SYSCALL_STATUS_CALL_ERROR;
    }
    else if (pid == 0)
    {
        std::vector<std::string> split_command = splitString(command, ' ');
        
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
        
        execvp(trimmed_command.c_str(), argv);
        exit(-1);
    }

    int rc;
    int status;
    while (killswitch == false)
    {
        rc = waitpid(pid, &status, WNOHANG | WUNTRACED);
        if (rc == pid)
        {
            if (WIFEXITED(status))
            {
                return_code = WEXITSTATUS(status);
                return SYSCALL_STATUS_OK;
            }
            else if (WIFSIGNALED(status))
            {
                return_code = -1;
                return SYSCALL_STATUS_RUNTIME_ERROR;
            }
        }
        usleep(LOOP_WAIT * 1000);
    }

    // killswitch engaged
    
    kill(pid, SIGKILL);
    waitpid(pid, &status, WNOHANG | WUNTRACED); 
    return_code = 0;
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
