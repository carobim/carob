#include "util/jobs.h"

#include "os/condition-variable.h"
#include "os/mutex.h"
#include "os/thread.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/function.h"
#include "util/int.h"
#include "util/queue.h"
#include "util/vector.h"

static size_t workerLimit = 0;
static Vector<Thread> workers;
static int jobsRunning = 0;

// Empty jobs are the signal to quit.
static Queue<Job> jobs;

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
    Job job;

    do {
        {
            LockGuard lock(jobsMutex);

            while (jobs.size == 0) {
                jobAvailable.wait(lock);
            }

            job = static_cast<Job&&>(jobs.front());
            jobs.pop();

            jobsRunning += 1;
        }

        if (job) {
            job();
        }

        {
            LockGuard lock(jobsMutex);

            jobsRunning -= 1;

            if (jobsRunning == 0 && jobs.size == 0) {
                jobsDone.notifyOne();
            }
        }
    } while (job);
}

void
JobsEnqueue(Job job) noexcept {
    LockGuard lock(jobsMutex);

    assert_(!tearingDown);

    jobs.push(static_cast<Job&&>(job));

    if (workerLimit == 0) {
        workerLimit = threadHardwareConcurrency();
    }

    if (workers.size < workerLimit) {
        workers.push_back(Thread(Job(work)));
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
        for (size_t i = 0; i < workers.size; i++) {
            jobs.push(Job());
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
