#ifndef LOG_H
#define LOG_H
#include "fooddelivery.h"

void log_added_request (RequestAdded requestAdded);

void log_removed_request(RequestRemoved requestRemoved);

void log_production_history(unsigned int produced[], unsigned int *consumed[]);

#endif
