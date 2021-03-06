/*
 *  version.h
 */

#define SND_LIB_MAJOR		0
#define SND_LIB_MINOR		4
#define SND_LIB_SUBMINOR	0
#define SND_LIB_VERSION		((SND_LIB_MAJOR<<16)|\
				 (SND_LIB_MINOR<<8)|\
				  SND_LIB_SUBMINOR)
#define SND_LIB_VERSION_STR	"0.4.0"

/* OBSOLETE DEFINES WHICH WON'T BE IN 0.3.0 FINAL!!! */
#define SOUNDLIB_VERSION_MAJOR		SND_LIB_MAJOR
#define SOUNDLIB_VERSION_MINOR		SND_LIB_MINOR
#define SOUNDLIB_VERSION_SUBMINOR	SND_LIB_SUBMINOR

