#include "pool.h"
#include <mutex>
#include <iostream>

Task::Task() = default;
Task::~Task() = default;

ThreadPool::ThreadPool(int num_threads) {
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(new std::thread(&ThreadPool::run_thread, this));
    }
}

ThreadPool::~ThreadPool() {
    for (std::thread *t: threads) {
        delete t;
    }
    threads.clear();

    for (Task *q: queue) {
        delete q;
    }
    queue.clear();
}

void ThreadPool::SubmitTask(const std::string &name, Task *task) {
    //TODO: Add task to queue, make sure to lock the queue
    //std::cout << "called submit_task" << std::endl;
    if (!done && task != nullptr) {
        std::cout << "Added task: " << name << std::endl;
        mtx.lock(); 
        task->name = name; 
        queue.push_back(task);
        num_tasks_unserviced++;
        task->running = false;
        //std::cout << "Task " << name << " submitted." << std::endl;
        mtx.unlock();
    } else {
        std::cout << "Cannot added task to queue: " << name << " because the thread pool is stopped." << std::endl;
    }
}

void ThreadPool::run_thread() {
    while (true) {

        //TODO1: if done and no tasks left, break
        if (done && num_tasks_unserviced == 0) {
            break;
        }

        //TODO2: if no tasks left, continue
        if (num_tasks_unserviced == 0) {
            continue;
        }
        //TODO3: get task from queue, remove it from queue, and run it
        Task* curr = nullptr;
        mtx.lock(); // don't want two threads to run the same task at the same time
        if (!queue.empty()) {
            curr = queue.back();
            std::cout << "Started task: " << curr->name << std::endl;
            curr->running = true;
            queue.pop_back();
            num_tasks_unserviced--;
        }

        mtx.unlock();

        //TODO4: delete task
        if (curr != nullptr) {
            curr->Run();
            std::cout << "Finished task: " << curr->name << std::endl;
            delete curr;
        }
    }
}

// Remove Task t from queue if it's there
void ThreadPool::remove_task(Task *t) {
    mtx.lock();
    for (auto it = queue.begin(); it != queue.end();) {
        if (*it == t) {
            queue.erase(it);
            mtx.unlock();
            return;
        }
        ++it;
    }
    mtx.unlock();
}

void ThreadPool::Stop() {
    //TODO: Delete threads, but remember to wait for them to finish first
    done = true; 

    std::cout << "Called Stop()" << std::endl;
    for (auto thread : threads) {
        if (thread->joinable()) {
            std::cout << "Stopping thread:" << thread->get_id() << std::endl;
            thread->join();
        }
        // delete thread;  should be done in destructor
    }
}
