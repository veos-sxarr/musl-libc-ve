#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>

char *__strchrnul(const char *, int);

char *__shm_mapname(const char *name, char *buf)
{
	char *p;
	while (*name == '/') name++;
	if (*(p = __strchrnul(name, '/')) || p==name ||
	    (p-name <= 2 && name[0]=='.' && p[-1]=='.')) {
		errno = EINVAL;
		return 0;
	}
	if (p-name > NAME_MAX) {
		errno = ENAMETOOLONG;
		return 0;
	}
#ifdef __ve__
	memcpy(buf, BASE_SEM_PATH, BASE_SEM_PATH_SIZE);
	memcpy(buf + BASE_SEM_PATH_SIZE, name, p-name+1);
#else
	memcpy(buf, "/dev/shm/", 9);
	memcpy(buf+9, name, p-name+1);
#endif
	return buf;
}

int shm_open(const char *name, int flag, mode_t mode)
{
	int cs;
	char buf[NAME_MAX+10];
	if (!(name = __shm_mapname(name, buf))) return -1;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	int fd = open(name, flag|O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK, mode);
	pthread_setcancelstate(cs, 0);
	return fd;
}

int shm_unlink(const char *name)
{
	char buf[NAME_MAX+10];
	if (!(name = __shm_mapname(name, buf))) return -1;
	return unlink(name);
}