#include "spsc_queue.hpp"
#include <sched.h>
#include <thread>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <chrono>

void pin_thread(int cpu)
{
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(cpu, &cpu_set);

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_set) == -1)
    {
        perror("pthread_setaffinity_np failed. \n");
        exit(EXIT_FAILURE);
    }
}

double median(std::vector<int64_t> &res)
{
    size_t k = res.size() / 2;
    std::nth_element(begin(res), begin(res) + k, end(res));

    double m = res[k];
    if (k & 1)
        return m;

    std::nth_element(begin(res), begin(res) + k - 1, end(res));
    return (m + res[k - 1]) * 0.5;
}

void print_metrics(std::vector<int64_t> &res)
{
    int64_t sum = std::accumulate(begin(res), end(res), int64_t{0});
    const char *unit = " elements/s \n";
    std::cout << std::fixed;
    std::cout.imbue(std::locale("en_US.utf8"));
    std::cout << "Mean: " << sum / static_cast<double>(res.size()) << unit;
    std::cout << "Median: " << median(res) << unit;
    std::cout << "Max: " << *std::max_element(begin(res), end(res)) << unit;
    std::cout << "Min: " << *std::min_element(begin(res), end(res)) << unit;
}

void run_benchmark(int cpu_consumer, int cpu_producer, int iterations)
{
    std::vector<int64_t> res(iterations, 0);
    constexpr int capacity = int(1 << 16);
    constexpr int elements = int(1e8);
    for (int iter = 0; iter < iterations; ++iter)
    {
        spsc_queue<int> sq(capacity);

        auto consumer = [&]
        {
            pin_thread(cpu_consumer);
            for (int i = 0; i < elements; ++i)
            {
                int item = -1;
                while (!sq.dequeue(item))
                    ;
                if (item != i)
                {
                    std::printf("Dequeue failed: expected = %d, but got %d \n", i, item);
                    exit(EXIT_FAILURE);
                }
            }
        };

        // start consumer thread
        std::thread th_consumer(consumer);

        // producer is current main thread
        pin_thread(cpu_producer);
        auto start = std::chrono::steady_clock::now();
        {
            for (int i = 0; i < elements; i++)
            {
                while (!sq.enqueue(i))
                    ;
            }

            // wait until the consumer dequeues all the items
            while (!sq.empty())
                ;
        }
        const std::chrono::duration<double> diff = std::chrono::steady_clock::now() - start;  // in seconds
        th_consumer.join();
        res[iter] = elements * double{1.0} / diff.count();
    }

    print_metrics(res);
}

int main()
{
    constexpr int cpu1 = 1;
    constexpr int cpu2 = 3;
    constexpr int iterations = 10;

    run_benchmark(cpu1, cpu2, iterations);
}