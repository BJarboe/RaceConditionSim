/** @name fooddelivery header
 *  @file fooddelivery.h
 *  @author Bryce Jarboe | RedID 825033151
 *  @date 4/18/2024
 *  @brief Concurrent producing and consuming
*/

#ifndef FOODDELIVERY_H
#define FOODDELIVERY_H

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <deque>
#include <iostream>
#include <getopt.h>
#include <atomic>

using namespace std;

extern const char *producers[];
extern const char *producerNames[];  
extern const char *consumerNames[]; 

// RequestType
typedef enum Requests {
  Pizza = 0,          
  Sandwich = 1,       
  RequestTypeN = 2,   
} RequestType;

// ConsumerType
typedef enum Consumers {
  DeliveryServiceA = 0,   
  DeliveryServiceB = 1,   
  ConsumerTypeN = 2, 
} ConsumerType;

// RequestAdded
typedef struct {
  RequestType type; 
  unsigned int *produced; 
  unsigned int *inBrokerQueue;
} RequestAdded;

// RequestRemoved
typedef struct {
  Consumers consumer; 
  RequestType type; 
  unsigned int *consumed; 
  unsigned int *inBrokerQueue;
} RequestRemoved;

struct Flags {
  int n = 100;
  int a = 0;
  int b = 0;
  int p = 0;
  int s = 0;
};

struct QueueData {
  int* sandwich_count;
  atomic<int>* production_limit;
  deque<RequestType>* brokerQ;
  pthread_mutex_t* access_ctrl;
  sem_t* amount_empty;
  sem_t* amount_full;

  unsigned int* produced;
  unsigned int** consumed;

  Flags* flags;
};

void success();
void fail();
void usage_error();
void parse_flags(int argc, char *argv[]);


// Queue Mutation
void order(RequestType food, QueueData* data);
void deliver(ConsumerType delivery, QueueData* data);

// Thread definitions
void *little_caesars(void* q);
void *jersey_mikes(void* q);
void *uberEats(void* q);
void *doorDash(void* q);

void sleep(int ms);

#endif
