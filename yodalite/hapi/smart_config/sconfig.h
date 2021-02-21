#ifndef _SCONFIG_H_
#define _SCONFIG_H_
#include <stdint.h>

#ifdef __CHECKER__
#define __force __attribute__((force))
#define __bitwise __attribute__((bitwise))
#else
#define __force
#define __bitwise
#endif

typedef uint16_t __bitwise be16;
typedef uint16_t __bitwise le16;
typedef uint32_t __bitwise be32;
typedef uint32_t __bitwise le32;
typedef uint64_t __bitwise be64;
typedef uint64_t __bitwise le64;

typedef struct ieee80211_hdr {
	le16 frame_control;
	le16 duration_id;
	uint8_t addr1[6];
	uint8_t addr2[6];
	uint8_t addr3[6];
	le16 seq_ctrl;
	/* followed by 'u8 addr4[6];' if ToDS and FromDS is set in data frame
	 */
} ieee80211_hdr_t;

#endif
