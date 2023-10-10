#ifndef WARP_STATE_COUNTER
#define WARP_STATE_COUNTER

// Defining custom indexes for the counter array
enum counters { WAITING, ISSUED, XALU, XMEM, OTHER, TOTAL };

// Counter array size
#define NUM_COUNTERS (TOTAL + 1)

// Declaring the counter array
extern unsigned long long warp_state_counters[NUM_COUNTERS];

#endif