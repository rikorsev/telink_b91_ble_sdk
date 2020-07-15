#include "opus_config.h"


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "opus.h"
#include "debug.h"
#include "opus_types.h"
#include "opus_private.h"
#include "opus_multistream.h"
#include "../../../../drivers/9518/compiler.h"
#include "opus_defines.h"
#include "../../../../tl_common.h"
#include "drivers.h"
#define CODEC_ENC_SIZE			(1024 * 2)

#define MAX_PACKET  			(320)
//volatile unsigned char data0Buf[MAX_PACKET];
//volatile unsigned char data1Buf[MAX_PACKET];
volatile unsigned char debug_bufin[640];
volatile unsigned char debug_bufout[40];
volatile unsigned char debug_dat0;
volatile unsigned char debug_dat1;
#if 0
const unsigned short inbuf[CODEC_ENC_SIZE] = {
	0xFFED,0xFFDF,0xFFE2,0xFFE0,0xFFDE,0xFFDE,0xFFDE,0xFFDE,
	0xFFDD,0xFFDC,0xFFDD,0xFFDC,0xFFDB,0xFFDB,0xFFDB,0xFFD9,
	0xFFDA,0xFFD9,0xFFD9,0xFFD8,0xFFD8,0xFFD8,0xFFDA,0xFFD9,
	0xFFD6,0xFFD7,0xFFD9,0xFFD6,0xFFD6,0xFFD7,0xFFD7,0xFFD6,
	0xFFD6,0xFFD7,0xFFD7,0xFFD7,0xFFD7,0xFFD7,0xFFD6,0xFFD5,
	0xFFD7,0xFFD8,0xFFD6,0xFFD7,0xFFD6,0xFFD7,0xFFD5,0xFFD5,
	0xFFD6,0xFFD7,0xFFD8,0xFFD6,0xFFD8,0xFFD7,0xFFD8,0xFFD7,
	0xFFD8,0xFFD9,0xFFD9,0xFFD9,0xFFDA,0xFFDB,0xFFDB,0xFFDA,
	0xFFDA,0xFFDA,0xFFDB,0xFFDC,0xFFDD,0xFFDD,0xFFDE,0xFFDC,
	0xFFDC,0xFFDE,0xFFE0,0xFFDF,0xFFE1,0xFFE1,0xFFE0,0xFFE2,
	0xFFE3,0xFFE3,0xFFE4,0xFFE4,0xFFE6,0xFFE6,0xFFE6,0xFFE6,
	0xFFE8,0xFFE8,0xFFE9,0xFFE9,0xFFEB,0xFFEB,0xFFED,0xFFED,
	0xFFEE,0xFFEF,0xFFEF,0xFFF1,0xFFF0,0xFFF3,0xFFF1,0xFFF4,
	0xFFF3,0xFFF5,0xFFF5,0xFFF5,0xFFF7,0xFFF8,0xFFF9,0xFFF7,
	0xFFFA,0xFFF9,0xFFFB,0xFFFC,0xFFFD,0xFFFC,0xFFFE,0x0000,
	0xFFFF,0x0000,0x0002,0x0002,0x0002,0x0002,0x0005,0x0005,
	0x0005,0x0009,0x0005,0x0005,0x0006,0x0007,0x0008,0x0007,
	0x0008,0x0009,0x000A,0x000A,0x000B,0x000B,0x000C,0x000D,
	0x000E,0x000C,0x000E,0x000E,0x000E,0x000F,0x000F,0x0010,
	0x0011,0x0011,0x0011,0x0011,0x0010,0x0012,0x0010,0x0012,
	0x0012,0x0013,0x0014,0x0014,0x0015,0x0014,0x0013,0x0016,
	0x0016,0x0016,0x0013,0x0016,0x0016,0x0016,0x0018,0x0018,
	0x0016,0x0015,0x0017,0x0016,0x0017,0x0018,0x0016,0x0016,
	0x0016,0x0015,0x0016,0x0016,0x0017,0x0016,0x0014,0x0013,
	0x0014,0x0016,0x0013,0x0015,0x0015,0x0016,0x0015,0x0015,
	0x0017,0x0016,0x0017,0x0015,0x0015,0x0015,0x0017,0x0016,
	0x0016,0x0016,0x0016,0x0015,0x0015,0x0016,0x0016,0x0015,
	0x0015,0x0016,0x0015,0x0015,0x0016,0x0014,0x0015,0x0014,
	0x0013,0x0014,0x0015,0x0013,0x0013,0x0012,0x0014,0x0012,
	0x0012,0x000F,0x000E,0x0010,0x000E,0x000F,0x000D,0x0010,
	0x000E,0x000E,0x000C,0x000D,0x000C,0x000B,0x0009,0x000A,
	0x0009,0x0008,0x0008,0x0007,0x0006,0x0006,0x0004,0x0006,
	0x0004,0x0005,0x0001,0x0003,0x0001,0x0002,0x0003,0x0001,
	0xFFFF,0x0000,0x0001,0xFFFE,0xFFFF,0xFFFE,0xFFFE,0xFFFC,
	0xFFFC,0xFFFC,0xFFFC,0xFFFA,0xFFF9,0xFFFA,0xFFF9,0xFFF9,
	0xFFF8,0xFFF8,0xFFF7,0xFFF6,0xFFF6,0xFFF8,0xFFF6,0xFFF5,
	0xFFF5,0xFFF2,0xFFF4,0xFFF4,0xFFF4,0xFFF1,0xFFF2,0xFFF2,
	0xFFF1,0xFFF1,0xFFF1,0xFFEF,0xFFF0,0xFFEF,0xFFF0,0xFFEF,
	0xFFF0,0xFFED,0xFFED,0xFFED,0xFFED,0xFFEC,0xFFEC,0xFFEB,
	0xFFEC,0xFFEC,0xFFEA,0xFFEC,0xFFEA,0xFFEB,0xFFE9,0xFFE9,
	0xFFEB,0xFFEA,0xFFE8,0xFFEA,0xFFE8,0xFFE8,0xFFE7,0xFFE6,
	0xFFE6,0xFFE6,0xFFE5,0xFFE6,0xFFE5,0xFFE4,0xFFE3,0xFFE4,
	0xFFE4,0xFFE3,0xFFE3,0xFFE3,0xFFE4,0xFFE3,0xFFE3,0xFFE2,
	0xFFE1,0xFFE1,0xFFE2,0xFFE0,0xFFE1,0xFFE1,0xFFE2,0xFFE1,
	0xFFE1,0xFFE4,0xFFE3,0xFFE4,0xFFE3,0xFFE3,0xFFE4,0xFFE3,
	0xFFE5,0xFFE4,0xFFE3,0xFFE4,0xFFE5,0xFFE7,0xFFE5,0xFFE7,
	0xFFE5,0xFFE5,0xFFE6,0xFFE8,0xFFE8,0xFFE8,0xFFEA,0xFFE7,
	0xFFE9,0xFFE9,0xFFEA,0xFFE9,0xFFEB,0xFFEA,0xFFEC,0xFFED,
	0xFFEA,0xFFEB,0xFFED,0xFFEE,0xFFEE,0xFFED,0xFFEE,0xFFED,
	0xFFF0,0xFFEE,0xFFEE,0xFFF0,0xFFEF,0xFFEF,0xFFF0,0xFFF1,
	0xFFF1,0xFFF1,0xFFF2,0xFFF2,0xFFF3,0xFFF2,0xFFF2,0xFFF2,
	0xFFF3,0xFFF5,0xFFF3,0xFFF4,0xFFF5,0xFFF4,0xFFF4,0xFFF5,
	0xFFF5,0xFFF6,0xFFF6,0xFFF8,0xFFF9,0xFFF7,0xFFF7,0xFFFA,
	0xFFF9,0xFFFB,0xFFFB,0xFFFA,0xFFFB,0xFFFA,0xFFFB,0xFFFB,
	0xFFFC,0xFFFD,0xFFFE,0xFFFC,0xFFFE,0xFFFD,0xFFFF,0xFFFE,
	0xFFFF,0xFFFF,0xFFFF,0x0000,0x0000,0x0001,0x0001,0x0000,
	0x0002,0x0003,0x0001,0x0004,0x0004,0x0005,0x0004,0x0005,
	0x0005,0x0006,0x0007,0x0006,0x0007,0x0007,0x0008,0x0006,
	0x0008,0x0008,0x0008,0x0009,0x000A,0x000A,0x000A,0x000C,
	0x000B,0x000B,0x000B,0x000C,0x000C,0x000C,0x000D,0x000E,
	0x000D,0x000E,0x000C,0x000F,0x000F,0x000F,0x000D,0x0010,
	0x0010,0x000E,0x0010,0x000F,0x000E,0x000F,0x0010,0x0010,
	0x000F,0x000E,0x0010,0x000E,0x000E,0x000F,0x000F,0x000D,
	0x000E,0x000F,0x000F,0x000E,0x000D,0x000F,0x000F,0x000D,
	0x000C,0x000C,0x000C,0x000C,0x000B,0x000C,0x000A,0x000B,
	0x0009,0x0009,0x0007,0x0009,0x0009,0x000A,0x0008,0x0008,
	0x0008,0x0008,0x0008,0x0006,0x0008,0x0007,0x0008,0x0009,
	0x0006,0x0006,0x0007,0x0006,0x0008,0x0006,0x0007,0x0006,
	0x0004,0x0005,0x0004,0x0006,0x0003,0x0004,0x0004,0x0003,
	0x0004,0x0003,0x0002,0x0000,0x0000,0x0000,0x0000,0x0001,
	0xFFFF,0xFFFD,0xFFFE,0x0000,0xFFFD,0xFFFD,0xFFFC,0xFFFC,
	0xFFFC,0xFFFB,0xFFFC,0xFFFB,0xFFFD,0xFFFB,0xFFFA,0xFFF9,
	0xFFF8,0xFFF9,0xFFF8,0xFFF7,0xFFF7,0xFFF6,0xFFF6,0xFFF4,
	0xFFF5,0xFFF4,0xFFF3,0xFFF4,0xFFF2,0xFFF1,0xFFF1,0xFFF0,
	0xFFF0,0xFFEF,0xFFEE,0xFFED,0xFFEC,0xFFEC,0xFFEA,0xFFE9,
	0xFFEA,0xFFE9,0xFFE8,0xFFE8,0xFFE7,0xFFE7,0xFFE6,0xFFE5,
	0xFFE5,0xFFE4,0xFFE5,0xFFE3,0xFFE2,0xFFE1,0xFFE1,0xFFDE,
	0xFFDE,0xFFDF,0xFFDD,0xFFDD,0xFFDB,0xFFDB,0xFFDB,0xFFD9,
	0xFFD8,0xFFDA,0xFFD8,0xFFD7,0xFFD6,0xFFD7,0xFFD6,0xFFD6,
	0xFFD4,0xFFD4,0xFFD4,0xFFD3,0xFFD3,0xFFD2,0xFFD2,0xFFD0,
	0xFFD2,0xFFD2,0xFFD1,0xFFD0,0xFFD0,0xFFD0,0xFFCF,0xFFD0,
	0xFFCE,0xFFCF,0xFFCE,0xFFCF,0xFFCE,0xFFCD,0xFFCE,0xFFD0,
	0xFFCE,0xFFCE,0xFFCE,0xFFCE,0xFFCF,0xFFD0,0xFFCF,0xFFCE,
	0xFFCE,0xFFD0,0xFFCF,0xFFD0,0xFFCE,0xFFD0,0xFFD0,0xFFD0,
	0xFFCF,0xFFD2,0xFFD1,0xFFD2,0xFFD1,0xFFD1,0xFFD1,0xFFD1,
	0xFFD3,0xFFD1,0xFFD3,0xFFD2,0xFFD2,0xFFD3,0xFFD3,0xFFD3,
	0xFFD1,0xFFD4,0xFFD5,0xFFD5,0xFFD4,0xFFD7,0xFFD6,0xFFD7,
	0xFFD8,0xFFD7,0xFFD6,0xFFD7,0xFFD7,0xFFD7,0xFFD9,0xFFD8,
	0xFFD9,0xFFDA,0xFFDA,0xFFDA,0xFFDA,0xFFDB,0xFFDB,0xFFDC,
	0xFFDD,0xFFDC,0xFFDC,0xFFDC,0xFFDD,0xFFDE,0xFFDE,0xFFDE,
	0xFFDD,0xFFDF,0xFFE1,0xFFE1,0xFFE1,0xFFE1,0xFFE2,0xFFE1,
	0xFFE3,0xFFE3,0xFFE3,0xFFE6,0xFFE4,0xFFE6,0xFFE6,0xFFE8,
	0xFFE8,0xFFE9,0xFFE8,0xFFE9,0xFFE9,0xFFE9,0xFFEA,0xFFEA,
	0xFFEB,0xFFEB,0xFFEC,0xFFEE,0xFFEE,0xFFEE,0xFFEE,0xFFEF,
	0xFFF1,0xFFEE,0xFFF0,0xFFF0,0xFFF1,0xFFF1,0xFFF2,0xFFF3,
	0xFFF3,0xFFF4,0xFFF6,0xFFF3,0xFFF4,0xFFF5,0xFFF4,0xFFF6,
	0xFFF7,0xFFF6,0xFFF6,0xFFF7,0xFFF9,0xFFF8,0xFFFA,0xFFF8,
	0xFFFB,0xFFFA,0xFFFA,0xFFFB,0xFFFB,0xFFFB,0xFFFA,0xFFFC,
	0xFFFD,0xFFFD,0xFFFD,0xFFFD,0xFFFC,0xFFFD,0xFFFF,0xFFFE,
	0xFFFF,0xFFFE,0xFFFD,0xFFFF,0x0000,0x0002,0x0002,0x0000,
	0x0001,0x0002,0x0003,0x0003,0x0003,0x0003,0x0002,0x0005,
	0x0004,0x0004,0x0003,0x0003,0x0003,0x0005,0x0004,0x0006,
	0x0004,0x0006,0x0005,0x0005,0x0004,0x0005,0x0006,0x0006,
	0x0008,0x0007,0x0006,0x0005,0x0008,0x0007,0x0007,0x0007,
	0x0007,0x0007,0x0008,0x0008,0x0008,0x0007,0x0009,0x000A,
	0x0009,0x0009,0x000A,0x000A,0x000A,0x000A,0x000A,0x0009,
	0x0009,0x000A,0x0009,0x000A,0x0009,0x0009,0x000A,0x0009,
	0x000A,0x0009,0x0009,0x0009,0x0009,0x0009,0x000A,0x000A,
	0x000A,0x0009,0x0009,0x0008,0x0009,0x0008,0x000A,0x0008,
	0x000A,0x000A,0x0008,0x0009,0x0009,0x0009,0x0008,0x0009,
	0x0009,0x0009,0x0008,0x0007,0x0007,0x0007,0x0009,0x0008,
	0x0007,0x0007,0x0007,0x0006,0x0006,0x0005,0x0005,0x0006,
	0x0006,0x0006,0x0006,0x0005,0x0005,0x0005,0x0004,0x0004,
	0x0004,0x0006,0x0003,0x0003,0x0002,0x0001,0x0002,0x0001,
	0x0002,0x0000,0xFFFF,0xFFFF,0x0001,0x0000,0xFFFF,0x0000,
	0xFFFF,0x0000,0xFFFE,0xFFFF,0xFFFD,0x0000,0xFFFE,0xFFFF,
	0xFFFE,0xFFFF,0xFFFF,0x0000,0xFFFE,0xFFFE,0xFFFE,0xFFFF,
	0xFFFE,0xFFFF,0xFFFE,0xFFFC,0xFFFD,0xFFFC,0xFFFB,0xFFFC,
	0xFFFC,0xFFFB,0xFFFB,0xFFFB,0xFFFC,0xFFFC,0xFFFA,0xFFFB,
	0xFFF9,0xFFFB,0xFFFA,0xFFFA,0xFFFB,0xFFF9,0xFFF9,0xFFF9,
	0xFFFB,0xFFFA,0xFFF8,0xFFF9,0xFFF9,0xFFF9,0xFFF8,0xFFF7,
	0xFFFA,0xFFF9,0xFFF8,0xFFF8,0xFFF7,0xFFF8,0xFFF7,0xFFF7,
	0xFFF5,0xFFF9,0xFFF7,0xFFF8,0xFFF9,0xFFF7,0xFFF7,0xFFF7,
	0xFFF7,0xFFF8,0xFFF9,0xFFF7,0xFFF6,0xFFF7,0xFFF7,0xFFF8,
	0xFFF6,0xFFF5,0xFFF6,0xFFF7,0xFFF6,0xFFF5,0xFFF7,0xFFF8,
	0xFFF8,0xFFF8,0xFFF7,0xFFF8,0xFFF7,0xFFF7,0xFFF7,0xFFF9,
	0xFFF8,0xFFF9,0xFFF9,0xFFF8,0xFFF8,0xFFF8,0xFFF7,0xFFF8,
	0xFFF9,0xFFF8,0xFFFA,0xFFFD,0xFFFC,0xFFFD,0xFFFB,0xFFFD,
	0xFFFD,0xFFFC,0xFFFF,0xFFFD,0xFFFE,0xFFFF,0x0001,0xFFFF,
	0x0001,0x0000,0x0000,0x0002,0x0002,0x0000,0x0001,0x0003,
	0x0003,0x0004,0x0005,0x0005,0x0004,0x0004,0x0004,0x0004,
	0x0004,0x0007,0x0007,0x0008,0x0006,0x0007,0x0008,0x0008,
	0x0008,0x0008,0x0008,0x0009,0x000B,0x000A,0x000B,0x000A,
	0x000B,0x000C,0x000D,0x000D,0x000C,0x000D,0x000D,0x000C,
	0x000D,0x000D,0x000C,0x000E,0x000E,0x000D,0x000E,0x000D,
	0x000F,0x0010,0x000E,0x000E,0x000F,0x0010,0x000F,0x000F,
	0x000F,0x000F,0x0011,0x0011,0x0011,0x0011,0x0010,0x0011,
	0x0011,0x0010,0x0011,0x0013,0x0013,0x0011,0x0013,0x0012,
	0x000F,0x0011,0x0011,0x0012,0x0010,0x0012,0x0012,0x0013,
	0x0013,0x0011,0x0013,0x0012,0x0012,0x0011,0x0013,0x0012,
	0x0013,0x0013,0x0013,0x0013,0x0012,0x0013,0x0014,0x0013,
	0x0014,0x0013,0x0015,0x0012,0x0013,0x0016,0x0016,0x0014,
	0x0014,0x0014,0x0015,0x0013,0x0013,0x0015,0x0014,0x0015,
	0x0014,0x0014,0x0014,0x0015,0x0014,0x0015,0x0013,0x0013,
	0x0014,0x0013,0x0014,0x0015,0x0014,0x0017,0x0014,0x0015,
	0x0016,0x0014,0x0014,0x0014,0x0014,0x0013,0x0016,0x0014,
	0x0015,0x0014,0x0017,0x0014,0x0016,0x0013,0x0015,0x0015,
	0x0013,0x0015,0x0014,0x0013,0x0014,0x0014,0x0013,0x0012,
	0x0016,0x0014,0x0014,0x0015,0x0015,0x0014,0x0014,0x0014,
	0x0013,0x0013,0x0013,0x0013,0x0013,0x0012,0x0013,0x0014,
	0x0014,0x0013,0x0013,0x0014,0x0013,0x0013,0x0013,0x0012,
	0x0014,0x0013,0x0011,0x0012,0x0012,0x0013,0x0013,0x0014,
	0x0015,0x0014,0x0014,0x0011,0x0014,0x0012,0x0014,0x0012,
	0x0014,0x0014,0x0013,0x0012,0x0014,0x0012,0x0013,0x0013,
	0x0010,0x0010,0x0010,0x0011,0x0014,0x0011,0x000F,0x0010,
	0x0010,0x0010,0x0011,0x000F,0x000F,0x000F,0x0010,0x000E,
	0x000F,0x000F,0x000C,0x000E,0x000E,0x000F,0x000E,0x000D,
	0x000D,0x000C,0x000D,0x000D,0x000D,0x000C,0x000B,0x000B,
	0x0009,0x0009,0x000B,0x000B,0x000A,0x000A,0x0008,0x0008,
	0x0009,0x0008,0x0008,0x0009,0x0007,0x0005,0x0006,0x0005,
	0x0004,0x0005,0x0004,0x0003,0x0005,0x0002,0x0001,0x0000,
	0x0000,0x0000,0xFFFE,0xFFFF,0x0000,0xFFFE,0xFFFD,0xFFFC,
	0xFFFC,0xFFFC,0xFFFB,0xFFFA,0xFFFA,0xFFFA,0xFFF9,0xFFF9,
	0xFFF9,0xFFF9,0xFFF7,0xFFF7,0xFFF8,0xFFF8,0xFFF6,0xFFF6,
	0xFFF5,0xFFF5,0xFFF4,0xFFF4,0xFFF4,0xFFF4,0xFFF3,0xFFF2,
	0xFFF2,0xFFF3,0xFFF1,0xFFF4,0xFFF2,0xFFF0,0xFFF0,0xFFF2,
	0xFFF1,0xFFEF,0xFFEF,0xFFED,0xFFEF,0xFFEF,0xFFEF,0xFFF0,
	0xFFEE,0xFFEE,0xFFEE,0xFFED,0xFFED,0xFFEE,0xFFED,0xFFEC,
	0xFFEE,0xFFEE,0xFFED,0xFFEE,0xFFEF,0xFFED,0xFFED,0xFFEC,
	0xFFED,0xFFEE,0xFFED,0xFFEC,0xFFEC,0xFFED,0xFFEE,0xFFED,
	0xFFEE,0xFFED,0xFFED,0xFFED,0xFFEF,0xFFED,0xFFED,0xFFEE,
	0xFFEF,0xFFEE,0xFFEF,0xFFEF,0xFFF0,0xFFF0,0xFFEF,0xFFF0,
	0xFFF0,0xFFF1,0xFFF2,0xFFF1,0xFFF2,0xFFF2,0xFFF2,0xFFF3,
	0xFFF3,0xFFF4,0xFFF5,0xFFF6,0xFFF6,0xFFF6,0xFFF7,0xFFF9,
	0xFFF8,0xFFF8,0xFFFA,0xFFFB,0xFFF9,0xFFFA,0xFFFA,0xFFFB,
	0xFFFA,0xFFFB,0xFFFD,0xFFFD,0xFFFC,0xFFFD,0xFFFD,0xFFFE,
	0xFFFE,0x0000,0x0001,0x0001,0x0000,0x0002,0x0003,0x0002,
	0x0004,0x0004,0x0003,0x0005,0x0007,0x0006,0x0007,0x0008,
	0x0007,0x0008,0x000A,0x0009,0x0009,0x0009,0x0009,0x0009,
	0x000B,0x000E,0x000D,0x000B,0x000B,0x000C,0x000C,0x000D,
	0x000C,0x000C,0x000A,0x000C,0x000B,0x000B,0x000C,0x000A,
	0x000A,0x000B,0x000B,0x000C,0x000A,0x0009,0x000B,0x0009,
	0x0009,0x000B,0x0009,0x000A,0x0007,0x000A,0x0008,0x0009,
	0x0008,0x000A,0x0009,0x0009,0x000A,0x000A,0x0008,0x0009,
	0x000A,0x000B,0x000A,0x000C,0x000C,0x000E,0x000D,0x000C,
	0x000D,0x000C,0x000D,0x000F,0x000D,0x000C,0x000F,0x000F,
	0x000F,0x000E,0x000E,0x000F,0x000F,0x000F,0x000E,0x000E,
	0x000E,0x000E,0x000F,0x000C,0x000E,0x000E,0x000D,0x000F,
	0x000F,0x0010,0x000F,0x000F,0x0011,0x000F,0x0010,0x0010,
	0x000F,0x0011,0x0010,0x0010,0x0010,0x0010,0x0012,0x0011,
	0x0010,0x000E,0x0011,0x0011,0x000F,0x0012,0x0012,0x0013,
	0x0012,0x0014,0x0015,0x0013,0x0012,0x0014,0x0017,0x0017,
	0x0017,0x0018,0x0017,0x001A,0x0018,0x001B,0x001A,0x001A,
	0x0019,0x0019,0x001A,0x0017,0x0019,0x0016,0x0015,0x0015,
	0x0013,0x0013,0x0013,0x0013,0x0013,0x0013,0x0012,0x0014,
	0x0015,0x0013,0x0013,0x0015,0x0016,0x0018,0x0016,0x0019,
	0x0018,0x0018,0x0018,0x0016,0x0017,0x0018,0x0016,0x0015,
	0x0015,0x0014,0x0015,0x0015,0x0014,0x0014,0x0015,0x0012,
	0x0011,0x0012,0x0012,0x0010,0x0013,0x0013,0x0011,0x0013,
	0x0013,0x0010,0x0011,0x0010,0x0010,0x0010,0x0010,0x000E,
	0x000F,0x0010,0x000D,0x000E,0x000B,0x000D,0x000C,0x000B,
	0x000B,0x000D,0x000B,0x000C,0x000B,0x000A,0x000A,0x000B,
	0x000C,0x000D,0x000F,0x000F,0x000D,0x000E,0x000D,0x0010,
	0x000E,0x000F,0x000C,0x000D,0x000F,0x0010,0x0010,0x000E,
	0x000E,0x000D,0x000F,0x000F,0x000B,0x000C,0x0008,0x000C,
	0x000C,0x000D,0x0008,0x000A,0x0003,0x0006,0x0008,0x000C,
	0x000D,0x000A,0x0009,0x0009,0x0004,0x0007,0x0004,0x0014,
	0x0016,0x0014,0x000D,0x000A,0x0009,0x0003,0x0003,0x0007,
	0x0008,0x0009,0x0009,0x000E,0x0005,0x0005,0x0006,0x0009,
	0x0007,0x000C,0x0009,0x0009,0x0007,0x0007,0x0006,0x0003,
	0x0003,0x0007,0x0008,0x0012,0x000E,0x0011,0x000D,0x0083,
	0x00AE,0x009F,0x0058,0x0034,0x0050,0x002E,0x0048,0x00B5,
	0x00CB,0x0099,0x0086,0x005D,0x0022,0x000B,0x000E,0xFFFC,
	0x0000,0x0016,0x000B,0x000A,0xFFF9,0xFFE6,0xFFE6,0xFFCC,
	0xFFBC,0xFFD9,0xFFD0,0xFFE2,0x000B,0x0016,0xFFFD,0xFFEC,
	0xFFE7,0xFFED,0xFFFB,0xFFF6,0x000C,0x000D,0xFFFA,0xFFF7,
	0x0001,0xFFF8,0x0000,0xFFF9,0xFFF3,0xFFF2,0xFFEF,0xFFF3,
	0x0006,0x0012,0x0023,0x0015,0x0027,0x001C,0x0017,0x0016,
	0x001E,0x0024,0x0028,0x0024,0x002E,0x0031,0x0026,0x0024,
	0x002A,0x001D,0x0025,0x002A,0x0032,0x002F,0x002C,0x0012,
	0x0006,0x0004,0x0007,0x0011,0x0014,0x001D,0x0009,0x0011,
	0x0007,0x000A,0x0013,0x0001,0x000D,0x0000,0x0003,0x000A,
	0x0016,0x0014,0x001A,0x000D,0x0013,0x000B,0x0006,0x001B,
	0x0015,0x0017,0x0012,0x0017,0x0020,0x0018,0x0012,0x0004,
	0xFFFB,0x000C,0x0012,0x001F,0x003A,0x0035,0x0022,0x0028,
	0x0036,0x0026,0x003E,0x003E,0x0042,0x004F,0x0057,0x0056,
	0x0058,0x004E,0x003A,0x001B,0x000E,0x000A,0x0011,0x0005,
	0xFFE6,0x0005,0xFFF6,0xFFDF,0xFFE9,0xFFE3,0xFFD5,0xFFDE,
	0xFFD4,0xFFD4,0xFFE5,0xFFE1,0xFFF0,0xFFFB,0xFFF1,0xFFE2,
	0x0021,0x000D,0x000E,0x0012,0x0014,0x003E,0x0027,0x0024,
	0x0023,0x0028,0x0002,0x001A,0xFFFF,0x0015,0x0044,0x0014,
	0xFFF3,0xFFF1,0x0038,0x0028,0x0017,0x0015,0x000C,0xFFDB,
	0xFFD6,0xFFDA,0xFFCE,0xFFCE,0xFFCE,0xFFD3,0xFFCF,0xFFC0,
	0xFFB8,0xFF9B,0xFFA1,0xFF97,0xFF8B,0xFF9D,0xFFAF,0xFFEA,
	0x000E,0x000D,0x000E,0x0024,0x0014,0x0011,0xFFFD,0xFFFE,
	0x0015,0x000F,0x0014,0x0021,0x0022,0x0025,0x0013,0xFFF9,
	0xFFF4,0xFFFD,0x000B,0x000B,0x0006,0x000D,0x0003,0xFFFC,
	0xFFFF,0xFFF4,0x0008,0x001B,0x0018,0x0024,0x0020,0x0016,
	0x0018,0x000C,0xFFFF,0x0018,0x0019,0x000C,0x000B,0x0002,
	0x0002,0x0016,0x0027,0x0022,0x0033,0x0036,0x0013,0x0024,
	0x0036,0x0033,0x0034,0x0035,0x0036,0x0048,0x0036,0x0029,
	0x0033,0x0028,0x0024,0x0034,0x0033,0x0022,0x000E,0xFFE5,
	0xFFD4,0xFFD8,0xFFD2,0xFFCE,0xFFD0,0xFFC8,0xFFBF,0xFFC1,
	0xFFAB,0xFF97,0xFF86,0xFF7D,0xFF77,0xFF69,0xFF5C,0xFF58,
	0xFF49,0xFF48,0xFF30,0xFEFC,0xFEDD,0xFED1,0xFEC4,0xFEBF,
	0xFEC9,0xFEB9,0xFEA5,0xFE91,0xFE85,0xFE70,0xFE40,0xFE17,
	0xFE04,0xFDF9,0xFDFA,0xFDEF,0xFDDE,0xFDE3,0xFDDB,0xFDB5,
	0xFD9F,0xFDA0,0xFD95,0xFD9D,0xFDBF,0xFDE8,0xFE33,0xFE7C,
	0xFEBD,0xFEE1,0xFEF7,0xFF2A,0xFF79,0xFFB6,0x0008,0x006F,
	0x00CF,0x013D,0x01A6,0x01F3,0x0230,0x0247,0x0256,0x026A,
	0x0287,0x02A7,0x02C7,0x02DC,0x02CF,0x02BD,0x02B8,0x02A0,
	0x027A,0x0257,0x0229,0x0206,0x01EB,0x01FF,0x01FF,0x01DB,
	0x01C1,0x01AF,0x017C,0x0154,0x0146,0x012E,0x011C,0x0123,
	0x0132,0x011F,0x0105,0x00EE,0x00C6,0x008B,0x005B,0x003A,
	0x0023,0x000E,0x000C,0xFFF4,0xFFCF,0xFFAC,0xFF80,0xFF50,
	0xFF30,0xFF00,0xFEDA,0xFEB9,0xFE88,0xFE62,0xFE50,0xFE45,

};
#endif
#if 1
OpusEncoder * opus_para_init()
{
	write_stack(10 * 1024);
	int err;
	OpusEncoder* enc = NULL;

	int application = OPUS_APPLICATION_AUDIO;
	int channels;
	opus_int32 sampling_rate;
	sampling_rate = OPUS_ENC_FS;
	channels = OPUS_ENC_CHANNELS;
	application = OPUS_APPLICATION_RESTRICTED_LOWDELAY;
	/* defaults: */

	enc = opus_encoder_create(sampling_rate, channels, application, &err);

	return enc;
}
#else
short inin[OPUS_ENC_FRAMESIZE * OPUS_ENC_CHANNELS];
int opus_init()
{
	write_stack(10 * 1024);
	int err;
	char* inFile, * outFile;
	OpusEncoder* enc = NULL;
	OpusDecoder* dec = NULL;
	int args;
	int len[2];
	int frame_size, channels;
	opus_int32 bitrate_bps = 16000;
	unsigned char* data[2] = { NULL, NULL };
	unsigned char* fbytes = NULL;
	opus_int32 sampling_rate;
	int use_vbr;
	int max_payload_bytes;
	int complexity;
	int use_inbandfec;
	int use_dtx;
	int forcechannels;
	int cvbr = 0;
	int packet_loss_perc;
	opus_int32 count = 0, count_act = 0;
	int k;
	opus_int32 skip = 0;
	int stop = 0;
	short* in = NULL;
	short* out = NULL;
	int application = OPUS_APPLICATION_AUDIO;
	double bits = 0.0, bits_max = 0.0, bits_act = 0.0, bits2 = 0.0, nrg;
	double tot_samples = 0;
	opus_int32 tot_in = 0, tot_out;
	int bandwidth = OPUS_AUTO;
	const char* bandwidth_string;
	int lost = 0, lost_prev = 1;
	int toggle = 0;
	opus_uint32 enc_final_range[2];
	opus_uint32 dec_final_range;
	int encode_only = 0, decode_only = 0;
	int max_frame_size = OPUS_ENC_FRAMESIZE;	// 20ms
	size_t num_read;
	int curr_read = 0;
	int sweep_bps = 0;
	int random_framesize = 0, newsize = 0, delayed_celt = 0;
	int sweep_max = 0, sweep_min = 0;
	int random_fec = 0;
	const int(*mode_list)[4] = NULL;
	int nb_modes_in_list = 0;
	int curr_mode = 0;
	int curr_mode_count = 0;
	int mode_switch_time = 48000;
	int nb_encoded = 0;
	int remaining = 0;
	int variable_duration = OPUS_FRAMESIZE_ARG;
	int delayed_decision = 0;
	int ret = -1;
	encode_only = (OPUS_CODEC_MODE == OPUS_CODEC_ENCODE);
	decode_only = (OPUS_CODEC_MODE == OPUS_CODEC_DECODE);
	sampling_rate = OPUS_ENC_FS;
	frame_size = sampling_rate / 50;
	channels = OPUS_ENC_CHANNELS;
	application = OPUS_APPLICATION_RESTRICTED_LOWDELAY;
	/* defaults: */
	use_vbr = 0;
	max_payload_bytes = MAX_PACKET;
	complexity = 0;
	use_inbandfec = 0;
	forcechannels = OPUS_AUTO;
	use_dtx = 0;
	packet_loss_perc = 0;

	if (!decode_only)
	{
		enc = opus_encoder_create(sampling_rate, channels, application, &err);
		skip = OPUS_ENC_FS / 400;
	}
	in = inin;
	memset(data0Buf, 0, sizeof(data0Buf));
	memset(data1Buf, 0, sizeof(data1Buf));
	data[0] = (unsigned char*)data0Buf;
	if (use_inbandfec) {
		data[1] = (unsigned char*)data1Buf;
	}
//	while (!stop)
	if(1)
	{
		int i;
		if (tot_in + frame_size - remaining >= CODEC_ENC_SIZE) {
			num_read = CODEC_ENC_SIZE - tot_in;
		}
		else {
			num_read = frame_size - remaining;
		}
		memcpy(in + remaining * channels, inbuf + tot_in, num_read * 2);
		curr_read = (int)num_read;
		tot_in += curr_read;
		for (i = 0; i < curr_read * channels; i++)
		{
			opus_int32 s;
			s = in[i];
			//                s=((s&0xFFFF)^0x8000)-0x8000;
			in[i + remaining * channels] = s;
		}
		if (curr_read + remaining < frame_size)
		{
			for (i = (curr_read + remaining) * channels; i < frame_size * channels; i++)
				in[i] = 0;
			if (encode_only || decode_only)
				stop = 1;
		}
		memcpy(debug_bufin,in,640);
		DBG_CHN0_TOGGLE;
		len[toggle] = opus_encode(enc, in, frame_size, data[toggle], max_payload_bytes);
		DBG_CHN0_TOGGLE;
		memcpy(debug_dat0,len[toggle],1);
		memcpy(debug_bufout,data[toggle],40);
#if 1
		printf("output bytes=%d", len[toggle]);
		printf("\n");
		for (int i = 0; i < len[0]; i++)
			printf("%x,", data[0][i]);
		printf("\n");
#endif
		toggle = (toggle + use_inbandfec) & 1;
	}
	ret = 0;
	opus_decoder_destroy(dec);
	free(data[0]);
	free(data[1]);
	free(in);
	free(out);
	free(fbytes);
	return ret;
}
#endif