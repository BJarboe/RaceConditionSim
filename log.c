#include <stdio.h>
#include <time.h>

#include "log.h"

#ifdef __cplusplus
using namespace std;
#endif


const char *producers[] = {"Pizza delivery request", "Sandwich delivery request"};
const char *producerNames[] = {"PIZ", "SAN"};

const char *consumerNames[] = {"Delivery service A", "Delivery service B"};

double elapsed_s() {
  const double ns_per_s = 1e9;

  static timespec start;
  static int firsttime = 1;

  struct timespec t;

  clock_gettime(CLOCK_REALTIME, &t);

  if (firsttime) {
    firsttime = 0;  
    start = t; 
  }
  
  double s = (t.tv_sec - start.tv_sec) + 
    (t.tv_nsec - start.tv_nsec) / ns_per_s ;
  return s;
}

void log_added_request(RequestAdded requestAdded) {
  int idx;
  int total;

  printf("Broker: ");
  total = 0;  
  for (idx=0; idx < RequestTypeN; idx++) {
    if (idx > 0)
      printf(" + "); 
    printf("%d %s", requestAdded.inBrokerQueue[idx], producerNames[idx]);
    total += requestAdded.inBrokerQueue[idx];
  }

  printf(" = %d. ", total);

  printf("Added %s.", producers[requestAdded.type]);

  total = 0;
  printf(" Produced: ");
  for (idx=0; idx < RequestTypeN; idx++) {
    total += requestAdded.produced[idx]; 
    if (idx > 0)
      printf(" + "); 
    printf("%d %s", requestAdded.produced[idx], producerNames[idx]);
  }
  
  printf(" = %d in %.3f s.\n", total, elapsed_s());
  //printf(" = %d\n", total);
  fflush(stdout);  
};

void log_removed_request(RequestRemoved requestRemoved) {
  int idx;
  int total;

  total = 0;
  printf("Broker: ");
  for (idx=0; idx < RequestTypeN; idx++) {
    if (idx > 0)
      printf(" + "); 
    printf("%d %s", requestRemoved.inBrokerQueue[idx], producerNames[idx]);
    total += requestRemoved.inBrokerQueue[idx];
  }
  printf(" = %d. ", total);

  
  printf("%s consumed %s.  %s totals: ",
    consumerNames[requestRemoved.consumer],
    producers[requestRemoved.type],
    consumerNames[requestRemoved.consumer]);
  total = 0;
  for (idx = 0; idx < RequestTypeN; idx++) {
    if (idx > 0)
      printf(" + "); 
    total += requestRemoved.consumed[idx]; 
    printf("%d %s", requestRemoved.consumed[idx], producerNames[idx]);
  }
 
  printf(" = %d consumed in %.3f s.\n", total, elapsed_s());
  //printf(" = %d consumed\n", total);

  fflush(stdout);
};

void log_production_history(unsigned int produced[], 
                            unsigned int *consumed[]) {
  int p, c;  
  int total;

  printf("\nREQUEST REPORT\n----------------------------------------\n");
   
  for (p = 0; p < RequestTypeN; p++) {
    printf("%s producer generated %d requests\n",
	   producers[p], produced[p]);
  }
  for (c=0; c < ConsumerTypeN; c++) {
    printf("%s consumed ", consumerNames[c]);
    total = 0;
    for (p = 0; p < RequestTypeN; p++) {
      if (p > 0)
	printf(" + ");
    
      total += consumed[c][p];
      printf("%d %s", consumed[c][p], producerNames[p]);
    }
    printf(" = %d total\n", total);
  }

  printf("Elapsed time %.3f s\n", elapsed_s());
}
