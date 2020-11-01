
#ifndef Lock_Library
#define Lock_Library


#include <condition_variable>
#include <mutex>
#include <atomic>


// lockSpace includes class : Lock
// Lock allows taking read/write lock
//
// write lock are prioritized, and all new read lock are
// scheduled, after all current write lock are scheduled.
// No Pre-Emption is used.
//
// write lock allows recursively lock acquiring by same id
//
// provides blocking/non-blocking request methods
//
// provides methods : getWriteLock, releaseWriteLock, getReadLock, releaseReadLock,
//                  getAsyncWriteLock, getAsyncReadLock
//
// provides methods : block, release to support synchronous start between threads
namespace lockSpace
{
    class Lock
    {
        public:
        std::condition_variable waitcv;                 // wait lock queue cv
        std::condition_variable readcv;                 // read lock queue cv
        std::condition_variable writecv;                // write lock queue cv
        std::mutex mxt;

        size_t ReadRequest = 0;                            // number of current blocking readlock requests
        size_t WriteRequest = 0;                           // number of current blocking writelock requests

        size_t ReadRunning = 0;                            // number of read request running
        size_t WriteRunning = 0;                           // number of write request running(1|0)

        size_t writer_id = 0;                              // if write lock is acquired, id of owner

        std::mutex read;
        std::mutex write;
        std::mutex wait;

        public:
        size_t newID()
        {
            static std::atomic<size_t> id(0);
            return ++id;
        }

        public:
        // blocking write lock request
        size_t getWriteLock(size_t id = -1)
        {
            if(id == -1){id = newID();}

            mxt.lock();

            if(ReadRunning == 0){
                if((WriteRunning != 0 && writer_id == id) || WriteRunning == 0){
                    WriteRunning++;
                    writer_id = id;
                    mxt.unlock();

                    return id;
                }
            }

            WriteRequest++;

            mxt.unlock();

            std::unique_lock<std::mutex> lck(write);
            writecv.wait(lck);

            mxt.lock();
            writer_id = id;
            mxt.unlock();

            return id;
        }

        // write lock release method
        void releaseWriteLock()
        {
            mxt.lock();

            WriteRunning--;
            if(WriteRunning == 0){
                writer_id = 0;

                if(WriteRequest != 0){
                    WriteRequest--;
                    WriteRunning++;
                    writecv.notify_one();
                }
                else if(ReadRequest != 0){
                    ReadRunning = ReadRequest;
                    ReadRequest = 0;
                    readcv.notify_all();
                }
            }

            mxt.unlock();
        }

        // blocking read lock request
        void getReadLock()
        {
            mxt.lock();

            if(WriteRunning == 0 && WriteRequest == 0){
                ReadRunning++;
                mxt.unlock();
                return;
            }
            ReadRequest++;

            mxt.unlock();

            std::unique_lock<std::mutex> lck(read);
            readcv.wait(lck);
        }

        // readlock release method
        void releaseReadLock()
        {
            mxt.lock();

            ReadRunning--;
            if(ReadRunning == 0)
            {
                if(WriteRequest != 0){
                    WriteRequest--;
                    WriteRunning++;
                    writecv.notify_one();
                }
                else if(ReadRequest != 0){
                    ReadRunning = ReadRequest;
                    ReadRequest = 0;
                    readcv.notify_all();
                }
            }

            mxt.unlock();
        }

        // non-blocking write lock request
        bool getAsyncWriteLock(size_t id = 0)
        {
            if(id == 0){id = newID();}

            bool state = false;
            mxt.lock();

            if(ReadRunning == 0){
                if((WriteRunning == 0 || writer_id == id)){
                    WriteRunning++;
                    writer_id = id;
                    state = true;
                }
            }

            mxt.unlock();
            return state;
        }

        // non-blocking read lock request
        bool getAsyncReadLock()
        {
            bool state = false;
            mxt.lock();

            if(WriteRunning == 0 && WriteRequest == 0){
                ReadRunning++;
                state = true;
            }

            mxt.unlock();
            return state;
        }

        // blocking lock request
        // can be used to block all threads for a synchronous start point
        void block()
        {
            std::unique_lock<std::mutex> lck(wait);
            waitcv.wait(lck);
        }

        // release all threads over lock request
        void release()
        {
            waitcv.notify_all();
        }
    };
}
#endif // lock_library
