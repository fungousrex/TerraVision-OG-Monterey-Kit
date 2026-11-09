#ifndef _pthr_sproc_h_
#define _pthr_sproc_h_

/* Suppress <sys/pthread.h> on SGI */
#define _SYS_PTHREAD_H_

#ifndef _SGI_MP_SOURCE
#  define _SGI_MP_SOURCE
#endif

#ifdef __GNUC__
# define NOEXIT_FUNC volatile
#else
# define NOEXIT_FUNC
#endif

#ifndef DEBUG_PTHREAD
#  define DEBUG_PTHREAD (1)
#endif

#include <time.h>

extern int __pthread_monitor (int new_val);
extern int __pthread_debug (int new_val);

typedef struct _pthread *pthread_t;
typedef struct _pthread_attr pthread_attr_t;

extern int
pthread_create (pthread_t *new_thread_id,
		const pthread_attr_t *attr,
		void *(*start_func) (void*),
		void *arg);

extern pthread_t
pthread_self (void);

extern NOEXIT_FUNC void
pthread_exit (void *status);

extern int
pthread_join (pthread_t target, void **statusp);

extern int
pthread_detach (pthread_t target);

extern int
pthread_kill (pthread_t thread, int signo);


#define PTHREAD_CANCELED ((void *) -1)

#define PTHREAD_CANCEL_DISABLE (0)
#define PTHREAD_CANCEL_ENABLE (1)

#define PTHREAD_CANCEL_DEFERRED (0)
#define PTHREAD_CANCEL_ASYNCHRONOUS (1)

extern int
pthread_cancel (pthread_t thread);

extern int
pthread_setcancelstate (int state, int *oldstate);

extern int
pthread_setcanceltype (int ctype, int *oldctype);

extern void
pthread_testcancel (void);

extern void
pthread_cleanup_push (void (*func) (void *), void *closure);

extern void
pthread_cleanup_pop (int execute);

typedef int pthread_key_t;

extern int
pthread_key_create (pthread_key_t *keyp, void (*destructor) (void *value));

extern int
pthread_setspecific (pthread_key_t key, void const *value);

extern void *
pthread_getspecific (pthread_key_t key);

extern int
pthread_key_delete (pthread_key_t key);

typedef struct _pthread_mutex *pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER ((pthread_mutex_t) 1)

typedef struct {
  int foo;
} pthread_mutex_attr_t;

int
pthread_mutex_init (pthread_mutex_t *mp,
		    pthread_mutex_attr_t const *attr);

extern int
pthread_mutex_lock (pthread_mutex_t *mp);

extern int
pthread_mutex_unlock (pthread_mutex_t *mp);

extern int
pthread_mutex_trylock (pthread_mutex_t *mp);

extern int
pthread_mutex_destroy (pthread_mutex_t *mp);

typedef struct {
  int called;
  pthread_mutex_t lock;
} pthread_once_t;

#define PTHREAD_ONCE_INIT {0, PTHREAD_MUTEX_INITIALIZER}

extern int
pthread_once (pthread_once_t *once, void (*init_routine) (void));

typedef struct _pthread_cond *pthread_cond_t;
typedef struct _pthread_condattr *pthread_condattr_t;

#define PTHREAD_COND_INITIALIZER ((pthread_cond_t *) 1)

extern int
pthread_cond_init (pthread_cond_t *condp, pthread_condattr_t const *attr);

extern int
pthread_cond_wait (pthread_cond_t *condp, pthread_mutex_t *mutex);

extern int
pthread_cond_timed_wait (pthread_cond_t *condp, pthread_mutex_t *mutex,
			 struct timespec const abstime);

extern int
pthread_cond_signal (pthread_cond_t *condp);

extern int
pthread_cond_broadcast (pthread_cond_t *condp);

extern int
pthread_cond_destroy (pthread_cond_t *condp);


enum sched_policy { SCHED_FIFO, SCHED_RR, SCHED_OTHER };

struct sched_param {
  enum sched_policy policy;
};

enum pthread_attr_scope { PTHREAD_SCOPE_SYSTEM, PTHREAD_SCOPE_PROCESS };
enum pthread_attr_detach { PTHREAD_CREATE_DETACHED, PTHREAD_CREATE_JOINABLE };
enum pthread_attr_sched { PTHREAD_EXPLICIT_SCHED };

struct  _pthread_attr {
  enum pthread_attr_scope scope;
  enum pthread_attr_detach detach;
  void *stackaddr;
  size_t stacksize;
  struct sched_param sched;
  enum pthread_attr_sched inheritsched;
};

extern int
pthread_attr_init (pthread_attr_t *attrp);

extern int
pthread_attr_destroy (pthread_attr_t *attr);

extern int
pthread_attr_setscope (pthread_attr_t *attr,
		       enum pthread_attr_scope val);

extern int
pthread_attr_getscope (pthread_attr_t *attr,
		       enum pthread_attr_scope *valp);

extern int
pthread_attr_setdetachstate (pthread_attr_t *attr,
			     enum pthread_attr_detach val);

extern int
pthread_attr_getdetachstate (pthread_attr_t *attr,
			     enum pthread_attr_detach *valp);

extern int
pthread_attr_setstacksize (pthread_attr_t *attr,
			   size_t val);

extern int
pthread_attr_getstacksize (pthread_attr_t *attr,
			   size_t *valp);

extern int
pthread_attr_setstackaddr (pthread_attr_t *attr,
			   void *val);

extern int
pthread_attr_getstackaddr (pthread_attr_t *attr,
			   void **valp);

extern int
pthread_attr_setschedpolicy (pthread_attr_t *attr,
			     enum sched_policy val);

extern int
pthread_attr_getschedpolicy (pthread_attr_t *attr,
			     enum sched_policy *valp);

extern int
pthread_attr_setinheritsched (pthread_attr_t *attr,
			      enum pthread_attr_sched val);


extern int
pthread_attr_getinheritsched (pthread_attr_t *attr,
			      enum pthread_attr_sched *valp);


extern int
sched_yield (void);


extern int
pthread_is_pthread (void const *p);

extern void
pthread_setname (pthread_t thr, char const *name);

extern char const *
pthread_name (pthread_t thr);

#endif /* _pthr_sproc_h_ */
