#include "config.h"

#include "pthr_sproc.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ulocks.h>
#include <stdarg.h>

#include <signal.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/mman.h>

#if DEBUG_PTHREAD
static int debug_level = 0;
static int debug_threads (int level, char const *file, int lineno);
static void _debug (int level, int inc, char const *fmt, ...);
# define DBG(x) (_debug x)
# define DBGTEST(l) ((l) <= debug_level)
# define DBGLEVEL(l) (debug_level = (l))
# define DBGTHREADS(l) (DBGTEST(l) ? debug_threads(l, __FILE__, __LINE__) : 0)
#else
# define DBG(x) (0)
# define DBGTEST(l) (0)
# define DBGLEVEL(l) (l)
# define DBGTHREADS(l) (l)
#endif

static usptr_t *thread_arena;
static usptr_t *thread_arena_max;

static void (*thread_init) (void);

typedef enum {
  MON_RUN, MON_CREATE, MON_TRYLOCK, MON_SUSPEND, MON_COND_WAIT, MON_JOIN
} monitor_state;

static char const * const monitor_state_name[] = {
  "RUN", "CREATE", "TRYLOCK", "SUSPEND", "COND_WAIT", "JOIN"
};

static void thread_finish (pthread_t thr, void *status);

typedef struct cleanup_struct *cleanup_list;

typedef enum {
  THR_INIT, THR_RUNNING, THR_EXITED, THR_FREED, THR_UNSTARTED
} thread_state_t;

static char const * const thread_state_name[] = {
  "Initializing", "Running", "Exited", "Freed", "Unstarted"
};

#define thread_running(thr) (((thr)->state) < THR_EXITED)

struct _pthread {
  pthread_mutex_t lock;
  thread_state_t state;
  int pid;
  char const *name;
  void *exit_status;
  cleanup_list cleanups;
  pthread_attr_t attr;
  pthread_t joined;
  void const **specific;
  int indent_level;
  int cancelled;
  int cancelstate;
  int canceltype;
  usema_t *handshake;
  monitor_state mstate;
  void *mdata;
};

#define COMMON_SIZE (1024)
#define NSPECIFIC ((COMMON_SIZE-sizeof(pthread_t))/sizeof(void const *))

typedef struct _pthread_common {
  pthread_t self;
  void const *specific[NSPECIFIC];
} _pthread_common;

static _pthread_common *thread_common;

#define PTHREAD_SELF (thread_common->self)

pthread_t
pthread_self (void)
{
  /* Initialize if necessary. */
  if (thread_init) (*thread_init) ();

  return PTHREAD_SELF;
}

static inline int
arena_in (void *ptr)
{
  return ptr >= thread_arena && ptr <= thread_arena_max;
}

static void *
arena_malloc (size_t size)
{
  char *val = usmalloc (size, thread_arena);
  if (val && val+size > (char *) thread_arena_max) {
    thread_arena_max = val+size;
    DBG ((1, 0, "usmalloc(%d) arena %p[%d]", thread_arena,
	  ((char*)thread_arena_max) - ((char*)thread_arena)));
  }
  return val;
}

static inline void
arena_free (void *p)
{
  DBG ((1, 0, "usfree (%p) arena %p", p, thread_arena));
  usfree (p, thread_arena);
}

static usema_t *
arena_newsema (int start)
{
  usema_t *val = usnewsema (thread_arena, start);
  if (val && (void *)val > thread_arena_max) {
    thread_arena_max = (void *)val;
    DBG ((1, 0, "usnewsema(%p) arena %p[%d]", val, thread_arena,
	  ((char*)thread_arena_max) - ((char*)thread_arena)));
  }
  if (! val) {
    fprintf (stderr, "usnewsema (%p, %d) arena[%d]: %s\n",
	     thread_arena, start, 
	     ((char*)thread_arena_max) - ((char*)thread_arena),
	     sys_errlist[errno]);
    abort ();
  }
  return val;
}

static inline void
arena_freesema (usema_t *sema)
{
  DBG ((1, 0, "usfreesema (%p) arena %p", sema, thread_arena));
  usfreesema (sema, thread_arena);
}

typedef struct _list list;
static void list_enqueue (list **listp, void *value);
static void list_remove (list **listp, void *value);

struct _pthread_mutex {
  ulock_t ulock;
  pthread_t holding;
};

static list *locks = 0;
static struct _pthread_mutex locks_lock;

static void
locks_init (void)
{
  locks_lock.ulock = usnewlock (thread_arena);
}

static inline int
setlock (pthread_mutex_t lock)
{
  int ret = ussetlock (lock->ulock);
  lock->holding = PTHREAD_SELF;
  return ret;
}

static inline int
csetlock (pthread_mutex_t lock, int spins)
{
  int ret = uscsetlock (lock->ulock, spins);
  if (ret) lock->holding = PTHREAD_SELF;
  return ret;
}

static inline int
unsetlock (pthread_mutex_t lock)
{
  lock->holding = 0;
  return usunsetlock (lock->ulock);
}

static pthread_mutex_t
newlock (void)
{
  ulock_t ulock;

  pthread_mutex_t lock = usmalloc (sizeof (*lock), thread_arena);
  if (! lock) {
    fprintf (stderr, "usmalloc (%d, %p) arena[%d]: %s\n",
	     sizeof (*lock), thread_arena, 
	     ((char*)thread_arena_max) - ((char*)thread_arena),
	     sys_errlist[errno]);
    return 0;
  }

  ulock = usnewlock (thread_arena);
  if (ulock && (void *)ulock > thread_arena_max) {
    thread_arena_max = (void *)ulock;
    DBG ((1, 0, "usnewlock(%p) arena %p[%d]", ulock, thread_arena,
	  ((char*)thread_arena_max) - ((char*)thread_arena)));
  }
  if (! ulock) {
    fprintf (stderr, "usnewlock (%p) arena[%d]: %s\n",
	     thread_arena, 
	     ((char*)thread_arena_max) - ((char*)thread_arena),
	     sys_errlist[errno]);
    return 0;
  }

  lock->ulock = ulock;
  lock->holding = 0;

  setlock (&locks_lock); {
    list_enqueue (&locks, lock);
  }; unsetlock (&locks_lock);

  return lock;
}

static void
freelock (pthread_mutex_t lock)
{
  DBG ((1, 0, "usfreelock (%p) arena %p", lock, thread_arena));
  usfreelock (lock->ulock, thread_arena);
  usfree (lock, thread_arena);

  setlock (&locks_lock); {
    list_remove (&locks, lock);
  }; unsetlock (&locks_lock);
}

struct _list {
  list *next;
  void *value;
};

struct {
  struct _pthread_mutex lock;
  list *free;
} list_pool;

static void
list_init (void)
{
  list_pool.lock.ulock = usnewlock (thread_arena);
  list_pool.lock.holding = 0;
  list_pool.free = 0;
}

static list *
list_alloc (void)
{
  list *w = 0;
  setlock (&list_pool.lock); {
    /* If there are any waiting, */
    if (list_pool.free) {
      w = list_pool.free;
      list_pool.free = w->next;
    }
  }; unsetlock (&list_pool.lock);

  return w ? w : (list *) malloc (sizeof (*w));
}

static void
list_free (list *w)
{
  setlock (&list_pool.lock); {
    w->value = 0;
    w->next = list_pool.free;
    list_pool.free = w;
  }; unsetlock (&list_pool.lock);
}

static list **
list_find (list **listp, void *value)
{
  while (*listp) {
    list *w = *listp;
    if (w->value == value)
      return listp;
    listp = &w->next;
  }
  return 0;
}

static void
list_remove (list **listp, void *value)
{
  listp = list_find (listp, value);
  if (listp) {
    list *l = *listp;
    *listp = l->next;
    list_free (l);
  }
}

static void
list_enqueue (list **listp, void *value)
{
  list *w = list_alloc ();
  w->value = value;
  w->next = *listp;
  *listp = w;
}

static inline void *
list_dequeue (list **listp)
{
  list *w = *listp;
  void *value = 0;
  
  if (w) {
    /* Walk to the last one. */
    for ( ; w->next; listp = &w->next, w = *listp);
    value = w->value;
    *listp = w->next;
    list_free (w);
  } 
  return value;
}

#if DEBUG_PTHREAD
struct _pthread_mutex dbg_lock;

static inline void
report_start (void)
{
  if (thread_init) (*thread_init) ();
  if (! dbg_lock.ulock) dbg_lock.ulock = usnewlock (thread_arena);
  setlock (&dbg_lock);
}

static inline void
report_end (void)
{
  unsetlock (&dbg_lock);
}

static void
report_print (FILE *fp, char const *buf, int newline)
{
  static int last_newline = 1;

  report_start (); {
    if (newline && !last_newline) putc ('\n', fp);
    fputs (buf, fp);
    if (newline) putc ('\n', fp);
    fflush (fp);
    last_newline = newline;
  }; report_end ();
}

int
__pthread_debug (int new_level)
{
  int old = debug_level;
  debug_level = new_level;
  return old;
}
#endif

void
pthread_setname (pthread_t thr, char const *name)
{
  int len = strlen (name) + 1;
  char *thr_name = malloc (len);
  memcpy (thr_name, name, len);
  thr->name = thr_name;
}

char const *
pthread_name (pthread_t thr)
{
  return thr->name;
}

int
pthread_mutex_init (pthread_mutex_t *mpp,
		    pthread_mutex_attr_t const *attr)
{
  pthread_mutex_t mp;

  /* Initialize if necessary. */
  if (thread_init) (*thread_init) ();

  if (! mpp) {
    DBG ((0, 0, "pthread_mutex_init (%p): NULL pointer <= EINVAL", mpp));
    return EINVAL;
  }
  mp = *mpp;
  if (mp && mp != PTHREAD_MUTEX_INITIALIZER && arena_in (mp)) {
    DBG ((0, 0, "pthread_mutex_init (&%p): reinit attempted", mp));
    return EBUSY;
  }

  *mpp = mp = newlock ();
  if (! mp) {
    DBG ((0, 0, "pthread_mutex_init: malloc(%d) failed <= ENOMEM",
	  sizeof (*mp)));
    return ENOMEM;
  }

  DBG ((1, 0, "pthread_mutex_init (%p) <= %d",
	mp, (! mp ? errno : 0)));

  return (! mp ? errno : 0);
}

int
pthread_mutex_lock (pthread_mutex_t *mpp)
{
  pthread_mutex_t mp;
  pthread_t self;
  int ret;

  if (! mpp) {
    DBG ((0, 0, "pthread_mutex_lock (%p): NULL pointer <= EINVAL", mpp));
    return EINVAL;
  }
  mp = *mpp;

  /* Initialize if necessary. */
  if (thread_init) (*thread_init) ();

  /* If lock uninitialized, initialize it. */
  if (mp == PTHREAD_MUTEX_INITIALIZER) {
    pthread_mutex_init (mpp, NULL);
    mp = *mpp;
  }
  if (! mp || ! arena_in (mp)) {
    DBG ((0, 0, "pthread_mutex_lock (&%p): not in arena <= EINVAL", mp));
    return EINVAL;
  }
 
  DBG ((2, 0, "pthread_mutex_lock (&%p) trylock", mp));

  self = PTHREAD_SELF;
  if (mp->holding == self) {
    DBG ((0, 0, "pthread_mutex_lock (&%p): already held by %p <= EDEADLK", 
	  mp, self));
    abort ();
    return EDEADLK;
  }

  self->mstate = MON_TRYLOCK;
  self->mdata = mp;

  ret = setlock (mp);

  self->mstate = MON_RUN;
  
  DBG ((2, 0, "pthread_mutex_lock (%p) locked <= %d",
	mp, (ret == -1 ? errno : 0)));
  return (ret == -1 ? errno : 0);
}

int
pthread_mutex_unlock (pthread_mutex_t *mpp)
{
  pthread_mutex_t mp;
  int ret;

  if (! mpp) {
    DBG ((0, 0, "pthread_mutex_unlock (%p): NULL pointer <= EINVAL", mpp));
    return EINVAL;
  }
  mp = *mpp;
  if (! mp || ! arena_in (mp)) {
    DBG ((0, 0, "pthread_mutex_unlock (&%p): not in arena <= EINVAL", mp));
    return EINVAL;
  }

  if (! mp->holding) {
    DBG ((0, 0, "pthread_mutex_unlock (&%p): not held <= EINVAL", mp));
    return EINVAL;
  }
  if (mp->holding != PTHREAD_SELF) {
    DBG ((0, 0, "pthread_mutex_unlock (&%p): held by (%d, %s) <= EINVAL", 
	  mp, mp->holding->pid, pthread_name (mp->holding)));
    return EINVAL;
  }

  ret = unsetlock (mp);

  DBG ((2, 0, "pthread_mutex_unlock (%p) <= %d", mp, 
	(ret == -1 ? errno : 0)));
  return (ret == -1 ? errno : 0);
}

int
pthread_mutex_trylock (pthread_mutex_t *mpp)
{
  pthread_mutex_t mp;
  int avail;

  if (! mpp) {
    DBG ((0, 0, "pthread_mutex_trylock (%p): NULL pointer <= EINVAL", mpp));
    return EINVAL;
  }
  mp = *mpp;

  /* Initialize if necessary. */
  if (thread_init) (*thread_init) ();

  /* If lock uninitialized, initialize it. */
  if (mp == PTHREAD_MUTEX_INITIALIZER) {
    pthread_mutex_init (mpp, NULL);
    mp = *mpp;
  }
  if (! mp || ! arena_in (mp)) {
    DBG ((0, 0, "pthread_mutex_trylock (&%p): not in arena <= EINVAL", mp));
    return EINVAL;
  }
 
  avail = csetlock (mp, 16);

  DBG ((2, 0, "pthread_mutex_trylock (%p) <= %d",
	mp, (avail ? 0 : EBUSY)));
  return (avail ? 0 : EBUSY);
}

int
pthread_mutex_destroy (pthread_mutex_t *mpp)
{
  pthread_mutex_t mp;

  if (! mpp) {
    DBG ((0, 0, "pthread_mutex_destroy (%p): NULL pointer <= EINVAL", mpp));
    return EINVAL;
  }
  mp = *mpp;
  if (! mp || ! arena_in (mp)) {
    DBG ((0, 0, "pthread_mutex_destroy (&%p): not in arena <= EINVAL", mp));
    return EINVAL;
  }

  if (mp->holding) {
    DBG ((0, 0, "pthread_mutex_destroy (&%p): thread (%d, %s) <= EBUSY", 
	  mp, mp->holding->pid, pthread_name (mp->holding)));
    return EBUSY;
  }

  DBG ((2, 0, "pthread_mutex_destroy (%p)", mp));
  freelock (mp);

  *mpp = 0;

  return 0;
}

int
pthread_once (pthread_once_t *once, void (*init_func) (void))
{
  if (! once->called) {
    pthread_mutex_lock (&once->lock); {
      if (! once->called) {
	(*init_func) ();
	once->called = 1;
      }
    }; pthread_mutex_unlock (&once->lock);
  }
  return 0;
}

static const int init_table_len = 37;

static int nthreads = 0;
static int thr_table_len;
static pthread_t *thr_table = 0;
static pthread_t thr_main;
static pthread_t thr_manager;
static pthread_mutex_t table_mutex;

static int nspecific = 0;
static int len_specific = 8;

typedef void (*destructor)(void *);
static destructor *spec_destructor;

static pthread_t thr_locked = 0;
static pthread_t thread_find (pid_t pid);
static pthread_t *thread_slot (pid_t pid);

#if DEBUG_PTHREAD
static void
_debug (int level, int inc, char const *fmt, ...)
{
  va_list ap;
  static int _indent_level = 0;

  if (DBGTEST (level)) {
    pthread_t self = (nthreads ? PTHREAD_SELF : 0);
    int *indent_level = (self ? &self->indent_level : &_indent_level);
    char buf[1024], *p;
    int indent;

    if (inc < 0) *indent_level += inc;
    indent = *indent_level;

    if (indent < 0)
      fprintf (stderr, "[%d: indent_level < 0]\n", getpid ());
    if (fmt[0]) {
      if (self && pthread_name (self))
	sprintf (buf, "%5d %p %s: ", self->pid, self, pthread_name (self));
      else if (self)
	sprintf (buf, "%5d %p: ", self->pid, self);
      else
	sprintf (buf, "%5d: ", getpid ());
      p = strchr (buf, '\0');
      if (indent > 32) indent = 32;
      for ( ; indent >= 0; --indent) *p++ = ' ';
      va_start (ap, fmt);
      vsprintf (p, fmt, ap);
      va_end (ap);
    }
    if (inc > 0) *indent_level += inc;

    report_print (stderr, buf, 1);
  }
}
#endif

static pthread_t
thread_new (void)
{
  pthread_t thr = (pthread_t) calloc (1, sizeof (*thr));

  thr->state = THR_INIT;
  thr->lock = newlock ();
  thr->specific = (void const **)
    calloc (len_specific, sizeof (*thr->specific));
  thr->handshake = arena_newsema (0);

  thr->cancelstate = PTHREAD_CANCEL_ENABLE;
#if 0
  thr->canceltype = PTHREAD_CANCEL_DEFERRED;
#else
  thr->canceltype = PTHREAD_CANCEL_ASYNCHRONOUS;
#endif

  return thr;
}

static int
thread_lock (pthread_t thr)
{
  int ret;
  DBG ((2, 1, "thread_lock (%d, %s) {", thr->pid, pthread_name (thr)));
  return setlock (thr->lock);
}

static int
thread_unlock (pthread_t thr)
{
  DBG ((2, -1, "} thread_unlock (%d, %s)", thr->pid, pthread_name (thr)));
  return unsetlock (thr->lock);
}

static void
thread_suspend (pthread_t thr, monitor_state mstate)
{
  DBGTHREADS (4);
  DBG ((1, 0, "suspend thread (%d, %s)", thr->pid, pthread_name (thr)));
  thr->mstate = mstate;
  uspsema (thr->handshake);
  DBG ((1, 0, "continue thread (%d, %s)", thr->pid, pthread_name (thr)));
  thr->mstate = MON_RUN;
  DBGTHREADS (4);
}

static void
thread_resume (pthread_t thr)
{
  DBGTHREADS (4);
  DBG ((1, 0, "resume thread (%d, %s)", thr->pid, pthread_name (thr)));
  usvsema (thr->handshake);
  DBGTHREADS (4);
}

static int
table_lock (void)
{
  int ret;
  
  if (DBGTEST (4)) fprintf (stderr, "%5d: try table_lock()\n", getpid ());
  ret = setlock (table_mutex);
  if (DEBUG_PTHREAD) thr_locked = PTHREAD_SELF;
  if (DBGTEST (4)) fprintf (stderr, "%5d: table_lock() {\n", getpid ());
  return ret;
}

static int
table_unlock (void)
{
  if (DEBUG_PTHREAD) thr_locked = 0;
  if (DBGTEST (4)) fprintf (stderr, "%5d: } table_unlock()\n", getpid ());
  return unsetlock (table_mutex);
}

static void
thread_register (pthread_t thr, pid_t pid)
{
  thread_common->self = thr;

  table_lock (); {
    pthread_t *slot;

    thr->pid = pid;
  
    if (nthreads > thr_table_len / 2) {
      /* If table over half full, then extend it. */
      pthread_t *old_table = thr_table, *old_end = old_table + thr_table_len;

      /* Extend the table. */
      thr_table_len *= 2;
      thr_table_len -= 1;
      thr_table = (pthread_t *) calloc (thr_table_len, sizeof (pthread_t));

      /* Now reinsert all old threads into new table. */
      for ( ; old_table < old_end; ++old_table) {
	pthread_t thr_moving = *old_table;
	if (thr_moving) {
	  slot = thread_slot (thr_moving->pid);
	  *slot = thr_moving;
	}
      }

      /* And free the old table. */
      free (old_table);
    }

    /* Find an empty slot and enter the new thread. */
    slot = thread_slot (pid);
    DBG ((3, 0, "thread_slot[%d] = %p", slot-thr_table, *slot));
    if (*slot) {
      fprintf (stderr, "PID %d already registered.\n", pid);
      abort ();
    }

    *slot = thr;
    ++nthreads;
  }; table_unlock ();

  DBG ((1, 0, "thread_register(%p, %d).", thr, thr->pid));
}

static void
thread_testcancel (pthread_t thread)
{
  DBG ((1, 0, "thread_testcancel (%p, %d) cancelled %d", 
	thread, thread->pid, thread->cancelled));
  if (thread->cancelstate == PTHREAD_CANCEL_ENABLE
      && thread->cancelled) {
    thread_finish (thread, PTHREAD_CANCELED);
    exit (0);
  }
}

int
pthread_cancel (pthread_t thread)
{
  DBG ((1, 0, "pthread_cancel (%p, %d) %s %s",
	thread, thread->pid,
	thread->cancelstate ? "enabled" : "disabled",
	(thread->canceltype == PTHREAD_CANCEL_DEFERRED ? "deferred"
	 : thread->canceltype == PTHREAD_CANCEL_ASYNCHRONOUS ? "async"
	 : "????")));
  if (thread->cancelstate == PTHREAD_CANCEL_ENABLE) {
    thread->cancelled = 1;
    if (thread->canceltype == PTHREAD_CANCEL_ASYNCHRONOUS
	|| thread->mstate == MON_TRYLOCK
	|| thread->mstate == MON_SUSPEND
	|| thread->mstate == MON_COND_WAIT)
      pthread_kill (thread, SIGUSR1);
  }
  return 0;
}

static void
thread_cancel_async (pthread_t thread)
{
  sigset_t set;

  sigemptyset (&set);
  sigaddset (&set, SIGUSR1);
  sigprocmask (SIG_UNBLOCK, &set, NULL);
  DBG ((1, 0, "thread_cancel_async: unblock SIGUSR1"));
}

/* We want to enable the async signal in either case, because we may need to
   interrupt certain states even with deferred cancellation. */
#define thread_cancel_sync thread_cancel_async

static int
thread_setcancelstate (pthread_t thread, int state, int *oldstate)
{
  if (! thread)
    return ESRCH;
  
  if (state < PTHREAD_CANCEL_DISABLE
      || state > PTHREAD_CANCEL_ENABLE)
    return EINVAL;

  if (thread->cancelstate != state) {
    *oldstate = thread->cancelstate;
    thread->cancelstate = state;

    DBG ((1, 0, "thread_setcancelstate (%s)", 
	  (state == PTHREAD_CANCEL_DISABLE ? "disable"
	   : state == PTHREAD_CANCEL_ENABLE ? "enable"
	   : "???")));

    if (state == PTHREAD_CANCEL_ENABLE) {
      thread_testcancel (thread);
      if (thread->canceltype == PTHREAD_CANCEL_ASYNCHRONOUS) {
	thread_cancel_async (thread);
      } else {
	thread_cancel_sync (thread);
      }
    }
  }
  
  return 0;
}

int
pthread_setcancelstate (int state, int *oldstate)
{
  return thread_setcancelstate (pthread_self (), state, oldstate);
}

static int
thread_setcanceltype (pthread_t thread, int ctype, int *oldctype)
{
  if (! thread)
    return ESRCH;
  
  if (ctype < PTHREAD_CANCEL_DEFERRED
      || ctype > PTHREAD_CANCEL_ASYNCHRONOUS)
    return EINVAL;

  if (thread->canceltype != ctype) {
    *oldctype = thread->canceltype;
    thread->canceltype = ctype;

    DBG ((1, 0, "thread_setcanceltype (%s)", 
	  (ctype == PTHREAD_CANCEL_ASYNCHRONOUS ? "async"
	   : ctype == PTHREAD_CANCEL_DEFERRED ? "deferred"
	   : "???")));

    if (thread->cancelstate == PTHREAD_CANCEL_ENABLE) {
      if (ctype == PTHREAD_CANCEL_ASYNCHRONOUS) {
	thread_cancel_async (thread);
      } else {
	thread_cancel_sync (thread);
      }
    }
  }
  
  return 0;
}

int
pthread_setcanceltype (int ctype, int *oldctype)
{
  return thread_setcanceltype (pthread_self (), ctype, oldctype);
}

void
pthread_testcancel (void)
{
  thread_testcancel (pthread_self ());
}

struct cleanup_struct {
  cleanup_list tail;
  void (*func) (void *);
  void *closure;
};

static void
thread_cleanup (pthread_t thr)
{
  cleanup_list cleanup = thr->cleanups;

  DBG ((1, 1, "thread_cleanup(%d, %s):",
	thr->pid,  pthread_name (thr)));

  while (cleanup) {
    cleanup_list tail = cleanup->tail;
    thread_unlock (thr);
    DBG ((1, 0, "cleanup (%s) call func %p",
	  pthread_name (thr), cleanup->func));
    (*cleanup->func) (cleanup->closure);
    thread_lock (thr);
    free (cleanup);
    thr->cleanups = cleanup = tail;
  }
  thr->cleanups = 0;

  DBG ((1, -1, "thread_cleanup(%d, %s): done.", thr->pid, pthread_name (thr)));
}

static void
thread_push_cleanup (pthread_t thr, void (*func) (void *), void *closure)
{
  cleanup_list entry = (cleanup_list) malloc (sizeof (*entry));
  entry->func = func;
  entry->closure = closure;
  thread_lock (thr); {
    cleanup_list cleanup = thr->cleanups;
    entry->tail = cleanup;
    thr->cleanups = entry;
  }; thread_unlock (thr);
}

static void
thread_pop_cleanup (pthread_t thr, int execute)
{
  thread_lock (thr); {
    cleanup_list cleanup = thr->cleanups;
    if (cleanup) {
      thr->cleanups = cleanup->tail;
      if (execute) {
	thread_unlock (thr);
	(*cleanup->func) (cleanup->closure);
	thread_lock (thr);
      }
      free (cleanup);
    }
  }; thread_unlock (thr);
}

static void
thread_free (pthread_t thr)
{
  thread_lock (thr); {
    cleanup_list cleanup = thr->cleanups;
    while (cleanup) {
      cleanup_list tail = cleanup->tail;
      free (cleanup);
      cleanup = tail;
    }

    arena_freesema (thr->handshake);

    thr->state = THR_FREED;
  }; thread_unlock (thr);

  freelock (thr->lock);
}

static void
thread_delete (pthread_t thr)
{
  pthread_t *slot;

  table_lock (); {
    slot = thread_slot (thr->pid);
    *slot = 0;
    thread_free (thr);
  }; table_unlock ();
}

static int
thread_hash (pid_t pid)
{
  return (pid + ((pid << 6) ^ 0x0f0f0f0f)) % thr_table_len;
}

/*
 * Returns either a pointer to the slot that refers to the thread or an
 * empty slot.  Assumes that table_lock is held.
 */
static pthread_t *
thread_slot (pid_t pid)
{
  pthread_t *thr_end = thr_table + thr_table_len;
  pthread_t *curr = &thr_table[thread_hash (pid)];

  DBG ((3, 0, "thread_slot[%d] = %p", curr-thr_table, *curr));
  while (*curr != 0) {
    DBG ((3, 0, "thread_slot[%d] = %p", curr-thr_table, *curr));
    if ((*curr)->pid == pid)
      break;
    if (++curr >= thr_end)
      curr = thr_table;
  }

  return curr;
}

static pthread_t
thread_find (pid_t pid)
{
  pthread_t thr;
  table_lock (); {
    thr = *thread_slot (pid);
  }; table_unlock ();
  return thr;
}

int
pthread_is_pthread (void const *p)
{
  int ret = 0;

  table_lock (); {
    pthread_t *thr_end = thr_table + thr_table_len;
    pthread_t *curr = thr_table;
    for ( ; curr < thr_end; ++curr)
      if (*curr == (pthread_t) p) {
	ret = 1;
	break;
      }
  }; table_unlock ();

  return ret;
}

static void
mutex_describe_ (pthread_mutex_t mp)
{
  fprintf (stderr, "MUTEX %p", mp);
  if (mp->holding) 
    fprintf (stderr, " HELD %d", mp->holding->pid);
}

static void cond_describe_ (pthread_cond_t cond);

static void
locks_describe_ (char const *prefix)
{
  setlock (&locks_lock); {
    list *l = locks;
    for ( ; l; l = l->next) {
      pthread_mutex_t lock = l->value;
      if (lock->holding) {
	fprintf (stderr, "%sMUTEX %p HELD %d\n",
		 prefix, lock, lock->holding->pid);
      }
    }
  }; unsetlock (&locks_lock);
}

static void
locks_describe (void)
{
  locks_describe_ ("");
}

static void
thread_describe_ (char const *prefix, pthread_t thr)
{
  fprintf (stderr, "%sthread (%d, %s):\n",
	   prefix, thr->pid, pthread_name (thr));
  if (thr->state != THR_RUNNING) {
    fprintf (stderr, "%s  State %s", prefix, thread_state_name[thr->state]);
    if (! thread_running (thr))
      fprintf (stderr, ": status %p", thr->exit_status);
    fprintf (stderr, "\n");
  } else {
    fprintf (stderr, "%s  ", prefix);
    switch (thr->mstate) {
    case MON_RUN:
      fprintf (stderr, "RUNNING");
      break;
    case MON_CREATE:
      fprintf (stderr, "CREATING thread");
      break;
    case MON_TRYLOCK:
      fprintf (stderr, "TRYLOCK ");
      mutex_describe_ (thr->mdata);
      break;
    case MON_SUSPEND:
      fprintf (stderr, "SUSPENDED");
      break;
    case MON_COND_WAIT:
      fprintf (stderr, "COND_WAIT ");
      cond_describe_ (thr->mdata);
      break;
    case MON_JOIN: {
      pthread_t join = thr->mdata;
      fprintf (stderr, "JOIN thread (%d, %s)",
	       join->pid, pthread_name (join));
      break;
    }
    }
    fprintf (stderr, "\n");
  }
}

static void
thread_describe (pthread_t thr)
{
  thread_describe_ ("", thr);
}

static void
thread_describe_all_ (char const *prefix)
{
  table_lock (); {
    pthread_t *thr_end = thr_table + thr_table_len;
    pthread_t *curr = thr_table;
    for ( ; curr < thr_end; ++curr)
      if (*curr) thread_describe_ (prefix, *curr);
  }; table_unlock ();
}

static void
thread_describe_all (void)
{
  thread_describe_all_ ("");
}

#if DEBUG_PTHREAD
static int
debug_threads (int level, char const *file, int lineno)
{
  _debug (level, 1, "%s:%d:", file, lineno);
  if (thr_locked)
    _debug (level, 0, "Hmmm... table locked by (%d %s)",
	    thr_locked->pid, pthread_name (thr_locked));
  table_lock (); {
    pthread_t *thr_end = thr_table + thr_table_len;
    pthread_t *curr = thr_table;
    for ( ; curr < thr_end; ++curr)
      if (*curr)
	_debug (level, 0, "thread (%d, %s)", 
		curr[0]->pid, pthread_name (curr[0]));
  }; table_unlock ();
  _debug (level, -1, "");
  return 1;
}
#endif


typedef struct {
  void (*op) (void*);
  void *arg;
} manager_entry_t;

static int manager_send;
static int manager_recv;

static void
manager_queue_op (void (*op) (void*), void *arg)
{
  manager_entry_t entry;

  entry.op = op;
  entry.arg = arg;

  DBG ((2, 0, "queue entry {op = %p, arg = %p}", op, arg));
  write (manager_send, &entry, sizeof (entry));
}

static void
manager_cancelall (void)
{
  table_lock (); {
    int i;
    for (i = 0; i < thr_table_len; ++i) {
      pthread_t thread = thr_table[i];
      if (thread && thread_running (thread)) {
	DBG ((1, 0, "cancel thread (%d, %s)",
	      thread->pid, pthread_name (thread)));
	thread->cancelled = 1;
	kill (thread->pid, SIGUSR1);
      }
    }
  }; table_unlock ();
}

static void
manager_handler (int sig)
{
  pid_t pid;
  int status;

  DBG ((1, 1, "manager_handler(sig = %d):", sig));

  switch (sig) {
  case SIGCHLD:
    /* Some thread has exited. */
    DBG ((1, 0, "received SIGCHLD"));
    pid = waitpid(-1, &status, WNOHANG);
    while (pid > 0) {
      pthread_t thread = thread_find (pid);
      --nthreads;
      if (thread) {
	DBG ((1, 0, "child %d state %d status %d. Now %d left", 
	      (int) pid, thread->state, status, nthreads));
	if (thread_running (thread))
	  thread_finish (thread, NULL);
      } else {
	fprintf (stderr, "SIGCHLD received from PID %d.  Not a thread?\n", pid);
      } 
      pid = waitpid(-1, &status, WNOHANG);
    }

    /* Check if signal was sent by main process. */
    if (errno == ECHILD && getppid () == 1) {
      DBG ((1, -1, "no more children. Exit."));
      exit (0);
    }
    break;

  case SIGTERM:
    /* Cause all threads to exit. */
    DBG ((1, 0, "received SIGTERM"));
    DBGTHREADS(1);
    manager_cancelall ();
    break;

  case SIGHUP:
    thread_describe_all ();
    locks_describe ();
    break;
  }

  DBG ((1, -1, "manager_handler(): done."));
}

static void
manager_thread (void *arg)
{
  struct sigaction act;
  sigset_t set, emptyset;

  DBG ((1, 0, "manager_thread ():"));

  /* Register as a thread and allow main to continue. */
  DBGTHREADS(4);
  pthread_attr_init (&thr_manager->attr);
  thr_manager->attr.detach = PTHREAD_CREATE_DETACHED;
  thread_register (thr_manager, getpid ());
  pthread_setname (thr_manager, "_manager");
  thr_manager->state = THR_RUNNING;
  thr_manager->mstate = MON_RUN;
  DBGTHREADS(4);

  unblockproc (thr_main->pid);

  /* Setup signal handlers. */
  act.sa_handler = manager_handler;
  sigemptyset (&act.sa_mask);
  act.sa_flags = SA_RESTART;
  sigaction (SIGCHLD, &act, NULL);
  sigaction (SIGTERM, &act, NULL);
  sigaction (SIGHUP, &act, NULL);

  /* Block all signals except SIGCHLD and SIGBUS. */
  sigfillset(&set);
  sigdelset(&set, SIGCHLD);
  sigdelset(&set, SIGTERM);
  sigdelset(&set, SIGHUP);
  sigprocmask(SIG_SETMASK, &set, NULL);
  sigemptyset(&emptyset);

  /* Handle op requests, blocking when none available. */
  for (;;) {
    manager_entry_t entry;
    struct timeval timeout;
    fd_set fdset;

    /* Set up select for single fd and 100msec timeout. */
    FD_ZERO (&fdset);
    FD_SET (manager_recv, &fdset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    if (select (manager_recv+1, &fdset, NULL, NULL, &timeout) == 1) {
      /* Wait for request to come in on pipe. */
      DBG ((1, 0, "read queue"));
      thr_manager->mstate = MON_SUSPEND;
      if (read (manager_recv, &entry, sizeof (entry)) < 0) {
	fprintf (stderr, "Thread manager request read failed. %s\n",
		 strerror (errno));
	exit (1);
      }
      thr_manager->mstate = MON_RUN;

      DBG ((2, 0, "entry {func = %p, arg = %p}", entry.op, entry.arg));
      (*entry.op) (entry.arg);
    }

    /* Check for termination of the main thread */
    if (getppid() == 1) {
      DBG ((1, 0, "Main process exited."));
      manager_cancelall ();

      close (manager_send);
      close (manager_recv);

      return;
    }
  }
}


/* void thread_init_ (void):

   Initialize the data structures for the threads library and start the thread
   manager thread.

   This thread_init trick might look a little like a potential data race, but
   we know that thread_init_() is always called while there is only a single
   active thread. */

static void exit_main (void);

static void
thread_init_ (void)
{
  pthread_t self;

  /* Ensure that this can only be called once. */
  thread_init = NULL;

  {
    int fd = open ("/dev/zero");
    thread_common = mmap (0, COMMON_SIZE, 
			  PROT_READ | PROT_WRITE,
			  MAP_LOCAL | MAP_PRIVATE | MAP_AUTORESRV,
			  fd, 0);
    close (fd);
  }
			
  usconfig (CONF_LOCKTYPE, US_DEBUG);
  usconfig (CONF_ARENATYPE, US_SHAREDONLY);
  usconfig (CONF_INITUSERS, 64);
  usconfig (CONF_INITSIZE, 1<<20);
  usconfig (CONF_AUTOGROW, 1);
  /*usconfig (CONF_AUTORESV, 1);*/
  thread_arena = usinit ("/dev/zero");
  if (! thread_arena) {
    fprintf (stderr, "Unable to create arena. %s\n", strerror (errno));
    exit (1);
  }
  usconfig (CONF_HISTOFF, thread_arena);

  DBG ((1, 0, "thread_init(): arena %p common %p",
	thread_arena, thread_common));

  /* Set the maximum stack size. */
  prctl (PR_SETSTACKSIZE, 1<<20);

  /* Initialize the basic management structures */
  locks_init ();
  list_init ();

  /* If not previously initialized, allocate table. */
  thr_table_len = init_table_len;
  thr_table = (pthread_t *) calloc (thr_table_len, sizeof (pthread_t));
  spec_destructor =
    (destructor*) calloc (len_specific, sizeof(destructor));

  /* Allocate the pid->thread table lock. */
  table_mutex = newlock ();

  /* Register the parent process. */
  thr_main = thread_new ();
  thr_main->mstate = MON_RUN;
  pthread_attr_init (&thr_main->attr);
  thread_register (thr_main, getpid ());
  pthread_setname (thr_main, "_main");
  thr_main->state = THR_RUNNING;
  DBGTHREADS (4);

  /* And start the thread manager. */
  thr_manager = thread_new ();
  if (! thr_manager) {
    fprintf (stderr, "pthreads: Unable to initialize manager thread.\n");
    abort ();
  }

  /* This is the basic mechanism for sending requests to the manager. */
  {
    int fds[2];
    if (! pipe(fds)) {
      manager_send = fds[1];
      manager_recv = fds[0];
    } else {
      fprintf (stderr, "pthreads: Unable to start manager thread.\n");
      abort ();
    }
  }

  DBG ((1, 0, "start manager_thread(): suspend pid %d", getpid ()));
  self = PTHREAD_SELF;
  self->mstate = MON_CREATE;
  self->mdata = thr_manager;

  sproc (manager_thread, PR_SALL|PR_BLOCK, 0);
  self->mstate = MON_RUN;
  
  /* Ensure that main thread is treated as a thread on exit. */
  atexit (exit_main);

  DBG ((1, 0, "thread_init(): done."));
}

static void
exit_main (void)
{
  thread_finish (PTHREAD_SELF, 0);
}

static void (*thread_init) (void) = thread_init_;

typedef struct {
  void *(*start_func) (void*);
  void *arg;
  pthread_t parent;
  pthread_t thread;
} sproc_arg;

static void sproc_entry (void *arg);

static void
manager_sproc (void *arg)
{
  sproc_arg *spr_arg = (sproc_arg *) arg;
  pthread_t thread = spr_arg->thread;
  pthread_attr_t *attr = &thread->attr;

  DBG ((1,  1, "manager_sproc(): stacksize %d", attr->stacksize));
  prctl (PR_SETSTACKSIZE, attr->stacksize);
  DBG ((1,  0, "block pid %d", getpid ()));

  thr_manager->mstate = MON_CREATE;
  thr_manager->mdata = thread;

  if (sproc (sproc_entry, PR_SALL|PR_BLOCK, arg) < 0) {
    fprintf (stderr, "sproc failed: %s", sys_errlist[errno]);

    /* Finish up the thread. */
    thread->state = THR_UNSTARTED;
    thread->exit_status = (void *) oserror ();

    /* Allow parent process to continue. */
    usvsema (spr_arg->parent->handshake);
  }

  thr_manager->mstate = MON_RUN;
  DBG ((1, -1, "manager_sproc(): leave"));
}

int
pthread_create (pthread_t *new_thread_id,
		const pthread_attr_t *attr,
		void *(*start_func) (void*),
		void *arg)
{
  sproc_arg spr_arg;
  pthread_t thread;
  pthread_t self;

  /* Initialize if necessary. */
  if (thread_init) (*thread_init) ();

  DBG ((1, 1, "pthread_create():"));
  DBGTHREADS (4);

  /* Allocate and initialize the new thread structure. */
  thread = thread_new ();
  DBG ((1, 0, "allocate thread %p", thread));
  DBGTHREADS (4);

  if (attr)
    thread->attr = *attr;
  else
    pthread_attr_init (&thread->attr);

  /* Pass (start_func,arg) into the sproc. */
  spr_arg.start_func = start_func;
  spr_arg.arg = arg;
  spr_arg.thread = thread;
  spr_arg.parent = self = PTHREAD_SELF;
  DBG ((1, 0, "sproc_arg %p = {func=%p, arg=%p, thread=%p, parent=%d}",
	&spr_arg, spr_arg.start_func, spr_arg.arg, spr_arg.thread, spr_arg.parent));

  /* Ask the manager to create the thread. */
  manager_queue_op (manager_sproc, &spr_arg);

  /* Wait for thread to be initialized. */
  self->mstate = MON_SUSPEND;
  uspsema (self->handshake);
  self->mstate = MON_RUN;

  *new_thread_id = thread;
  if (thread->state == THR_UNSTARTED) {
    int status = (int) thread->exit_status;
    thread_delete (thread);
    DBG ((1, -1, "pthread_create(): failure <= %d", status));
    return status;
  }

  DBG ((1, -1, "pthread_create(): thread_id = %p", thread));
  return 0;
}

static void cond_remove_wait (pthread_cond_t cond, pthread_t thr);

static void
thread_release (pthread_t thr)
{
  if (dbg_lock.holding == thr) {
    unsetlock (&dbg_lock);
    DBG ((1, 0, "thread_release(%d, %d) release dbg_lock",
	  thr->pid, pthread_name (thr)));
  }

  if (list_pool.lock.holding == thr) {
    DBG ((1, 0, "thread_release(%d, %d) release list_pool.lock",
	  thr->pid, pthread_name (thr)));
    unsetlock (&list_pool.lock);
  }

  {
    list *l = locks;
    for ( ; l; l = l->next) {
      pthread_mutex_t lock = l->value;
      if (lock->holding == thr) {
	DBG ((1, 0, "thread_release(%d, %d) release lock %p",
	      thr->pid, pthread_name (thr), lock));
	unsetlock (lock);
      }
    }
  }
}

static void
thread_finish (pthread_t thr, void *status)
{
  /* Clean up lock (possibly including thr->lock) */
  thread_release (thr);

  thread_lock (thr); {
    if (thread_running (thr)) {
      thr->state = THR_EXITED;

      DBG ((1, 1, "thread_finish(%d, %s) <= %p:",
	    thr->pid, pthread_name (thr), status));

      /* Process pending cleanups. */
      thread_cleanup (thr);

      DBG ((1, 1, "thread_finish(%d, %s) %s %p",
	    thr->pid, pthread_name (thr),
	    monitor_state_name[thr->mstate], thr->mdata));
      /* We may need to remove this thread from cond_wait list. */
      if (thr->mstate == MON_COND_WAIT)
	cond_remove_wait ((pthread_cond_t) thr->mdata, thr);

      /* Update exit_status. */
      thr->exit_status = status;
    }
  }; thread_unlock (thr);

  if (thr->attr.detach == PTHREAD_CREATE_DETACHED)
    thread_free (thr);
  else if (thr->joined)
    thread_resume (thr->joined);

  DBG ((1, -1, "thread_finish(%s, %p): exit", pthread_name (thr), status));
}

static void
thread_term (int sig)
{
  pthread_t thread = PTHREAD_SELF;

  if (thread->cancelled) {
    /* Finish up the thread. */
    thread_finish (thread, PTHREAD_CANCELED);
    DBG ((1, 0, "exit"));
    exit (0);
  }
}

static void
sproc_entry (void *arg)
{
  sproc_arg *spr_arg = (sproc_arg *) arg;
  void *(*pthread_func) (void*) = spr_arg->start_func;
  void *pthread_arg = spr_arg->arg;
  pthread_t thread = spr_arg->thread;
  pthread_t parent = spr_arg->parent;

  void *status;

  struct sigaction act;
  sigset_t set;

  DBG ((2, 1, "sproc_entry(%p):", arg));
  DBG ((2, 0, "sproc_arg %p = {func=%p, arg=%p, thread=%p, parent=%d}",
	spr_arg, pthread_func, pthread_arg, thread, parent));

  /* Must properly initialize this thread. */
  thread_register (thread, getpid ());

  /* Setup SIGUSR1 handler. */
  sigemptyset (&act.sa_mask);
  act.sa_flags = SA_RESTART;
  act.sa_handler = thread_term;
  sigaction (SIGUSR1, &act, NULL);

  sigemptyset (&set);
  sigaddset (&set, SIGUSR1);
  sigprocmask (SIG_BLOCK, &set, NULL);

  sigemptyset (&set);
  sigaddset (&set, SIGSEGV);
  sigaddset (&set, SIGBUS);
  sigaddset (&set, SIGIOT);
  sigprocmask (SIG_UNBLOCK, &set, NULL);

  /* Initialize the cancelstate. */
  {
    int state = thread->cancelstate;
    thread->cancelstate = PTHREAD_CANCEL_DISABLE;
    thread_setcancelstate (thread, state, &state);
  }

  /* Assert that the thread is now running. */
  thread->state = THR_RUNNING;
  thread->mstate = MON_RUN;

  /* Allow manager to continue. */
  unblockproc (thr_manager->pid);

  /* Allow parent process to continue. */
  if (parent)
    usvsema (parent->handshake);

  /* Call the start_func for the new thread. */
  DBG ((2, 0, "call func %p arg %p", pthread_func, pthread_arg));
  status = (*pthread_func) (pthread_arg);
  DBG ((1, 0, "func %p exited with status %p", pthread_func, status));

  /* Finish up the thread. */
  thread_finish (thread, status);
  DBG ((2, 0, "exit"));
  exit (0);
}

NOEXIT_FUNC void
pthread_exit (void *status)
{
  thread_finish (PTHREAD_SELF, status);
}

static int
pthread_not_joinable (pthread_t thread)
{
  if (! thread) {
    DBG ((0, 0, "pthread_join(%p): bad thread", thread));
    return ESRCH;
  }

  /* Can't join detached procs. */
  if (thread->attr.detach == PTHREAD_CREATE_DETACHED) {
    DBG ((0, 0, "pthread_join(%d, %s): thread detached", 
	  thread->pid, pthread_name (thread)));
    return EINVAL;
  }

  /* And only one proc can do a join. */
  if (thread->joined) {
    DBG ((0, 0, "pthread_join(%d, %s): thread already joined",
	  thread->pid, pthread_name (thread)));
    return ESRCH;
  }

  return 0;
}

int
pthread_join (pthread_t target, void **statusp)
{
  pthread_t self = PTHREAD_SELF;
  int status;
  int running;

  DBG ((1, 1, "pthread_join(%d, %s):", target->pid, pthread_name (target)));

  /* Check for a valid, joinable thread. */
  if ((status = pthread_not_joinable (target))) {
    DBG ((1, -1, ""));
    return status;
  }

  /* Avoid deadlock. */
  if (target == self) {
    DBG ((0, 0, "pthread_join(%d, %s): avoid deadlock <= EDEADLK",
	  target->pid, pthread_name (target)));
    DBG ((1, -1, ""));
    return EDEADLK;
  }

  /* Actually perform the join using the handshake semaphore. */
  thread_lock (target); {
    /* Check for a valid, joinable thread. */
    if ((status = pthread_not_joinable (target))) {
      DBG ((1, -1, ""));
      thread_unlock (target);
      return status;
    }

    /* Check that it's running */
    running = thread_running (target);
  }; thread_unlock (target);

  if (running) {
    target->joined = self;
    self->mdata = target;
    thread_suspend (self, MON_JOIN);
  }

  thread_free (target);

  DBG ((1, -1, "pthread_join(%d, %s): succeeded. status = %p",
	target->pid, pthread_name (target), target->exit_status));

  if (statusp) *statusp = target->exit_status;
  
  return 0;
}

int
pthread_detach (pthread_t thread)
{
  int status;

  /* Check for a valid, joinable thread. */
  if ((status = pthread_not_joinable (thread)))
    return status;

  thread->attr.detach = PTHREAD_CREATE_DETACHED;

  return 0;
}


int
pthread_kill (pthread_t thread, int signo)
{
  if (! thread || thread->attr.detach == PTHREAD_CREATE_DETACHED) {
    DBG ((0, 0, "pthread_kill(%s): thread invalid <= ESRCH", 
	  pthread_name (thread)));
    return ESRCH;
  }

  /* Use basic kill. */
  DBG ((1, 0, "pthread_kill (%d, %s): signum %d",
	thread->pid, pthread_name (thread), signo));
  if (kill (thread->pid, signo) == -1)
    return errno;

  return 0;
}

void
pthread_cleanup_push (void (*func) (void *), void *closure)
{
  thread_push_cleanup (pthread_self (), func, closure);
}

void
pthread_cleanup_pop (int execute)
{
  thread_pop_cleanup (pthread_self (), execute);
}

static void
default_key_destructor (void *value)
{
}

int
pthread_key_create (pthread_key_t *keyp, void (*destruct) (void *value))
{
  pthread_key_t key;

  if (thread_init) (*thread_init) ();
  
  table_lock (); {
    int i;
    /* We use the destructor table to determine key availability. */
    for (i = 0, key = -1; i < nspecific; ++i) 
      if (! spec_destructor[i]) {
	key = i;
	DBG ((1, 0, "pthread_key_create (): key = %d", key));
	break;
      }

    /* If adding a new one, simply extend. */
    if (!nspecific || key == nspecific) {
      key = nspecific++;
      DBG ((1, 0, "pthread_key_create (): key = %d", key));

      /* If we need to extend the key list, then do so for every thread. */
      if (key >= len_specific) {
	int new_specific = 2*len_specific;
	spec_destructor =
	  (destructor*) realloc (spec_destructor,
				 new_specific*sizeof(destructor));
	memset (spec_destructor+len_specific, 0,
		len_specific*sizeof(destructor));
	DBG ((1, 0, "pthread_key_create (): extend from %d to %d keys",
	      len_specific, new_specific));

	for (i = 0; i < thr_table_len; ++i) {
	  pthread_t thread = thr_table[i];
	  if (thread && thread_running (thread)) {
	    thread->specific =
	      (void const **) realloc (thread->specific, new_specific*sizeof(void*));
	    memset (thread->specific+len_specific, 0,
		    len_specific*sizeof(void*));
	  }
	}
	len_specific = new_specific;
      }
    }

    DBG ((1, 0, "pthread_key_create (%d): destructor %p",
	  key, destruct));
    spec_destructor[key] =
      (destruct ? destruct : default_key_destructor);
  }; table_unlock ();

  *keyp = key;
  return 0;
}

int
pthread_setspecific (pthread_key_t key, void const *value)
{
  pthread_t self;

  if (key >= nspecific || ! spec_destructor[key])
    return EINVAL;

  self = PTHREAD_SELF;
  self->specific[key] = value;

  return 0;
}

void *
pthread_getspecific (pthread_key_t key)
{
  pthread_t self;

  if (key >= nspecific || ! spec_destructor[key])
    return 0;
  
  self = PTHREAD_SELF;
  return (void *) self->specific[key];
}

int
pthread_key_delete (pthread_key_t key)
{
  if (key >= nspecific || ! spec_destructor[key])
    return EINVAL;

  table_lock (); {
    spec_destructor[key] = 0;
  }; table_unlock ();
  return 0;
}

struct _pthread_cond {
  pthread_mutex_t lock;
  list *list;
  pthread_mutex_t *mutex;
};

static void
cond_describe_ (pthread_cond_t cond)
{
  list *list = cond->list;
  fprintf (stderr, "COND %p ", cond);
  mutex_describe_ (*cond->mutex);
}

struct _pthread_condattr {
  int dummy;
};

int
pthread_cond_init (pthread_cond_t *condp, pthread_condattr_t const *attr)
{
  pthread_cond_t cond = (pthread_cond_t) calloc (1, sizeof (*cond));

  if (! cond) {
    DBG ((0, 0, "pthread_cond_init: calloc(%d) failed", sizeof (*cond)));
    return ENOMEM;
  }
#if 0
  if (*condp) return EBUSY;
#endif

  /* Initialize if necessary. */
  if (thread_init) (*thread_init) ();

  DBG ((1, 0, "pthread_cond_init (&%p)", cond));

  cond->lock = newlock ();
  *condp = cond;

  return 0;
}

static int
cond_lock (pthread_cond_t cond)
{
  DBG ((2, 1, "cond_lock (&%p) {", cond));
  return setlock (cond->lock);
}

static int
cond_unlock (pthread_cond_t cond)
{
  DBG ((2, -1, "} cond_unlock (&%p)", cond));
  return unsetlock (cond->lock);
}

static void
cond_remove_wait (pthread_cond_t cond, pthread_t thr)
{
  cond_lock (cond); {
    DBG ((1, 0, "cond_remove_wait (%p) thread (%d, %s)",
	  cond, thr->pid, pthread_name (thr)));
    list_remove (&cond->list, thr);
  }; cond_unlock (cond);
}

int
pthread_cond_wait (pthread_cond_t *condp, pthread_mutex_t *mutex)
{
  /* Initialize if necessary. */
  if (thread_init) (*thread_init) ();

  /* Check for an uninitialized cond. */
  if (condp == PTHREAD_COND_INITIALIZER) {
    int ret;
    *condp = malloc (sizeof (**condp));
    if ((ret = pthread_cond_init (condp, NULL)) != 0)
      return ret;
  }

  {
    pthread_t self = PTHREAD_SELF;
    pthread_cond_t cond = *condp;

    thread_testcancel (self);

    DBG ((1, 1, "pthread_cond_wait (&%p):", cond));

    /* Check that a common mutex is being used. */
    if (cond->mutex && cond->mutex != mutex) {
      DBG ((0, 0, "pthread_cond_wait (&%p): invalid mutex <= EINVAL", cond));
      DBG ((1, -1, ""));
      return EINVAL;
    }

    /* Add the current thread to the start of the list. */
    cond_lock (cond); {
      list_enqueue (&cond->list, self);
      cond->mutex = mutex;
      pthread_mutex_unlock (mutex);
    }; cond_unlock (cond);

    /* Now wait for the condition to be raised. */
    self->mdata = cond;
    thread_suspend (self, MON_COND_WAIT);
    pthread_mutex_lock (mutex);
    DBG ((1, -1, "pthread_cond_wait (&%p): done.", cond));
  }

  return 0;
}

int
pthread_cond_timed_wait (pthread_cond_t *condp, pthread_mutex_t *mutex,
			 struct timespec const abstime)
{
  return pthread_cond_wait (condp, mutex);
}

int
pthread_cond_signal (pthread_cond_t *condp)
{
  pthread_cond_t cond;

  /* Ensure that we've got a real cond. */
  if (! condp || ! (cond = *condp)) {
    DBG ((0, 0, "pthread_cond_signal(%p): invalid <= EFAULT", condp));
    return EFAULT;
  }

  if (cond->list) {
    cond_lock (cond); {
      /* Grab the head of the list. */
      pthread_t thread = list_dequeue (&cond->list);

      DBG ((1, 1, "pthread_cond_signal (%p):", cond));
      thread_resume (thread);
      DBG ((1, -1, "pthread_cond_signal (%p): done", cond));
    }; cond_unlock (cond);
  }

  return 0;
}

int
pthread_cond_broadcast (pthread_cond_t *condp)
{
  pthread_cond_t cond;

  /* Ensure that we've got a real cond. */
  if (! condp || ! (cond = *condp)) {
    DBG ((0, 0, "pthread_cond_broadcast(%p): invalid <= EFAULT", condp));
    return EFAULT;
  }

  if (cond->list) {
    cond_lock (cond); {
      pthread_t thread = list_dequeue (&cond->list);

      /* Go to end of list. */
      DBG ((1, 1, "pthread_cond_broadcast (&%p):", cond));
      while (thread) {
	thread_resume (thread);
	thread = list_dequeue (&cond->list);
      }
      DBG ((1, -1, "pthread_cond_broadcast (&%p): done", cond));
    }; cond_unlock (cond);
  }

  return 0;
}

int
pthread_cond_destroy (pthread_cond_t *condp)
{
  pthread_cond_t cond;

  /* Ensure that we've got a real cond. */
  if (! condp || ! (cond = *condp)) {
    DBG ((0, 0, "pthread_cond_destroy(%p): invalid <= EFAULT", condp));
    return EFAULT;
  }

  /* Check for a busy condition. */
  if (cond->list) {
    DBG ((0, 0, "pthread_cond_destroy(&%p): waiting threads (%p,...) <= EBUSY",
	  cond, cond->list->value));
    return EBUSY;
  }

  DBG ((1, 0, "pthread_cond_destroy (&%p)", cond));

  /* Free the condition variable. */
  freelock (cond->lock);
  free (cond);
  *condp = 0;
  return 0;
}

int
pthread_attr_init (pthread_attr_t *attr)
{
  if (! attr)
    return errno;

  attr->scope = PTHREAD_SCOPE_PROCESS;
  attr->detach = PTHREAD_CREATE_JOINABLE;
  attr->stackaddr = NULL;
  attr->stacksize = 1<<20;
  attr->sched.policy = SCHED_OTHER;
  attr->inheritsched = PTHREAD_EXPLICIT_SCHED;

  return 0;
}

int
pthread_attr_destroy (pthread_attr_t *attr)
{
  return 0;
}

int
pthread_attr_setscope (pthread_attr_t *attr,
		       enum pthread_attr_scope val)
{
  attr->scope = val;
  return 0;
}

int
pthread_attr_getscope (pthread_attr_t *attr,
		       enum pthread_attr_scope *valp)
{
  if (! valp) return EINVAL;
  *valp = attr->scope;
  return 0;
}

int
pthread_attr_setdetachstate (pthread_attr_t *attr,
			     enum pthread_attr_detach val)
{
  attr->detach = val;
  return 0;
}

int
pthread_attr_getdetachstate (pthread_attr_t *attr,
			     enum pthread_attr_detach *valp)
{
  if (! valp) return EINVAL;
  *valp = attr->detach;
  return 0;
}

int
pthread_attr_setstacksize (pthread_attr_t *attr,
			   size_t val)
{
  attr->stacksize = val;
  return 0;
}

int
pthread_attr_getstacksize (pthread_attr_t *attr,
			   size_t *valp)
{
  if (! valp) return EINVAL;
  *valp = attr->stacksize;
  return 0;
}

int
pthread_attr_setstackaddr (pthread_attr_t *attr,
			   void *val)
{
  attr->stackaddr = val;
  return 0;
}

int
pthread_attr_getstackaddr (pthread_attr_t *attr,
			   void **valp)
{
  if (! valp) return EINVAL;
  *valp = attr->stackaddr;
  return 0;
}

int
pthread_attr_setschedpolicy (pthread_attr_t *attr,
			     enum sched_policy val)
{
  attr->sched.policy = val;
  return 0;
}

int
pthread_attr_getschedpolicy (pthread_attr_t *attr,
			     enum sched_policy *valp)
{
  if (! valp) return EINVAL;
  *valp = attr->sched.policy;
  return 0;
}

int
pthread_attr_setinheritsched (pthread_attr_t *attr,
			      enum pthread_attr_sched val)
{
  attr->inheritsched = val;
  return 0;
}

int
pthread_attr_getinheritsched (pthread_attr_t *attr,
			      enum pthread_attr_sched *valp)
{
  if (! valp) return EINVAL;
  *valp = attr->inheritsched;
  return 0;
}

int
sched_yield (void)
{
  sginap (0);
  return 0;
}

#if 0
#define CANCELABLE_SYSCALL(res_type, name, param_list, params)		      \
res_type (*__libc_##name) param_list;					      \
res_type								      \
sproc_##name param_list							      \
{									      \
  res_type result;							      \
  int oldtype;								      \
  fprintf (stderr, "Wrapped: " #name "\n"); \
  pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);	      \
  result = __libc_##name params;					      \
  pthread_setcanceltype (oldtype, NULL);				      \
  return result;							      \
}

CANCELABLE_SYSCALL (int, open, (const char *pathname, int flags, int mode),
		    (pathname, flags, mode))

CANCELABLE_SYSCALL (int, close, (int fd), (fd))
#endif
