/*
 * testLIFO.cc
 *
 *  Created on: Jan 16, 2012
 *      Author: slynen
 */


#include <gtest/gtest.h>

/*
 *   Written 2008 by <mgix@mgix.com>
 *   This code is in the public domain
 *   See http://www.mgix.com/snippets/?LockFree for details
 *
 *   Load test for the lock-free LIFO stack.
 */

#include <slynen_tools/lockfree/rng.h>
#include <slynen_tools/lifo_nonlocking.h>
#include <fcntl.h>
#include <stdio.h>
#include <slynen_tools/lockfree/timer.h>
#include <slynen_tools/lockfree/atomic.h>
#include <stdlib.h>
#include <slynen_tools/lockfree/thread.h>
#include <inttypes.h>

using namespace nonlocking;

// Code control
static const int nbThreads = 8;
static const double maxSeconds = 5.0;
static const int maxLoops =  500 * 1000 * 1000;

// Shared state
static int32_t volatile threadCounts = 0;
static int32_t volatile *histo = 0;
static Lifo lifo1;
static Lifo lifo2;

struct Node:public Lifo::Node
{
	uint32_t id;
	Node(uint32_t i) : id(i) {}
};

#define CHCK(x)                             \
		if(false==(x))                          \
		{                                       \
			printf(                             \
					"%s:%d Assertion %s fails\n",   \
					__FILE__,                       \
					__LINE__,                       \
					#x                              \
			);                                  \
			exit(1);                            \
		}                                       \

static void *stress(
		void *seed
)
{
	Node *head = 0;
	int32_t ownedNodes = 0;
	intptr_t tId = (intptr_t)seed;
	printf( "    thread %2d started ...\n", (int)tId);

	int32_t failedPops = 0;
	int32_t attemptedPops = 0;
	int32_t successfulPops = 0;

	int32_t failedPushes = 0;
	int32_t attemptedPushes = 0;
	int32_t successfulPushes = 0;

	int i=0;
	RNG rng(tId);
	bool useTry = 0==(tId%2);
	double nano0 = Timer::nano();
	while(i<maxLoops)
	{
		if(rng.sample32()&0x1)
		{
			Node *node = 0;
			if(false==useTry) node = (Node*)lifo1.pop();
			else
			{
				while(1)
				{
					uint32_t ok;
					++attemptedPops;
					node = (Node*)lifo1.tryPop(ok);
					if(0==node || 0!=ok) break;
					++failedPops;
				}
			}
			++successfulPops;

			if(node!=0)
			{
				node->next = head;
				++ownedNodes;
				head = node;
			}
		}
		else
		{
			Node *node = head;
			if(node!=0)
			{
				head = (Node*)node->next;
				if(false==useTry) lifo1.push(node);
				else
				{
					while(1)
					{
						++attemptedPushes;
						if(lifo1.tryPush(node)) break;
						++failedPushes;
					}
				}
				++successfulPushes;
				--ownedNodes;
			}
		}

		if(0==(i&0xFFFF))
		{
			double nano = Timer::nano()-nano0;
			if((maxSeconds*1e9)<nano) break;
		}
		++i;
	}

	int failedOps = failedPops + failedPushes;
	double elapsed = (Timer::nano()-nano0)*1e-9;
	int attemptedOps = attemptedPushes + attemptedPushes;
	int successfulOps = successfulPushes + successfulPops;

	if(useTry)
	{
		printf(
				"    thread %2d ended\n"
				"        %5.2f seconds\n"
				"        %7d nodes still owned\n"
				"        %6.2fM loops\n"
				"        %6.2fM successful ops,     %6.2fM attempts,     %6.2fM fails (%.2f%% of all attempts failed)\n"
				"        %6.2fM pushes,             %6.2fM attempts,     %6.2fM fails (%.2f%% of all attempts failed)\n"
				"        %6.2fM pops,               %6.2fM attempts,     %6.2fM fails (%.2f%% of all attempts failed)\n"
				"        %6.2fM successful ops/sec, %6.2fM total ops/sec\n"
				"\n",
				(int)tId,
				elapsed,
				ownedNodes,
				1e-6*i,

				1e-6*successfulOps,
				1e-6*attemptedOps,
				1e-6*failedOps,
				100.0*failedOps/(double)attemptedOps,

				1e-6*successfulPushes,
				1e-6*attemptedPushes,
				1e-6*failedPushes,
				100.0*failedPushes/(double)attemptedPushes,

				1e-6*successfulPops,
				1e-6*attemptedPops,
				1e-6*failedPops,
				100.0*failedPops/(double)attemptedPops,

				1e-6*successfulOps/elapsed,
				1e-6*attemptedOps/elapsed
		);
	}
	else
	{
		printf(
				"    thread %2d ended\n"
				"        %6.2f seconds\n"
				"        %7d nodes still owned\n"
				"        %6.2fM loops\n"
				"        %6.2fM ops\n"
				"        %6.2fM pushes\n"
				"        %6.2fM pops\n"
				"        %6.2fM ops/sec\n"
				"\n",
				(int)tId,
				elapsed,
				ownedNodes,
				1e-6*i,

				1e-6*successfulOps,
				1e-6*successfulPushes,
				1e-6*successfulPops,
				1e-6*successfulOps/elapsed
		);
	}

	atomicAdd32(&threadCounts, ownedNodes);
	while(head!=0)
	{
		Node *next = (Node*)head->next;
		atomicAdd32(head->id + histo, 1);
		head = next;
	}

	return 0;
}


TEST(LIFO, MultiThreadStress){
	printf("\n");

	// See what platform we're running on
#if defined(__i386__)
	printf("platform: i386\n");
#elif defined(__x86_64__)
	printf("platform: x86-64\n");
#else
#error "????"
#endif
	printf("\n");

	// Push nodes in the LIFO
	const int n = 10000;
	Node **nodes = new Node*[n];
	printf("pushing %d nodes into shared LIFO ... ", n);
	for(int i=0; i<n; ++i) lifo1.push(nodes[i]=new Node(i));
	histo = (int32_t volatile*)calloc(n, sizeof(uint32_t));
	printf("done\n");
	printf("\n");

	// Fork lots of threads that hammer on the LIFO
	void *threads[nbThreads];
	printf("starting %d thread accessing the LIFO concurrently for %.2f seconds\n", nbThreads, maxSeconds);
	for(int i=0; i<nbThreads; ++i) threads[i] = thread(stress, (void*)i);
	for(int i=0; i<nbThreads; ++i) join(threads[i]);
	printf("all threads done.\n");
	printf("\n");

	// Examine what's in the LIFO after the hammering
	int count = 0;
	printf("finalizing histogram ... ");
	Node *p = (Node*)lifo1.head;
	while(p)
	{
		Node *next = (Node*)p->next;
		++(histo[p->id]);
		p = next;
		++count;
	}
	printf("done\n");

	// Check that all nodes are still here and here exactly once
	printf("shared LIFO still contains %d nodes\n", count);
	printf("verifying histogram ... all entries should be exactly 1 ...");
	for(int i=0; i<n; ++i) CHCK(histo[i]==1);
	printf("check.\n");

	printf("sum of nodes left in various places = %d ... should be %d ... ", count + threadCounts, n);
	CHECK(0 == n - (count + threadCounts));
	printf("check.\n");
	free((void*)histo);

	// Cleanup
	printf("LIFO-tracked opCount = %d\n", (int)lifo1.sync);
	printf("everything checks out.\n");
	for(int i=0; i<n; ++i) delete nodes[i];
	delete [] nodes;
	printf("\n");
	return;
}


TEST(LIFO, EmptyCheck){
	const int n = 10000;
	Node **nodes = new Node*[n];
	printf("pushing %d nodes into shared LIFO ... ", n);
	for(int i=0; i<n; ++i) lifo2.push(nodes[i]=new Node(i));

	int cnt = 0;
	Node* nde = NULL;
	do{
		Node* head = (Node*)lifo2.head;
		nde = (Node*)lifo2.pop();
		std::cout<<"popping node "<<++cnt<<"             \r";std::cout.flush();
		if(nde)
			ASSERT_TRUE(head != NULL); //assert pre not empty
	}while(nde);
	std::cout<<std::endl;
	ASSERT_TRUE(lifo2.head == NULL); //assert empty
}

// Run all the tests that were declared with TEST()
int main(int argc, char **argv){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}


