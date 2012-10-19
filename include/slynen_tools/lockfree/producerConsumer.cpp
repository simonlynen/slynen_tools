/*
 *   Written 2009, 2010 by <mgix@mgix.com>
 *   This code is in the public domain
 *   See http://www.mgix.com/snippets/?LockFree for details
 *
 *   Simple consumer-producer test for the lock-free LIFO stack.
 */

#include <lifo.h>
#include <stdio.h>
#include <atomic.h>
#include <thread.h>
#include <inttypes.h>

static Lifo recycled;                       // Pool of unused messages
static Lifo incoming;                       // Queue of incoming messages
static bool volatile done;                  // Signals that producers are done.
static int64_t volatile sum;                // Sum of all processed message contents
static int64_t volatile cnt;                // Total # of messages processed by consumers
static int64_t volatile nMsgs;              // Total # of messages structures created
static const int nConsumers =  8;           // Number of consumer threads to launch
static const int nProducers =  8;           // Number of produced threads to launch
static const int64_t workSize = (1<<24);    // # of messages created by a single producer

// Message data structure
struct Msg:public Lifo::Node
{
    int id;
    Msg() { atomicAdd64(&nMsgs, 1); }
};

// Consumer thread
static void *consumer(
    void *
)
{
    int64_t processed = 0;
    while(1)
    {
        Msg *msg = (Msg*)incoming.pop();    // Try to grab a message from incoming queue
        if(msg==0 && done) break;           // Queue empty, and no more messages will come
        if(msg)                             // If we got a message
        {
            atomicAdd64(&sum, msg->id);     // Add message content to sum
            atomicAdd64(&cnt, 1);           // Add one to count
            recycled.push(msg);             // Recycle message
            ++processed;                    // Count processed
        }
    }

    printf("Consumer thread consumed %" PRIu64 " messages\n", processed);
    return 0;
}

// Producer thread
static void *producer(
    void *
)
{
    uint32_t n = workSize;
    while(n--)                              // Produce lots of messages
    {
        Msg *msg = (Msg*)recycled.pop();    // Grab an unused message
        if(msg==0) msg = new Msg;           // If none to be had, make a new one
        msg->id = n;                        // Set message content
        incoming.push(msg);                 // Post it on shared queue
    }
}

int main(
    int     ,
    char    *[]
)
{
    // Launch a bunch of consumer threads
    void *consumers[nConsumers];
    for(int i=0; i<nConsumers; ++i) consumers[i] = thread(consumer);

    // Launch a bunch of producer threads
    void *producers[nProducers];
    for(int i=0; i<nProducers; ++i) producers[i] = thread(producer);

    // Wait for everything to complete
    for(int i=0; i<nProducers; ++i) join(producers[i]); // Wait for producers to finish
    done = true;                                        // Let consumers know producers are done
    for(int i=0; i<nConsumers; ++i) join(consumers[i]); // Wait for consumers to finish

    // Check result
    uint64_t n0 = nProducers*workSize;
    uint64_t n1 = (nProducers*workSize*(workSize-1))/2;
    printf("Messages processed = %" PRIu64 "\n", cnt);
    printf("Envelopes created = %" PRIu64 "\n", nMsgs);
    printf("delta1= %" PRIu64 " (should be zero)\n", cnt-n0);
    printf("delta2= %" PRIu64 " (should be zero\n", sum-n1);
    return 0;
}

