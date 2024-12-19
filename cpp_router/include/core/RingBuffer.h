// RingBuffer.h
#pragma once
#include <array>
#include <pthread.h>
#include <sched.h>
#include <stdexcept>
#include <thread>

template <typename T, size_t SIZE>
class RingBuffer
{
private:
    std::array<T, SIZE> buffer;
    static constexpr size_t MAX_CONSUMERS = SIZE - 1;
    std::array<volatile size_t, MAX_CONSUMERS> array_indexconsumer_to_indexbuffer{}; // member variable declaration NOT FUNCTION IMPLEMENTATION !!

public:
    // Helper methods
    static void pin_to_core(int core_id)
    {
        // cpu_set_t is a bit mask representing CPU cores
        cpu_set_t cpuset;
        // Initialize the CPU set to empty
        CPU_ZERO(&cpuset);
        // Add our target core to the set
        CPU_SET(core_id, &cpuset);

        // Attempt to pin current thread to specified core
        int result = pthread_setaffinity_np(
            pthread_self(),    // Current thread
            sizeof(cpu_set_t), // Size of the CPU set
            &cpuset            // Our CPU set
        );

        // Check if pinning was successful
        if (result != 0)
            throw std::runtime_error("Failed to set thread affinity");
        }

    static int get_cpu_count()
    {
        return std::thread::hardware_concurrency();
    }
    
    write_buffer(size_t indexbuffer, const T &data);
    auto read_buffer(size_t index_buffer);

    class Consumer
    {
    private:
        RingBuffer &ring;
        // No need to be volatile, cached and wont be edited
        const int64_t consumer_id;
        const int64_t neighbour_consumer_id;
        bool ready_for_next() const;

    public:
        Consumer(RingBuffer &r, int64_t id);
        void write(); // write directly onto ringbuffer
        bool read();
        void wait();
    };

    class Producer
    {
    private:
        RingBuffer &ring;
        const int64_t index_last_consumer;
        volatile size_t index_access_ready_buffer;

    public:
        explicit Producer(RingBuffer &r);
        bool write();
        void wait();
    };

    // Factory methods
    Producer createProducer();
    Consumer createConsumer(size_t id);

    size_t size() const;
};

// Note: Template implementation would go in RingBuffer.hpp
// or be included at the bottom of this header file