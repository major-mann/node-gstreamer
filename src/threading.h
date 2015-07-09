//Note: adapter from from https://github.com/mapbox/node-sqlite3/src/threading.h

#ifndef NSTR_THREADING_H
#define NSTR_THREADING_H
	#ifdef _WIN32
	    #include <windows.h>
	    #define NSTR_MUTEX_t HANDLE mutex;
	    #define NSTR_MUTEX_INIT mutex = CreateMutex(NULL, FALSE, NULL);
	    #define NSTR_MUTEX_LOCK(m) WaitForSingleObject(*m, INFINITE);
	    #define NSTR_MUTEX_UNLOCK(m) ReleaseMutex(*m);
	    #define NSTR_MUTEX_DESTROY CloseHandle(mutex);
	#elif defined(NSTR_BOOST_THREADING)
	    #include <boost/thread/mutex.hpp>
	    #define NSTR_MUTEX_t boost::mutex mutex;
	    #define NSTR_MUTEX_INIT
	    #define NSTR_MUTEX_LOCK(m) (*m).lock();
	    #define NSTR_MUTEX_UNLOCK(m) (*m).unlock();
	    #define NSTR_MUTEX_DESTROY mutex.unlock();
	#else
	    #define NSTR_MUTEX_t pthread_mutex_t mutex;
	    #define NSTR_MUTEX_INIT pthread_mutex_init(&mutex,NULL);
	    #define NSTR_MUTEX_LOCK(m) pthread_mutex_lock(m);
	    #define NSTR_MUTEX_UNLOCK(m) pthread_mutex_unlock(m);
	    #define NSTR_MUTEX_DESTROY pthread_mutex_destroy(&mutex);
	#endif
#endif