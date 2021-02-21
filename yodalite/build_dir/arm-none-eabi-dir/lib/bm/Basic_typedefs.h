/******************************************************************************
 *   Filename    : Basic_typedefs.h                                           *
 *   Start date  : 2002. 10. 26.                                              *
 *   By          : Taejin Kwon                                                *
 *   Contact     : tj1kwon@samsung.co.kr                                      *
 *   Description : Type defines for this project                              *
 *                some of this definitions comes from API guide document.     *
 ******************************************************************************
 */
#ifndef	_BASIC_TYPEDEFS_H_
#define	_BASIC_TYPEDEFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_IRTOS)
#ifndef __UCHAR
#define __UCHAR
typedef	unsigned char	UCHAR;
#endif

#ifndef	__USHORT
#define __USHORT
typedef	unsigned short	USHORT;
#endif

#ifndef __UINT
#define __UINT
typedef unsigned int	UINT;
#endif

#ifndef __ULONG
#define __ULONG
typedef unsigned long	ULONG;
#endif

#ifndef __ULLONG
#define	__ULLONG
typedef	unsigned long long		ULLONG;
#endif


#endif
/*********************************************
 *   Data types
 *********************************************/
#ifndef __NCHAR
#define	__CHAR
typedef char					NCHAR;
typedef char					CHAR;
#endif
#ifndef __SINT
#define	__SINT
typedef signed int				SINT;
#endif
#ifndef __SLONG
#define	__SLONG
typedef signed long				SLONG;
#endif
#ifndef __SCHAR
#define	__SCHAR
typedef signed char				SCHAR;
#endif
#ifndef __SSHORT
#define	__SSHORT
typedef signed short			SSHORT;
#endif
#ifndef __WCHAR
#define __WCHAR
typedef unsigned short			WCHAR;
#endif
#ifndef __UINT
#define	__UINT
typedef unsigned int			UINT;
#endif
#ifndef __ULONG
#define	__ULONG
typedef unsigned long			ULONG;
#endif
#ifndef __UCHAR
#define	__UCHAR
typedef unsigned char			UCHAR;
#endif
#ifndef __USHORT
#define	__USHORT
typedef unsigned short			USHORT;
#endif
#ifndef __FLOAT
#define __FLOAT
typedef float					FLOAT;
#endif
#ifndef __DOUBLE
#define __DOUBLE
typedef double					DOUBLE;
#endif
#ifndef __LDOUBLE
#define	__LDOUBLE
typedef long double				LDOUBLE;
#endif
#ifndef __BOOL
#define	__BOOL
typedef SINT					BOOL;
#endif
#ifndef __BYTE1
#define	__BYTE1
typedef UCHAR					BYTE1;
#endif
#ifndef __BYTE2
#define	__BYTE2
typedef USHORT					BYTE2;
#endif
#ifndef __BYTE4
#define	__BYTE4
typedef UINT					BYTE4;
#endif
#ifndef __BYTE8
#define	__BYTE8
typedef unsigned long long		BYTE8;
#endif
#ifndef __SBYTE1
#define	__SBYTE1
typedef SCHAR					SBYTE1;
#endif
#ifndef __SBYTE2
#define	__SBYTE2
typedef SSHORT					SBYTE2;
#endif
#ifndef __SBYTE4
#define	__SBYTE4
typedef SINT					SBYTE4;
#endif
#ifndef __LBA
#define	__LBA
typedef UINT					LBA;
#endif
#ifndef __COPYINFO
#define	__COPYINFO
typedef UCHAR					COPYINFO;
#endif
#ifndef __ADMINT
#define	__ADMINT
typedef USHORT					ADMINT;
#endif
#ifndef __VOID
#define	__VOID
typedef void					VOID;
#endif

/*********************************************
 *   Define
 *********************************************/
#define TRUE				1
#define FALSE				0

#ifndef __CONST
#define __CONST
#define CONST					const
#endif
#ifndef __REGISTER
#define __REGISTER
#define REGISTER				register
#endif
#ifndef __VOLATILE
#define __VOLATILE
#define VOLATILE				volatile
#endif
#ifndef __STATIC
#define __STATIC
#define STATIC					static
#endif
/*
 * To support ARM's __inline technology
 */
#ifdef	_ARM
#define INLINE	__inline
#define PURE	__pure
#else
#define INLINE
#define PURE
#endif

typedef UINT	FS_File_t;

typedef enum
{
	FAT_NO_ERROR = 0,
	FAT_READ_ERROR,
#if 1	// FS_SUPPORT_FAT_WRITE
	FAT_WRITE_ERROR,
	FAT_NOTFOUND_ERROR,
	FAT_TOO_MANY_ENTRY_ERROR,
	FAT_DISK_FULL_ERROR,
#endif  /* FS_SUPPORT_FAT_WRITE */
	FAT_INIT_ERROR,
	FAT_TERM_ERROR
}FATError_t;

#ifdef __cplusplus
}
#endif

#endif /* _BASIC_TYPEDEFS_H_ */
