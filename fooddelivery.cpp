/**
 * @file    fooddelivery.cpp
 * @author  Bryce Jarboe
 * @brief   Simulates a concurrent producer-consumer model using POSIX threads.
 *          Pizza and sandwich requests are produced and consumed by delivery services.
 * @date    2024-04-18
 */

#include "log.h"
#include <iostream>
#include <unistd.h>
#include <deque>
#include <pthread.h>
#include <semaphore.h>
#include <atomic>
#include <cstring>
#include <cstdlib>
#include <ctime>

using namespace std;

// ---- Utility Functions ----

void fail() {
    cerr << "Error: terminating program due to incorrect usage or runtime failure.\n";
    exit(EXIT_FAILURE);
}

void success() {
    cerr << "Program completed successfully.\n";
    exit(EXIT_SUCCESS);
}

void usage_error() {
    cerr << "\nUsage: ./fooddelivery [options]\n"
         << "\t-n [Total number of delivery requests]        Default: 100\n"
         << "\t-a [Time for delivery (Service A, ms)]        Default: 0\n"
         << "\t-b [Time for delivery (Service B, ms)]        Default: 0\n"
         << "\t-p [Production time for sandwich request, ms] Default: 0\n"
         << "\t-s [Production time for pizza request, ms]    Default: 0\n\n";
    fail();
}

// ---- Argument Parsing ----

void parse_flags(int argc, char *argv[], Flags* flags) {
    int option;
    while ((option = getopt(argc, argv, "n:a:b:p:s:")) != -1) {
        int arg = stoi(optarg);
        if (arg < 0) usage_error();

        switch (option) {
            case 'n': flags->n = arg; break;
            case 'a': flags->a = arg; break;
            case 'b': flags->b = arg; break;
            case 'p': flags->p = arg; break;
            case 's': flags->s = arg; break;
            default:  usage_error();
        }
    }
}

// ---- Sleep Utility ----

void sleep_ms(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1'000'000L;
    nanosleep(&ts, nullptr);
}

// ---- Producer Function ----

void order(RequestType food, QueueData* data) {
    pthread_mutex_lock(data->access_ctrl);

    unsigned int in_queue[RequestTypeN] = {0};
    for (Requests r : *data->brokerQ) {
        in_queue[r]++;
    }

    bool can_add_sandwich = (food != Sandwich || in_queue[Sandwich] < 8);
    if (can_add_sandwich && data->production_limit->fetch_sub(1) > 0) {
        data->brokerQ->push_back(food);
        data->produced[food]++;
        log_added_request({food, data->produced, in_queue});
    }

    pthread_mutex_unlock(data->access_ctrl);
}

// ---- Producer Threads ----

void* little_caesars(void* args) {
    auto* data = static_cast<QueueData*>(args);
    int sleep_duration = data->flags->s;

    while (data->production_limit->load() > 0) {
        sleep_ms(sleep_duration);
        sem_wait(data->amount_empty);
        order(Pizza, data);
        sem_post(data->amount_full);
    }
    return nullptr;
}

void* jersey_mikes(void* args) {
    auto* data = static_cast<QueueData*>(args);
    int sleep_duration = data->flags->p;

    while (data->production_limit->load() > 0) {
        sleep_ms(sleep_duration);
        sem_wait(data->amount_empty);
        order(Sandwich, data);
        sem_post(data->amount_full);
    }
    return nullptr;
}

// ---- Consumer Logic ----

void deliver(ConsumerType delivery, QueueData* data) {
    pthread_mutex_lock(data->access_ctrl);
    if (!data->brokerQ->empty()) {
        RequestType req = data->brokerQ->front();
        data->brokerQ->pop_front();

        unsigned int in_queue[RequestTypeN] = {0};
        for (Requests r : *data->brokerQ) {
            in_queue[r]++;
        }

        data->consumed[delivery][req]++;
        log_removed_request({delivery, req, data->consumed[delivery], in_queue});
    }
    pthread_mutex_unlock(data->access_ctrl);
}

// ---- Consumer Threads ----

void* uberEats(void* args) {
    auto* data = static_cast<QueueData*>(args);
    int sleep_duration = data->flags->a;

    while (data->production_limit->load() > 0 || !data->brokerQ->empty()) {
        sleep_ms(sleep_duration);

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;

        if (sem_timedwait(data->amount_full, &ts) == 0) {
            deliver(DeliveryServiceA, data);
            sem_post(data->amount_empty);
        }
    }
    return nullptr;
}

void* doorDash(void* args) {
    auto* data = static_cast<QueueData*>(args);
    int sleep_duration = data->flags->b;

    while (data->production_limit->load() > 0 || !data->brokerQ->empty()) {
        sleep_ms(sleep_duration);

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;

        if (sem_timedwait(data->amount_full, &ts) == 0) {
            deliver(DeliveryServiceB, data);
            sem_post(data->amount_empty);
        }
    }
    return nullptr;
}

// ---- Main ----

int main(int argc, char *argv[]) {
    Flags flags;
    parse_flags(argc, argv, &flags);

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    sem_t empty, full;
    sem_init(&empty, 0, 20);
    sem_init(&full, 0, 0);

    deque<RequestType> queue;
    atomic<int> production_limit(flags.n);
    unsigned int produced[RequestTypeN] = {0};

    auto** consumed = new unsigned int*[ConsumerTypeN];
    for (int i = 0; i < ConsumerTypeN; ++i) {
        consumed[i] = new unsigned int[RequestTypeN]();
    }

    QueueData data = {
        .sandwich_count = nullptr,
        .production_limit = &production_limit,
        .brokerQ = &queue,
        .access_ctrl = &mutex,
        .amount_empty = &empty,
        .amount_full = &full,
        .produced = produced,
        .consumed = consumed,
        .flags = &flags
    };

    pthread_t pizza, sandwich, consumerA, consumerB;
    pthread_create(&pizza, nullptr, little_caesars, &data);
    pthread_create(&sandwich, nullptr, jersey_mikes, &data);
    pthread_create(&consumerA, nullptr, uberEats, &data);
    pthread_create(&consumerB, nullptr, doorDash, &data);

    pthread_join(pizza, nullptr);
    pthread_join(sandwich, nullptr);
    pthread_join(consumerA, nullptr);
    pthread_join(consumerB, nullptr);

    log_production_history(produced, consumed);

    // Clean-up
    for (int i = 0; i < ConsumerTypeN; ++i) delete[] consumed[i];
    delete[] consumed;

    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    success();
}
