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

#include <thread>
#include <chrono>
#include <map>
#include <mutex>
#include <atomic>
#include <vector>
#include <iostream>
#include <PipingManager.h>

#ifdef WIN32
#include <Windows.h>
#else
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#endif

#define DEFAULT_TIMEOUT (10000)
#define DEFAULT_BUFFER (1024*128)
#define NAMED_PIPE_BUFFER (1024*64)
#define WRITE_SIZE (16*1024)
#define READ_SIZE (16*1024)


using namespace std::chrono;
using std::vector;

class CircularFifo
{
    vector<char> mBuffer;
    size_t       mStart;
    size_t       mEnd;
    size_t       mSize;
    size_t       mFree;

public:
    CircularFifo()
    {
        mStart = 0;
        mEnd = 0;
        mSize = 0;
        mFree = 0;
    }
    void SetSize(size_t size)
    {
        mSize = size;
        mBuffer.resize(size);
        mFree = size;
    }
    int Append(char* buffer, size_t size)
    {
        if (size > mFree) return -1;
        if (mStart > mEnd)
        {
            memcpy(mBuffer.data() + mEnd, buffer, size);
        }
        else if (mStart <= mEnd)
        {
            size_t first_half = (mSize - mEnd > size) ? size : mSize - mEnd;
            size_t second_half = size - first_half;
            memcpy(mBuffer.data() + mEnd, buffer, first_half);
            if (second_half > 0)
                memcpy(mBuffer.data(), buffer + first_half, second_half);
        }
        mEnd = (mEnd + size) % mSize;
        mFree -= size;
        return 0;
    }
    int PeekFront(char* buffer, size_t size)
    {
        if (size > Taken()) return -1;

        if (mStart < mEnd)
        {
            memcpy(buffer, mBuffer.data() + mStart, size);
        }
        else if (mStart >= mEnd)
        {
            size_t first_half = (mSize - mStart > size) ? size : mSize - mStart;
            size_t second_half = size - first_half;
            memcpy(buffer, mBuffer.data() + mStart, first_half);
            if (second_half > 0)
                memcpy(buffer + first_half, mBuffer.data(), second_half);
        }
        return 0;
    }
    int PopFront(char* buffer, size_t size)
    {
        if (PeekFront(buffer, size) != 0)
            return -1;
        mStart = (mStart + size) % mSize;
        mFree += size;
        return 0;
    }
    int PopFront(size_t size)
    {
        if (size > Taken())
            return -1;
        mStart = (mStart + size) % mSize;
        mFree += size;
        return 0;
    }
    size_t Free()
    {
        return mFree;
    }
    size_t Taken()
    {
        return mSize - mFree;
    }
    size_t Size()
    {
        return mSize;
    }
};

// PIPE DATA CODE

struct PipeData
{
    std::string         mName;
    CircularFifo        mInBuf;
    CircularFifo        mOutBuf;
    vector<char>        mTempBuf;
    std::atomic<double> mKillTimer;
    std::thread         mThread;
    std::mutex          mMutex;
    std::atomic_bool    mThreadRunning;
    std::atomic_bool    mStop;
    std::atomic_bool	mCloseIfEmpty;
    pipe_type_t         mType;
    size_t              mBufferSize;
    piping_status_t     mStatus;
    std::string         mErrorString;
    int                 mId;
    size_t              mTotalDataWritten;
#ifdef WIN32
    HANDLE              mHandle;
#else
    int                 mHandle;
#endif

    PipeData(std::string name, size_t maxbuf, pipe_type_t type, int id);
    ~PipeData();
    void closeThread();
};

#ifndef WIN32
static inline
int get_flag(pipe_type_t type)
{
    if (type == INPUT_PIPE)
    {
        return O_WRONLY;
    }
    else if (type == OUTPUT_PIPE)
    {
        return O_RDONLY;
    }
    else
    {
        return O_RDWR;
    }
}
#endif

static
int pipe_write_func(PipeData* data)
{
    piping_status_t status = PIPE_MGR_OK;

    data->mMutex.lock();
    size_t writeSize = data->mInBuf.Taken();
    if (writeSize > WRITE_SIZE) writeSize = WRITE_SIZE;
    data->mInBuf.PeekFront(data->mTempBuf.data(), writeSize);
    data->mMutex.unlock();

#ifdef WIN32
    DWORD bytes_written = 0;
    if (WriteFile(data->mHandle, data->mTempBuf.data(), (DWORD)writeSize, &bytes_written, NULL) == FALSE)
    {
        DWORD last_error = GetLastError();
        data->mErrorString = std::to_string(last_error);
        status = PIPE_MGR_WRITE_ERROR;
    }
#else
    int bytes_written = write(data->mHandle, data->mTempBuf.data(), writeSize);
    if (bytes_written < 0)
    {
        if (errno != EAGAIN)
        {
            data->mErrorString = "write returned " + std::to_string(bytes_written) + " (errno=" + std::to_string((int)errno) + ").";
            status = PIPE_MGR_WRITE_ERROR;
        }
        bytes_written = 0;
    }
#endif

    if (bytes_written > 0)
    {
        data->mMutex.lock();
        data->mInBuf.PopFront(bytes_written);
        data->mMutex.unlock();
        data->mTotalDataWritten += bytes_written;
    }

    data->mStatus = status;

    if (data->mStatus == PIPE_MGR_OK)
        return bytes_written;
    else
        return -1;
}

static
int pipe_read_func(PipeData* data)
{
    piping_status_t status = PIPE_MGR_OK;
    size_t readSize = data->mOutBuf.Free();
    if (readSize > READ_SIZE) readSize = READ_SIZE;
    if (readSize == 0) return 0;

    // try to read from pipe (up to the size available in the buffer) and write that to the buffer
#ifdef WIN32
    DWORD bytes_read;
    if (ReadFile(data->mHandle, data->mTempBuf.data(), (DWORD)(readSize), &bytes_read, NULL) == FALSE)
    {
        DWORD last_error = GetLastError();
        data->mErrorString = std::to_string(last_error);
        if (last_error == ERROR_BROKEN_PIPE)
            status = PIPE_MGR_PIPE_CLOSED;
        else
            status = PIPE_MGR_READ_ERROR;
    }
#else
    int bytes_read = read(data->mHandle, data->mTempBuf.data(), readSize);
    if (bytes_read < 0)
    {
        if (errno != EAGAIN)
        {
            data->mErrorString = "read returned " + std::to_string(bytes_read) + " (errno=" + std::to_string((int)errno) + ").";
            status = PIPE_MGR_READ_ERROR;
        }
        bytes_read = 0;
    }
#endif

    if (bytes_read > 0)
    {
        data->mMutex.lock();
        data->mOutBuf.Append(data->mTempBuf.data(), bytes_read);
        data->mMutex.unlock();
    }

    data->mStatus = status;

    if (data->mStatus == PIPE_MGR_OK)
        return bytes_read;
    else
        return -1;
}

static
void pipe_thread_func(PipeData* data)
{
    data->mThreadRunning = true;

    // connect the pipe
#ifdef WIN32
    if (ConnectNamedPipe(data->mHandle, NULL) == NULL)
    {
        DWORD last_error = GetLastError();
        data->mErrorString = std::to_string(last_error);
        data->mStatus = PIPE_MGR_CONNECT_ERROR;
    }
#else
    signal(SIGPIPE, SIG_IGN); // ignore sigpipe, we handle errors in another way
    while ((data->mHandle = open(data->mName.c_str(), get_flag(data->mType) | O_NONBLOCK )) == -1)
    {
        if (errno != ENXIO) // ENXIO means the other end of the pipe is not ready and we need to try again
        {
            data->mErrorString = "open failed (errno=" + std::to_string((int)errno) + ").";
            data->mStatus = PIPE_MGR_CONNECT_ERROR;
            break;
        }
        if (data->mStop == true)
        {
            data->mStatus = PIPE_MGR_CONNECT_ERROR;
            break;
        }
    }
#endif

    if (data->mStatus != PIPE_MGR_OK)
    {
        data->mThreadRunning = false;
        return;
    }

    int written_bytes = 0;
    int read_bytes = 0;

    // run main thread loop
    while (data->mStop == false)
    {
        if ((data->mType == INPUT_PIPE || data->mType == DUPLEX_PIPE) && data->mInBuf.Taken() > 0)
        {
            written_bytes = pipe_write_func(data);
            if (written_bytes < 0)
            {
                break;
            }
            else if (written_bytes > 0)
                data->mKillTimer = 0;
        }
        if ((data->mType == OUTPUT_PIPE || data->mType == DUPLEX_PIPE) && data->mOutBuf.Free() > 0)
        {
            read_bytes = pipe_read_func(data);
            if (read_bytes < 0)
            {
                break;
            }
            else if (read_bytes > 0)
                data->mKillTimer = 0;
        }

        if (data->mCloseIfEmpty && data->mInBuf.Taken() == 0 && data->mOutBuf.Taken() == 0)
        {
            break;
        }
        
        if (written_bytes == 0 && read_bytes == 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

#ifdef WIN32
    CloseHandle(data->mHandle);
#else
    close(data->mHandle);
#endif

    data->mStatus = PIPE_MGR_PIPE_CLOSED;
    data->mThreadRunning = false;
}

void PipeData::closeThread()
{
    mStop = true;
#ifdef WIN32
    CancelIoEx(mHandle, NULL);
#endif
    if (mThread.joinable())
        mThread.join();

    mThreadRunning = false;
}

PipeData::PipeData(std::string name, size_t maxbuf, pipe_type_t type, int id)
{
    mKillTimer = 0;
    mThreadRunning = false;
    mType = type;
    mBufferSize = maxbuf;
    mId = id;
    mStatus = PIPE_MGR_OK;
    mStop = false;
    mCloseIfEmpty = false;
    mTotalDataWritten = 0;

    mTempBuf.resize(mBufferSize);
    if (type == INPUT_PIPE || type == DUPLEX_PIPE) mInBuf.SetSize(mBufferSize);
    if (type == OUTPUT_PIPE || type == DUPLEX_PIPE) mOutBuf.SetSize(mBufferSize);

#ifdef WIN32
    // on windows we only use the name of the temp file to create the named pipe
    mName = std::string("\\\\.\\pipe\\") + name.substr(name.rfind("\\") + 1);

    DWORD access = 0;

    if (type == INPUT_PIPE)          access = PIPE_ACCESS_OUTBOUND;
    else if (type == OUTPUT_PIPE)    access = PIPE_ACCESS_INBOUND;
    else                             access = PIPE_ACCESS_DUPLEX;

    mHandle = CreateNamedPipe(mName.c_str(),
        access,
        PIPE_WAIT | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
        1,
        NAMED_PIPE_BUFFER,
        NAMED_PIPE_BUFFER,
        NMPWAIT_USE_DEFAULT_WAIT,
        NULL);

    if (mHandle == INVALID_HANDLE_VALUE)
    {
        DWORD last_error = GetLastError();
        mErrorString = std::to_string(last_error);
        mStatus = PIPE_MGR_CREATE_ERROR;
    }
#else
    // on linux we remove the original temp file and create a pipe in it's place
    mHandle = 0;
    mName = name;
    remove(name.c_str());

    if (mkfifo(mName.c_str(), S_IWUSR | S_IWOTH | S_IRUSR | S_IROTH) != 0)
    {
        mStatus = PIPE_MGR_CREATE_ERROR;
    }
#endif

    if (mStatus == PIPE_MGR_OK)
    {
        mThread = std::thread(pipe_thread_func, this);
    }
    else
    {
        mId = -1;
    }
}

PipeData::~PipeData()
{
    closeThread();
#ifdef WIN32

#endif
}

// PIPING MANAGER CODE

struct PipingManagerData
{
    std::map<int, PipeData*>    mPipes;
    std::thread                 mThread;
    std::atomic<bool>           mThreadRunning;
    std::mutex                  mPipesMutex;
    std::atomic<bool>           mStopThread;
    std::atomic<int>            mPipeTimeout;
    size_t                      mBufferSize;
    std::string                 mErrMsg;
    int                         mLastId;
    bool                        mGlobalTimeout;
};

static
void piping_manager_thread_func(PipingManagerData* data)
{
    data->mThreadRunning = true;

    std::map<int, PipeData*>::iterator it;
    auto now = std::chrono::steady_clock::now();
    auto lastTime = std::chrono::steady_clock::now();
    duration<double, std::milli> timeDiff = now - lastTime;
    bool activityFlag = false;

    while (data->mStopThread == false)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        now = std::chrono::steady_clock::now();
        timeDiff = now - lastTime;
        activityFlag = false;

        data->mPipesMutex.lock();

        for (it = data->mPipes.begin(); it != data->mPipes.end(); ++it)
        {
            PipeData* pipe = it->second;
            if (pipe->mStatus != PIPE_MGR_OK)
            {
                continue;
            }
            pipe->mKillTimer = pipe->mKillTimer + timeDiff.count();
            if (pipe->mKillTimer >= data->mPipeTimeout && data->mGlobalTimeout == false && pipe->mStatus != PIPE_MGR_PIPE_CLOSED)
            {
                pipe->closeThread();
                pipe->mStatus = PIPE_MGR_TIMEOUT;
            }
            else if (pipe->mKillTimer < data->mPipeTimeout)
            {
                activityFlag = true;
            }
        }

        // no pipe activity detected, close all pipes
        if (activityFlag == false)
        {
            for (it = data->mPipes.begin(); it != data->mPipes.end(); ++it)
            {
                PipeData* pipe = it->second;
                if (pipe->mStatus != PIPE_MGR_PIPE_CLOSED)
                {
                    pipe->closeThread();
                    pipe->mStatus = PIPE_MGR_TIMEOUT;
                }
            }
        }

        data->mPipesMutex.unlock();

        lastTime = now;
    }

    data->mThreadRunning = false;
}

int PipingManager::createNamedPipe(std::string path, pipe_type_t type)
{
    PipeData* pipe_data = new PipeData(path, mData->mBufferSize, type, mData->mLastId);

    if (pipe_data->mId == -1)
    {
        delete pipe_data;
        return -1;
    }
    else
    {
        mData->mPipesMutex.lock();
        mData->mPipes.emplace(pipe_data->mId, pipe_data);
        mData->mPipesMutex.unlock();
        mData->mLastId += 1;

        return pipe_data->mId;
    }
}

piping_status_t PipingManager::destroyNamedPipe(int pipe_id)
{
    std::map<int, PipeData*>::iterator it = mData->mPipes.find(pipe_id);
    if (it == mData->mPipes.end())
    {
        return PIPE_MGR_PIPE_NOT_FOUND;
    }
    else
    {
        mData->mPipesMutex.lock();
        delete it->second;
        mData->mPipes.erase(it);
        mData->mPipesMutex.unlock();
        return PIPE_MGR_OK;
    }
}

piping_status_t PipingManager::closePipe(int pipe_id)
{
    std::map<int, PipeData*>::iterator it = mData->mPipes.find(pipe_id);
    if (it == mData->mPipes.end())
    {
        return PIPE_MGR_PIPE_NOT_FOUND;
    }
    else
    {
        it->second->mCloseIfEmpty = true;
        return PIPE_MGR_OK;
    }
}

piping_status_t PipingManager::getPipePath(int pipe_id, std::string& path)
{
    std::map<int, PipeData*>::iterator it = mData->mPipes.find(pipe_id);
    if (it == mData->mPipes.end())
    {
        return PIPE_MGR_PIPE_NOT_FOUND;
    }
    else
    {
        path = it->second->mName;
        return PIPE_MGR_OK;
    }
}

piping_status_t PipingManager::writeToPipe(int pipe_id, void* buffer, size_t data_size, size_t& bytes_written)
{
    piping_status_t status = PIPE_MGR_OK;
    bytes_written = 0;
    std::map<int, PipeData*>::iterator it = mData->mPipes.find(pipe_id);

    if (it == mData->mPipes.end())
    {
        status = PIPE_MGR_PIPE_NOT_FOUND;
    }
    else
    {
        PipeData* pipe = it->second;
        std::lock_guard<std::mutex> lock(pipe->mMutex);

        size_t data_to_write = data_size;
        if (data_to_write > pipe->mInBuf.Free())
        {
            data_to_write = pipe->mInBuf.Free();
        }

        if (pipe->mStatus != PIPE_MGR_OK)
        {
            status = pipe->mStatus;
        }
        else if (data_to_write > 0)
        {
            pipe->mInBuf.Append((char*)buffer, data_to_write);
            bytes_written = data_to_write;
        }
    }

    return status;
}

piping_status_t PipingManager::readFromPipe(int pipe_id, void* buffer, size_t buffer_size, size_t& bytes_read)
{
    piping_status_t status = PIPE_MGR_OK;
    bytes_read = 0;
    std::map<int, PipeData*>::iterator it = mData->mPipes.find(pipe_id);
    if (it == mData->mPipes.end())
    {
        status = PIPE_MGR_PIPE_NOT_FOUND;
    }
    else
    {
        PipeData* pipe = it->second;
        std::lock_guard<std::mutex> lock(pipe->mMutex);

        size_t data_to_read = buffer_size;
        if (data_to_read > pipe->mOutBuf.Taken())
        {
            data_to_read = pipe->mOutBuf.Taken();
        }

        if (data_to_read > 0)
        {
            pipe->mOutBuf.PopFront((char*)buffer, data_to_read);
            bytes_read = data_to_read;
        }

        status = pipe->mStatus;
    }

    return status;
}

piping_status_t PipingManager::pipeDataReady(int pipe_id, size_t& bytes_ready)
{
    piping_status_t status = PIPE_MGR_OK;
    std::map<int, PipeData*>::iterator it = mData->mPipes.find(pipe_id);
    if (it == mData->mPipes.end())
    {
        status = PIPE_MGR_PIPE_NOT_FOUND;
    }
    else
    {
        PipeData* pipe = it->second;
        std::lock_guard<std::mutex> lock(pipe->mMutex);
        bytes_ready = pipe->mOutBuf.Taken();
        status = pipe->mStatus;
    }

    return status;
}


piping_status_t PipingManager::pipeBufferFree(int pipe_id, uint64_t& bytes_available)
{
    piping_status_t status = PIPE_MGR_OK;
    std::map<int, PipeData*>::iterator it = mData->mPipes.find(pipe_id);
    if (it == mData->mPipes.end())
    {
        status = PIPE_MGR_PIPE_NOT_FOUND;
    }
    else
    {
        PipeData* pipe = it->second;
        std::lock_guard<std::mutex> lock(pipe->mMutex);
        bytes_available = pipe->mInBuf.Free();
        status = pipe->mStatus;
    }

    return status;
}

piping_status_t PipingManager::getPipeStatus(int pipe_id)
{
    std::map<int, PipeData*>::iterator it = mData->mPipes.find(pipe_id);
    if (it == mData->mPipes.end())
    {
        return PIPE_MGR_PIPE_NOT_FOUND;
    }
    else
    {
        return it->second->mStatus;
    }
}


void PipingManager::setTimeout(int timeout)
{
    mData->mPipeTimeout = timeout;
}

void PipingManager::setMaxbuf(size_t maxbuf)
{
    mData->mBufferSize = maxbuf;
}

void PipingManager::setGlobalTimeout(bool global_timeout)
{
    mData->mGlobalTimeout = global_timeout;
}

PipingManager::PipingManager()
{
    mData = new PipingManagerData();
    mData->mPipeTimeout = DEFAULT_TIMEOUT;
    mData->mThreadRunning = false;
    mData->mStopThread = false;
    mData->mBufferSize = DEFAULT_BUFFER;
    mData->mLastId = 0;
    mData->mGlobalTimeout = true;
    mData->mThread = std::thread(piping_manager_thread_func, mData);
    opened = true;
}

PipingManager::~PipingManager()
{
    close();
}

void
PipingManager::close()
{
    if(opened)
    {
        mData->mStopThread = true;
        mData->mThread.join();

        std::map<int, PipeData*>::iterator it;
        for (it = mData->mPipes.begin(); it != mData->mPipes.end(); ++it)
        {
            delete it->second;
        }

        delete mData;
        opened = false;
    }
}

std::string PipingManager::printInternalState()
{
    std::string message = "Piping manager state: \n";
    message += "    Thread running: " + std::to_string(mData->mThreadRunning) + "\n";

    std::map<int, PipeData*>::iterator it;
    for (it = mData->mPipes.begin(); it != mData->mPipes.end(); ++it)
    {
        PipeData* pipe = it->second;
        message += "    Pipe " + pipe->mName + ":\n";
        message += "        Running: " + std::to_string(pipe->mThreadRunning) + "\n";
        message += "        Status: " + std::to_string(pipe->mStatus) + "\n";
        message += "        Inbuffer: " + std::to_string(pipe->mInBuf.Taken()) + "/" + std::to_string(pipe->mInBuf.Size()) + "\n";
        message += "        Outbuffer: " + std::to_string(pipe->mOutBuf.Taken()) + "/" + std::to_string(pipe->mOutBuf.Size()) + "\n";
        message += "        Error message: " + pipe->mErrorString + "\n";
    }

    return message;
}
