#ifndef __INC_LIBTHECORE_TOPT_H__
#define __INC_LIBTHECORE_TOPT_H__

extern int compute_code(const uint8_t *secret, int secretLen, unsigned long value);
extern int verify_code(const uint8_t * secret, int secretLen, int code);

#endif
