#ifdef ANDROID
#include <sys/prctl.h>
#endif

#include "Thread.h"

namespace yoghurt
{
Thread::Thread()
    :mExitPending(false),
    mRunning(false),
    mName(NULL)
{
}

Thread::~Thread()
{
    requestExitAndWait();
}

bool Thread::run(const char * name)
{
    AutoMutex lock(mLock);
    if (mRunning)
    {
        return false;
    }

    mName = name;
    mExitPending = false;
    pthread_t threadID;

    pthread_attr_t attr;
    int ret = pthread_attr_init(&attr);
    if (ret < 0)
    {
        return false;
    }

    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (ret < 0)
    {
        pthread_attr_destroy(&attr);
        return false;
    }

    ret = pthread_create(&threadID, &attr, &entryFunction, (void*)this);
    if (ret < 0)
    {
        pthread_attr_destroy(&attr);
        return false;
    }

    pthread_attr_destroy(&attr);
    mRunning = true;
    return true;
}

void Thread::requestExitAndWait()
{
    AutoMutex lock(mLock);

    while (mRunning)
    {
        mExitPending = true;
        mCondition.wait(mLock);
    }
}

void Thread::requestExit()
{
    AutoMutex lock(mLock);
    mExitPending = true;
}

bool Thread::exitPending()
{
    AutoMutex lock(mLock);
    return mExitPending;
}

void* Thread::entryFunction(void* userData)
{
    if (userData != NULL)
    {
        ((Thread*)userData)->_threadLoop();
    }

    return NULL;
}

bool Thread::readyToRun()
{
    return true;
}

void Thread::_threadLoop()
{
    bool ret = true;

#ifdef ANDROID
    prctl(PR_SET_NAME, mName);
#endif

    if (!readyToRun())
    {
        goto OUT;
    }

    do
    {
        ret = threadLoop();
    }
    while (ret && !exitPending());

OUT:
    AutoMutex lock(mLock);
    mExitPending = true;
    mRunning = false;
    mCondition.boardCast();
}

} // namespace yoghurt
