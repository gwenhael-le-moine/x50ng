#ifndef _X49GP_BYTEORDER_H
#define _X49GP_BYTEORDER_H 1

#include <stdint.h>
#include <sys/types.h>

static __inline__ uint16_t swab16( uint16_t x ) { return ( ( x & 0xff00 ) >> 8 ) | ( ( x & 0x00ff ) << 8 ); }

static __inline__ uint32_t swab32( uint32_t x )
{
    return ( ( x & 0xff000000 ) >> 24 ) | ( ( x & 0x00ff0000 ) >> 8 ) | ( ( x & 0x0000ff00 ) << 8 ) | ( ( x & 0x000000ff ) << 24 );
}

#if __BYTE_ORDER == __LITTLE_ENDIAN

#  define le16_to_cpu( x ) ( x )
#  define cpu_to_le16( x ) ( x )
#  define le32_to_cpu( x ) ( x )
#  define cpu_to_le32( x ) ( x )

#  define load_le16( p ) ( *( p ) )
#  define store_le16( p, x ) ( *( p ) = ( x ) )
#  define load_le32( p ) ( *( p ) )
#  define store_le32( p, x ) ( *( p ) = ( x ) )

#elif __BYTE_ORDER == __BIG_ENDIAN

#  define le16_to_cpu( x ) swab16( x )
#  define cpu_to_le16( x ) swab16( x )
#  define le32_to_cpu( x ) swab32( x )
#  define cpu_to_le32( x ) swab32( x )

#  define load_le16( p ) __load_le16( p )
#  define store_le16( p, x ) __store_le16( p, x )
#  define load_le32( p ) __load_le32( p )
#  define store_le32( p, x ) __store_le32( p, x )

#else
#  error "Cannot determine host byteorder"
#endif

#endif /* !(_X49GP_BYTEORDER_H) */
