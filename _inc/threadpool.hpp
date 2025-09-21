#pragma once

#include "common.h"

#include <bitset>
#include <cassert>
#include <cstddef>
#include <queue>
#include <thread>
#include <functional>
#include <atomic>
#include <condition_variable>


template <size_t N>
class ThreadPool
{
    std::array<std::jthread, N> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex tasks_lock;
    std::condition_variable cv;
    std::condition_variable main_thread_waiting_cv;

    void dump_log()
    {
        var(tasks.size());
    }

public:
    ThreadPool()
    {
        // startujemy N workerów
        for (size_t i = 0; i < N; ++i)
        {
            threads[i] = std::jthread([this, i](std::stop_token st)
            {
                // rejestrujemy callback, żeby przy request_stop() również wybudzić cv
                // std::stop_callback cb(st, [this]() { cv.notify_all(); });

                while (true)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(tasks_lock);

                        // czekamy aż będzie zadanie albo aż ktoś poprosi o stop
                        cv.wait(lock, [&]() { return st.stop_requested() || !tasks.empty(); });

                        // jeśli poproszono o stop i kolejka jest pusta -> wychodzimy
                        // stop_requested() = 1
                        // tasks.empty()    = 1
                        if (st.stop_requested() && tasks.empty()) { main_thread_waiting_cv.notify_all(); break; }

                        // jeśli mamy zadanie -> pobierz        -> nawet jeśli trzeba już kończyć
                        if (!tasks.empty())
                        {
                            // stop_requested() = 1 / 0
                            // tasks.empty()    = 0

                            task = std::move(tasks.front());
                            tasks.pop();
                        }
                        else
                        {
                            // stop_requested() = 0
                            // tasks.empty()    = 1
                            continue;
                        }
                    } // unlock mutex

                    // wykonujemy zadanie poza sekcją krytyczną
                    try
                    {
                        line("Worker " + std::to_string(i) + " starts task...");
                        task();
                        line("Worker " + std::to_string(i) + " FINISHES");
                        nline;
                    }
                    catch (...)
                    {
                        // opcjonalna obsługa wyjątków z tasków
                        line("Exception in task (ignored)");
                    }
                }

                line("Worker " + std::to_string(i) + " exiting");
            });
        }
    }

    void addTask(const std::function<void()>& task)
    {
        {
            std::lock_guard<std::mutex> lock(tasks_lock);
            tasks.push(task);
        }
        cv.notify_one();
    }

    ~ThreadPool()
    {
        // żądamy stopu dla wszystkich wątków
        // (nie przerywamy w połowie - worker zakończy po opróżnieniu kolejki)
        for (auto &t : threads) t.request_stop();

        // budzimy, żeby wszyscy czekający workerzy zobaczyli request_stop()
        cv.notify_all();

        // wątki same zdrainują kolejkę tasków // i potem się wyłączą

        {
            std::unique_lock<std::mutex> lock(tasks_lock);
            main_thread_waiting_cv.wait(lock, [&]() { return tasks.empty(); });
        }

        // tutaj mozna by ostatni cond variable -> konczace taski beda to callowac

        // while (true)
        // {
        //     std::unique_lock<std::mutex> lock(tasks_lock);
        //     dump_log();
        //     if (tasks.empty()) break;
        //     lock.unlock();
        //     std::this_thread::yield(); // hint to OS żeby wywłaszczył ten wątek i
        //                                // zdjął go z CPU, żeby inne mogły zrobić progres
        // }

        line("ThreadPool destroyed");
    }
};
