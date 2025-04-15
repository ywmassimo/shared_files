#ifndef __INC_METIN_II_TELLWAIT_H__
#define __INC_METIN_II_TELLWAIT_H__

#ifndef __WIN32__

extern void TELL_WAIT();

extern void WAIT_CHILD();
extern void TELL_CHILD(pid_t pid);

extern void WAIT_PARENT();
extern void TELL_PARENT(pid_t pid);

#endif

#endif
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
