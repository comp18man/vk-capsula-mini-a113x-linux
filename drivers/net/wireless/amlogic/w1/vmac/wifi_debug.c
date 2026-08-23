
#include "wifi_debug.h"


void address_print( unsigned char* address )
{
    PUTX8(1, address[0] );
    PUTC( ':' );
    PUTX8( 1, address[1] );
    PUTC( ':' );
    PUTX8( 1, address[2] );
    PUTC( ':' );
    PUTX8( 1, address[3] );
    PUTC( ':' );
    PUTX8( 1, address[4] );
    PUTC( ':' );
    PUTX8( 1, address[5] );
    PUTC( '\n' );
}

void IPv4_address_print( unsigned char* address )
{
    PRINT("IPv4 address=");
    PUTU8( address[0] );
    PUTC( '.' );
    PUTU8( address[1] );
    PUTC( '.' );
    PUTU8( address[2] );
    PUTC( '.' );
    PUTU8( address[3] );
    PUTC('\n');
}

  void dump_memory_internal(unsigned char *data,int len)
{

    unsigned char *cursor=data;
    char *xcursor = (char *)data;
    int i,j;
    pr_debug("\n*********************************\n");
    for (  i = 0; i < len; i++ )
    {
        if (( i != 0)&&(( i & 0x0F ) == 0 ))
        {

#ifdef  ASCII_IN
            pr_debug("|");
            for (j=0; j < len; j++)
            {
                if (((*xcursor)<' ')||((*xcursor)>'~'))
                {
                    pr_debug(".");
                    xcursor++;
                }
                else
                {
                    pr_debug("%c",*xcursor++);
                }
            }
#endif
            pr_debug( "\n");
        }

        pr_debug( "%02x",*cursor++ );
        pr_debug(" ");
    }

#ifdef  ASCII_IN
    if (len>LINEBYTE)
    {
        for (i=0; i<LINEBYTE-(len%LINEBYTE); i++)
        {
            pr_debug("   ");
        }
        pr_debug("|");
        for (i=0; i<(len%LINEBYTE); i++)
        {
            if (((*xcursor)<' ')||((*xcursor)>'~'))
            {
                pr_debug(".");
                xcursor++;
            }
            else
            {
                pr_debug("%c",*xcursor++);
            }
        }

    }
#endif
    pr_debug("\n*********************************\n");
}

void address_read( unsigned char* cursor, unsigned char* address )
{
    *address++ = *cursor++;
    *address++ = *cursor++;
    *address++ = *cursor++;
    *address++ = *cursor++;
    *address++ = *cursor++;
    *address++ = *cursor++;
}




 unsigned short READ_16L( const unsigned char* address )
{
#ifdef CTRL_BYTE
    return address[0] | ( address[1] << 8 );
#else
    return * (unsigned short *)address ;
#endif
}


 void WRITE_16L( unsigned char* address, unsigned short value )
{
#ifdef CTRL_BYTE
    address[0] = ( value >> 0 ) & 0xFF;
    address[1] = ( value >> 8 ) & 0xFF;
#else
    * (unsigned short *)address = value;
#endif
}


unsigned int READ_32L( const unsigned char* address )
{
#ifdef CTRL_BYTE
    return address[0] | ( address[1] << 8 ) | ( address[2] << 16 ) | ( address[3] << 24 );

#else
    return * (unsigned int *)address ;

#endif
}


 void WRITE_32L( unsigned char* address, unsigned int value )
{
#ifdef CTRL_BYTE
    address[0] = ( value >>  0 ) & 0xFF;
    address[1] = ( value >>  8 ) & 0xFF;
    address[2] = ( value >> 16 ) & 0xFF;
    address[3] = ( value >> 24 ) & 0xFF;

#else
    * (unsigned int *)address = value;
#endif
}

unsigned short READ_16B( const unsigned char* address )
{
    return address[1] | ( address[0] << 8 );
}


void WRITE_16B( unsigned char* address, unsigned short value )
{
    address[1] = ( value >> 0 ) & 0xFF;
    address[0] = ( value >> 8 ) & 0xFF;
}


 unsigned int READ_32B( const unsigned char* address )
{
    return address[3] | ( address[2] << 8 ) | ( address[1] << 16 ) | ( address[0] << 24 );
}

 void WRITE_32B( unsigned char* address, unsigned int value )
{
    address[3] = ( value >>  0 ) & 0xFF;
    address[2] = ( value >>  8 ) & 0xFF;
    address[1] = ( value >> 16 ) & 0xFF;
    address[0] = ( value >> 24 ) & 0xFF;
}


void ie_dbg(unsigned char *ie ) 
{
    int i = 0;
   
    DPRINTF(AML_DEBUG_DEBUG, "ie: %s %d\n", __func__, __LINE__);
    for(i = 0; i < ie[1] + 2; i++)
    {
        DPRINTF(AML_DEBUG_DEBUG, "%s %d ie 0x%x\n", __func__, __LINE__, ie[i]);
    }
}



