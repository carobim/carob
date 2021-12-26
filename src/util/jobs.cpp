#include "util/jobs.h"

#include "os/condition-variable.h"
#include "os/mutex.h"
#include "os/thread.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/queue.h"
#include "util/vector.h"

static Size workerLimit = 0;
static Vector<Thread> workers;
static U32 jobsRunning = 0;

// Empty jobs are the signal to quit.
static Queue<Function> jobs;

// Whether the destructor has been called,
static bool tearingDown = false;

// Access to workers vector, jobs deque, jobsRunning, and tearingDown.
static Mutex jobsMutex;

// Events for when a job is added.
static ConditionVariable jobAvailable;

// Events for when a job is finished.
static ConditionVariable jobsDone;

static void
work() noexcept {
    Function fn;

    do {
        {
            LockGuard lock(jobsMutex);

            while (jobs.size == 0) {
                jobAvailable.wait(lock);
            }

            fn = jobs.front();
            jobs.pop();

            jobsRunning += 1;
        }

        if (fn.fn) {
            fn.fn(fn.data);
        }

        {
            LockGuard lock(jobsMutex);

            jobsRunning -= 1;

            if (jobsRunning == 0 && jobs.size == 0) {
                jobsDone.notifyOne();
            }
        }
    } while (fn.fn);
}

void
JobsEnqueue(Function fn) noexcept {
    LockGuard lock(jobsMutex);

    assert_(!tearingDown);

    jobs.push(fn);

    if (workerLimit == 0) {
        workerLimit = threadHardwareConcurrency();
    }

    if (workers.size < workerLimit) {
        workers.push(Thread(fn));
    }

    jobAvailable.notifyOne();
}

void
JobsFlush() noexcept {
    // TODO: Don't quit the threads.

    // Wait for all jobs to finish.
    {
        Mutex m;
        LockGuard lock(m);

        while (jobsRunning > 0 || jobs.size > 0) {
            jobsDone.wait(lock);
        }
    }

    // Tell workers they can quit.
    {
        LockGuard lock(jobsMutex);

        // Send over empty jobs.
        for (Size i = 0; i < workers.size; i++) {
            Function fn;
            fn.fn = 0;
            fn.data = 0;
            jobs.push(fn);
        }

        tearingDown = true;
    }

    jobAvailable.notifyAll();

    // Wait for workers to quit.
    for (Thread* worker = workers.begin(); worker != workers.end(); worker++) {
        worker->join();
    }

    // Reset.
    workers.clear();
    tearingDown = false;
}
