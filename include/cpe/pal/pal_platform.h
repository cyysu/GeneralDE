#ifndef CPE_PAL_PLATFORM_H
#define CPE_PAL_PLATFORM_H

//TODO
#if 0
#define CPE_BIG_ENDIAN
#else
#define CPE_LITTLE_ENDIAN
#endif

#ifdef _MSC_VER
#  define CPE_DEFAULT_ALIGN (1)
#  if defined (__LP64__) || defined (__64BIT__) || defined (_LP64)
#    define __WORDSIZE (64)
#  else
#    define __WORDSIZE (32)
#  endif
#else
#  define CPE_DEFAULT_ALIGN (__WORDSIZE / 8)
#endif

#define CPE_PAL_CALC_ALIGN_8(__value) if ((__value) % 8) { (__value) = ((((__value) >> 3) + 1) << 3); }
#define CPE_PAL_CALC_ALIGN_4(__value) if ((__value) % 4) { (__value) = ((((__value) >> 2) + 1) << 2); }
#define CPE_PAL_CALC_ALIGN_2(__value) if ((__value) % 2) { (__value) = ((((__value) >> 1) + 1) << 1); }

#define CPE_PAL_CALC_ALIGN(__value, __align)    \
    switch(__align) {                           \
    case 2:                                     \
        CPE_PAL_CALC_ALIGN_2(__value);          \
        break;                                  \
    case 4:                                     \
        CPE_PAL_CALC_ALIGN_4(__value);          \
        break;                                  \
    case 8:                                     \
        CPE_PAL_CALC_ALIGN_8(__value);          \
        break;                                  \
    }


#if (__WORDSIZE == 64)
#define CPE_PAL_ALIGN_DFT CPE_PAL_CALC_ALIGN_8
#else
#define CPE_PAL_ALIGN_DFT CPE_PAL_CALC_ALIGN_4
#endif

#define CPE_SWAP_ENDIAN64(outp, inp) do {           \
        const char * __in = (const char *)(inp);    \
        char *__out = (char *)(outp);               \
        __out[0] = __in[7];                         \
        __out[1] = __in[6];                         \
        __out[2] = __in[5];                         \
        __out[3] = __in[4];                         \
        __out[4] = __in[3];                         \
        __out[5] = __in[2];                         \
        __out[6] = __in[1];                         \
        __out[7] = __in[0];                         \
    } while(0)

#define CPE_SWAP_ENDIAN32(outp, inp) do {       \
        const char * in = (const char *)(inp);  \
        char *out = (char *)(outp);             \
                                                \
        out[0] = in[3];                         \
        out[1] = in[2];                         \
        out[2] = in[1];                         \
        out[3] = in[0];                         \
    } while(0)

#ifdef CPE_BIG_ENDIAN
#define CPE_COPY_HTON64(outp, inp) CPE_SWAP_ENDIAN64(outp, inp)
#define CPE_COPY_NTOH64(outp, inp) CPE_SWAP_ENDIAN64(outp, inp)
#define CPE_COPY_HTON32(outp, inp) CPE_SWAP_ENDIAN32(outp, inp)
#define CPE_COPY_NTOH32(outp, inp) CPE_SWAP_ENDIAN32(outp, inp)
#else
#define CPE_COPY_HTON64(outp, inp) do { memcpy(outp, inp, 8); } while(0)
#define CPE_COPY_NTOH64(outp, inp) do { memcpy(outp, inp, 8); } while(0)
#define CPE_COPY_HTON32(outp, inp) do { memcpy(outp, inp, 4); } while(0)
#define CPE_COPY_NTOH32(outp, inp) do { memcpy(outp, inp, 4); } while(0)
#endif

#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE static
#endif

#endif

