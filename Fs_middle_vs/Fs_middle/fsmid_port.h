#ifndef _FSMID_PORT_H_
#define _FSMID_PORT_H_

#define fsmid_malloc(sz)		malloc(sz)
#define fsmid_free(ptr)			free(ptr)

#define FSMID_MUTEX				
#define fsmid_mutex_create(mtx)
#define fsmid_mutex_lock(mtx)
#define fsmid_mutex_unlock(mtx)

#endif
