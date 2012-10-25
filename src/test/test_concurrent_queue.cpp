
#include <queue>
#include <slynen_tools/concurrent_queue.h>
#include <boost/thread.hpp>
#include <gtest/gtest.h>

#define NELEMS  1000000
#define NTHREADS  8

class subelem{
	int value;
public:
	subelem(){
		value = 5;
	}
};

class testelem{
private:
	int _val;
	int _fromthread;
	subelem* _someclassarray;
	static const int nelems = 5;
	testelem& operator=(const testelem& other);
public:
	testelem(int val, int fromthread){
		_val = val;
		_fromthread = fromthread;

		_someclassarray = new subelem[nelems];
	}
	~testelem(){
		delete[] _someclassarray;
	}
	testelem(const testelem& other){
		this->_val = other._val;
		this->_fromthread = other._fromthread;

		_someclassarray = new subelem[nelems];
		memcpy(_someclassarray, other._someclassarray, sizeof(subelem)*nelems);
	}
	int getval(){
		return _val;
	}
};

typedef concurrent_queue<std::queue<testelem> > concqueue_T;

class consumer{
	boost::shared_ptr<concqueue_T> _queue;
public:
	consumer(boost::shared_ptr<concqueue_T> queue){
		_queue = queue;
	}
	void operator()(){
		long sumelems = 0;
		while(!_queue->empty()){
			testelem elem = _queue->pop();
			sumelems += elem.getval();
		}
		std::cout<<"consumer "<<sumelems<<std::endl;
	}
};

class producer{
	int _maxelem;
	int _id;
	boost::shared_ptr<concqueue_T> _queue;
public:
	producer(int nelems, int id, boost::shared_ptr<concqueue_T> queue){
		_maxelem = nelems;
		_queue = queue;
		_id = id;
	}
	void operator()(){
		int currelems = 0;
		while(currelems < _maxelem){
			_queue->push(testelem(++currelems, _id));
		}
		std::cout<<"producer "<<_id<<" pushed "<<currelems<<std::endl;
	}
};

int main(int argc, char **argv){

	long ntotalelementsexpected = NELEMS*NTHREADS;

	std::cout<<"now pushing "<<NELEMS*NTHREADS<<" elements of size "<<sizeof(testelem)<<" using "<<NTHREADS<<std::endl;

	boost::shared_ptr<concqueue_T> queue(new concqueue_T);

	// Create producers
	boost::thread_group producers;
	for (int i=0; i<NTHREADS; i++)
	{
		producer p(NELEMS, i, queue);
		producers.create_thread(p);
	}

	// Create consumers
	boost::thread_group consumers;
	for (int i=0; i<NTHREADS; i++)
	{
		consumer c(queue);
		consumers.create_thread(c);
	}

	boost::this_thread::sleep(boost::posix_time::milliseconds(500));

	std::cout<<"Waiting for the threads to exit"<<std::endl;
	//wait until everything is processed
	while(!queue->empty()){
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}

	//	ASSERT_EQ(x.size(), y.size()) << " some elements got lost";

	// Interrupt the threads and stop them
	producers.interrupt_all(); producers.join_all();
	consumers.interrupt_all(); consumers.join_all();
}

