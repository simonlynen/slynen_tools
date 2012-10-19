/*
 * universal_batch_processor.h
 *
 *  Created on: Dec 2, 2011
 *      Author: slynen
 */

#ifndef UNIVERSAL_BATCH_PROCESSOR_H_
#define UNIVERSAL_BATCH_PROCESSOR_H_

#include <slynen_tools/concurrent_queue.h>
#include <string>
#include <queue>
#include <algorithm>
#include <opencv2/opencv.hpp>

struct counter{
private:
	size_t done;
protected:
	mutable boost::mutex mmtx;
public:
	counter(){
		done = 0;
	}
	void increasedone(){
		boost::mutex::scoped_lock lock(mmtx);
		++done;
	}
	size_t getdone(){
		boost::mutex::scoped_lock lock(mmtx);
		return done;
	}
};

template<typename container_T, typename func_T>
struct worker{
	typedef typename container_T::value_type job_T;
	container_T* qjobs;
	container_T* qdone;
	counter* cnt;
	func_T (*function)(typename container_T::value_type&);
	worker(container_T& _qjobs, func_T _function(job_T&), container_T* _qdone, counter& _cnt){
		qjobs = &_qjobs;
		qdone = _qdone;
		function = _function;
		cnt = &_cnt;
	}
	void operator()(){
		while(1){
			boost::this_thread::interruption_point();
			job_T job = qjobs->pop();
			function(job);
			if(qdone){
				qdone->push(job);
			}
			cnt->increasedone();
		}
	}
};

template<typename container_T>
struct status{
	container_T* qjobs;
	container_T* qdone;
	boost::thread_group* threads;
	bool* termsignal;
	mutable boost::mutex* mmutex;
	boost::condition_variable* mcond;
	std::string jobname;
	size_t total;
	bool* finished;
	counter* cnt;
	double starttime;
	void (*function)(typename container_T::value_type&);
	status(container_T& _qjobs, container_T* _qdone, boost::thread_group& _threads,
			bool& termsig, std::string _jobname, size_t _total, boost::mutex& _mutex,
			boost::condition_variable& _cond, bool& _finished, counter& _cnt, double _starttime){
		qjobs = &_qjobs;
		threads = &_threads;
		termsignal = &termsig;
		jobname = _jobname;
		total = _total;
		mmutex = &_mutex;
		finished = &_finished;
		mcond = &_cond;
		cnt = &_cnt;
		qdone = _qdone;
		starttime = _starttime;
	}
	void operator()(){
		if(jobname!="")
			cout<<"Batch processor status monitor is alive...\r"; cout.flush();
		int i = 0;
		size_t idone = 0;
		while(idone != total && !(*termsignal)){
			idone = cnt->getdone();
			if(jobname!="")
				if(i++%100 == 0){
					int stcksze = total-idone;
					cout<<"Working on job: "<<jobname<<": "<<(double)idone/total * 100.<<"% [jobs: on stack: "<<stcksze<<" / done: "<<idone<<"]              \r"; cout.flush();
				}
			boost::this_thread::sleep(boost::posix_time::millisec(50));
		}
		if(jobname!=""){
			cout<<"Interrupting workers...                                                                        \r"; cout.flush();
			cout.flush();
		}
		//		qjobs->notify(); //in case something is blocking
		//		boost::this_thread::sleep(boost::posix_time::microsec(100));
		//		qjobs->notify(); //in case something is blocking
		boost::this_thread::sleep(boost::posix_time::millisec(10));
		threads->interrupt_all();
		if(jobname!=""){
			cout<<"Interrupted workers                                                                         \r"; cout.flush();
			cout.flush();
		}
		if(jobname!=""){
			cout<<"Joining workers...                                                                         \r"; cout.flush();
			cout.flush();
		}

		boost::this_thread::sleep(boost::posix_time::microsec(10));
		threads->join_all();

		double ttotal = (double)cvGetTickCount()-starttime;
		if(jobname!="")
			cout<<"Finished job "<<jobname<<": "<<(double)idone/total * 100.<<"% [completed "<<idone<<" jobs] took: "<<ttotal/((double)cvGetTickFrequency()*1000.*1000.)<<" s                                               \r"; cout.flush();


		//notify waiting threads
		boost::mutex::scoped_lock lock(*mmutex);
		*finished = true;
		lock.unlock();
		mcond->notify_one();
	}
};

struct batch_processor{
private:
	bool finished;
	mutable boost::mutex mmutex;
	boost::condition_variable mcond;
	counter cnt;
	boost::thread_group workers;
public:
	bool terminateafterfinish;
	size_t total;
	batch_processor(size_t totaljobs, bool terminatewhenfinished = true){
		terminateafterfinish = terminatewhenfinished;
		total = totaljobs;
		finished = false;
	}
	bool notify_me_when_finished(){
		boost::mutex::scoped_lock lock(mmutex);
		while(!finished)
		{
			mcond.wait(lock);
		}
		return finished;
	}


	template<typename container_T, typename func_T>
	boost::thread batch_process(container_T& qjobs, func_T function(typename container_T::value_type&), std::string jobname, container_T* qdone = NULL, size_t NTHREADS = 0){
		double tt = (double)cvGetTickCount();
		typedef typename container_T::value_type job_T;
		size_t ithreads;
		if(NTHREADS==0){
			ithreads = total < boost::thread::hardware_concurrency()?total : boost::thread::hardware_concurrency(); //don't create more threads than jobs.
		}else{
			ithreads = total < NTHREADS?total:NTHREADS; //don't create more threads than jobs.
		}
		for(size_t i = 0;i<ithreads;++i){
			worker<container_T, func_T> w(qjobs, function, qdone, cnt);
			workers.create_thread(w);
		}
		if(jobname!="")
			cout<<"Batch processor started "<<ithreads<<" threads                                            \r"; cout.flush();
		//create another thread to update the status
		status<container_T> s(qjobs, qdone, workers, terminateafterfinish, jobname, total, mmutex, mcond, finished, cnt, tt);
		return boost::thread(s);
	}
};
#endif /* UNIVERSAL_BATCH_PROCESSOR_H_ */
