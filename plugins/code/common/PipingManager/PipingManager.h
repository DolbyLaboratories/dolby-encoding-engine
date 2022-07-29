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

#include <string>
#include <condition_variable>

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
    void close();
    void setTimeout(int timeout);
    void setTimeout(int pipe_id, bool enable);
    void setMaxbuf(size_t maxbuf);
    void setGlobalTimeout(bool global_timeout);
    int createNamedPipe(std::string path, pipe_type_t type, bool client = false);
    piping_status_t destroyNamedPipe(int pipe_id);
    piping_status_t closePipe(int pipe_id, bool blocking = false);
    piping_status_t getPipePath(int pipe_id, std::string& path);
    piping_status_t writeToPipe(int pipe_id, void* buffer, size_t data_size, size_t& bytes_written);
    piping_status_t readFromPipe(int pipe_id, void* buffer, size_t buffer_size, size_t& bytes_read);
    piping_status_t pipeBufferFree(int pipe_id, uint64_t& bytes_available);
    piping_status_t pipeDataReady(int pipe_id, size_t& bytes_ready);
    piping_status_t getPipeStatus(int pipe_id);
    std::string printInternalState();
private:
    bool opened;
    PipingManagerData* mData;
};
