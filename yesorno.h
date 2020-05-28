#ifndef YESORNO_H_DEFINED_
#define YESORNO_H_DEFINED_

#if defined(HAVE_STDC)
typedef enum { NO = 0, YES = 1 } YESorNO;
#else /* K&R style */
#define NO  0				/* must be FALSE (zero) */
#define YES 1				/* must be TRUE (non-zero) */
typedef int YESorNO;
#endif

#endif /* YESORNO_H_DEFINED_ */
