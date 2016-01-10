/*
 A simple program that takes in an integer n, and computes the number of prime numbers in [1...n] using multi-threading.
 The program also prints out the time taken for the computation.
 Dev platform: OS X Yosemite (Intel Core i7 @ 2.9GHz, 2 cores)
 Compilation: g++ CountingPrime.cc -o CountingPrime -pthread
*/

#include <iostream>
#include <pthread.h>
#include <string>
#include <cmath>
#include <sys/time.h>
#include <iomanip>
using namespace std;

int n; // upper bound of range of integers to be searched for
int sqrt_n; // square root of n
bool *isPrime; // bool array storing whether the index no. is prime

// bounded buffer
int buffer[10] = {0}; // buffer
int job_count = 0;	// indicate the number of pending jobs
int next_job = 0;	// point to the next available job
int next_slot = 0;	// point to the next free slot

pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t wait_job = PTHREAD_COND_INITIALIZER;
pthread_cond_t wait_slot = PTHREAD_COND_INITIALIZER;

void *worker(void *);

int main(int argc, char* argv[]) {
	
	int W = 4; // no. of worker threads
	if ( argc < 2 ) {
		cout << "Error: not enough input arguments.\n";
		exit(1);
	} else {
		n = stoi( argv[1] );
		sqrt_n = sqrt(n);
		if ( argc > 2 )
			W = stoi( argv[2] );
	}
	
	// start counting the real time spent
	timeval time;
	gettimeofday(&time, NULL);
	double startTime = time.tv_sec + time.tv_usec / 1000000.0;
	
	// create W worker threads
	pthread_t worker_thread[W];
	for (int i=0; i<W; i++ ) {
		if ( pthread_create(&worker_thread[i], NULL, worker, NULL) ) {
			cout << "Error: thread creation failed.\n";
			exit(1);
		}
	}
	
	// isPrime[0] and isPrime[1] are dummy slots
	isPrime = new bool[n+1];
	memset(isPrime, 1, sizeof(bool) * (n+1));
	
	// between 2 and sqrt_n:
	// - put prime no. in the buffer
	// - mark composite no.
	for (int k=2; k<=sqrt_n; k++) {
		if ( !isPrime[k] )
			continue;
		
		// put prime no. into the buffer
		pthread_mutex_lock( &buffer_lock );
		while ( job_count == 10 )
			pthread_cond_wait( &wait_slot, &buffer_lock );
		buffer[next_slot] = k;
		job_count++;
		next_slot = (next_slot+1) % 10;
		pthread_cond_signal( &wait_job );
		pthread_mutex_unlock( &buffer_lock );
		
		// mark composite no.
		int k_2 = k*k;
		if ( k_2 > sqrt_n )
			continue;
		for (int i=k_2; i<=sqrt_n; i+=k)
			isPrime[i] = 0;
	}
	
	// tell all worker threads that there are no more jobs
	for (int i=0; i<W; i++) {
		pthread_mutex_lock( &buffer_lock );
		while ( job_count == 10 )
			pthread_cond_wait( &wait_slot, &buffer_lock );
		buffer[next_slot] = -1;
		job_count++;
		next_slot = (next_slot+1) % 10;
		pthread_cond_signal( &wait_job );
		pthread_mutex_unlock( &buffer_lock );
	}
	
	for (int i=0; i<W; i++)
		pthread_join( worker_thread[i], NULL );
	
	// count the no. of prime no. between 2 to n
	int count = 0;
	for (int i=2; i<=n; i++) {
		if (isPrime[i])
			count++;
	}
	
	// Record the end time
	gettimeofday(&time, NULL);
	double endTime = time.tv_sec + time.tv_usec / 1000000.0;
	double time_diff = endTime - startTime;
	
	cout << "Total number of prime numbers between 1 to " << n << " = " << count << endl;
	cout << "Total elapsed time: "
			 << fixed << setprecision(4)
			 << time_diff << " s\n";
	
	delete[] isPrime;
	pthread_mutex_destroy( &buffer_lock );
	pthread_cond_destroy( &wait_job );
	pthread_cond_destroy( &wait_slot );
	
	return 0;
	
}

void *worker(void *arg) {
	
	while (1) {
		// get a prime no. from the buffer
		int k;
		pthread_mutex_lock( &buffer_lock );
		while (job_count == 0)
			pthread_cond_wait( &wait_job, &buffer_lock );
		k = buffer[next_job];
		job_count--;
		next_job = (next_job+1) % 10;
		pthread_cond_signal( &wait_slot );
		pthread_mutex_unlock( &buffer_lock );
		
		// no more job
		if (k < 0)
			pthread_exit(NULL);
		
		// mark composite no.
		int startNo;
		if ( (sqrt_n+1) % k ) {
			int intermediate = (sqrt_n+1)/k;
			startNo = intermediate * k + k;
		} else
			startNo = sqrt_n+1;
		for (int i=startNo; i<=n; i+=k)
				isPrime[i] = 0;
	}
	
}

