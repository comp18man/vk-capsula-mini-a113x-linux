/*copyright (C) amlogic.com 2002-2006
 *
 * Project: amlogic Software
 *
 * Description:
 *      General shared FIFO management
 *
 * Revision History:
 * $Source: cosharedfifo.c,v
 *
 ****************************************************************************************
 */
#ifndef CO_SHARED_FIFO_H
#define CO_SHARED_FIFO_H

#ifdef HAL_FPGA_VER
#include "osdep.h"
#endif

#define CO_STATUS_GROUP_COMMON  (0 << 8)

/* COMMON STATUS CODE  */
#define CO_STATUS_OK  ((unsigned short)(CO_STATUS_GROUP_COMMON + 0))
#define CO_FAIL  ((unsigned short)(CO_STATUS_GROUP_COMMON + 1))
#define CO_BAD_PARAM  ((unsigned short)(CO_STATUS_GROUP_COMMON + 4))

#define CO_NO_ELT_IN_USE  ((unsigned short)(CO_STATUS_GROUP_COMMON + 12))
#define CO_IDX_OUT_OF_RANGE  ((unsigned short)(CO_STATUS_GROUP_COMMON + 13))
#define AL_BEGIN_CRITICAL_PATH  COMMON_LOCK()
#define  AL_END_CRITICAL_PATH  COMMON_UNLOCK()

/*
 ****************************************************************************************
 */
/* ID of the producer BlockID (the one which allocate and FREE) */
#define CO_SF_PROD_ID        0
#define CO_SF_BLOCK_DEF_NBR  2

/*
 * TYPES
 ****************************************************************************************
 */

#ifndef __INLINE
#endif

/* ERROR GROUP
*/
#ifndef CO_STATUS_GROUP_COMMON
//#define CO_STATUS_GROUP_COMMON                     0
#endif


/* Indexes for one BlockID */
struct _CO_SF_BLOCK_IDX
{
    unsigned int Flag;  /* Indicate if indexes are shared between both processors */
    unsigned int In;    /* Index of the next buffer to put  */
    unsigned int Out;   /* Index of the next buffer to get  */
} ;


//elt = element
struct _CO_SHARED_FIFO
{
    SYS_TYPE EltBasePhy; /* Physical address of the start of allocated memory  */
    void *EltBasePtr; /* Pointer to the start of allocated memory */
    unsigned int EltNbr; /* Nbr of elt which can takes place in allocated mem  */
    unsigned int EltSize; /* Size of each element */
    unsigned int BlockNbr; /* Number of Block sharing the FIFO */
    // struct semaphore mutex;
    struct _CO_SF_BLOCK_IDX IdxTab[CO_SF_BLOCK_DEF_NBR+2]; /* Indexes  */
} ;

/*
 * FUNCTION PROTOTYPES
 *****************************************************************************************
 */
void CO_SharedFifoInit(struct _CO_SHARED_FIFO* SharedFifoPtr, SYS_TYPE  EltBasePhy, void *EltBasePtr,
    unsigned int EltNbr, unsigned int EltSize, unsigned char BlockNbr);

unsigned short CO_SharedFifoPut(struct _CO_SHARED_FIFO* const SharedFifoPtr, unsigned char const BlockID,
    unsigned int const NbElt);

unsigned short CO_SharedFifoPut_NoCritSect(struct _CO_SHARED_FIFO* const SharedFifoPtr, unsigned char const BlockID,
    unsigned int const NbElt);

unsigned short CO_SharedFifoGet(struct _CO_SHARED_FIFO* const SharedFifoPtr, const unsigned char BlockID,
    const unsigned int NbElt, unsigned char **ReturnEltPtr);

unsigned short CO_SharedFifo_Pre_Get(struct _CO_SHARED_FIFO *const SharedFifoPtr, const unsigned char BlockID,
    const unsigned int NbElt, unsigned char **ReturnEltPtr);

unsigned short CO_SharedFifoGet_NoCritSect(struct _CO_SHARED_FIFO *const SharedFifoPtr, const unsigned char BlockID,
    const unsigned int NbElt, unsigned char **ReturnEltPtr);

unsigned short CO_SharedFifo_Pre_Get_NoCritSect(struct _CO_SHARED_FIFO *const SharedFifoPtr, const unsigned char BlockID,
    const unsigned int NbElt, unsigned char** ReturnEltPtr);

void CO_SharedFifoGetNElt(struct _CO_SHARED_FIFO *const SharedFifoPtr, const unsigned char BlockID, const unsigned int NbElt,
    unsigned char **ReturnEltPtr);

unsigned int CO_SharedFifoNbElt(struct _CO_SHARED_FIFO* SharedFifoPtr, unsigned char BlockID);
unsigned int CO_SharedFifoNbEltCont(struct _CO_SHARED_FIFO* SharedFifoPtr, unsigned char BlockID);
unsigned int CO_SharedFifoNbEltCont_NonRound(struct _CO_SHARED_FIFO *SharedFifoPtr, unsigned char BlockID);
void CO_SharedFifo_Dump(struct _CO_SHARED_FIFO* const  SharedFifoPtr, const unsigned char BlockID);
void qfifo_mng_info_dump(struct _CO_SHARED_FIFO* const  SharedFifoPtr);
unsigned char CO_SharedFifoEmpty(struct _CO_SHARED_FIFO *SharedFifoPtr, unsigned char BlockID);

/*
 *****************************************************************************************
 * function    : CO_SharedFifoGetInBlockIdxPtr
 *
 * parameters:
 *    struct _CO_SHARED_FIFO *SharedFifoPtr   Pointer to the shared FIFO to deal with
 *    unsigned char   BlockID   Block ID
 *
 * returns:
 *    struct _CO_SF_BLOCK_IDX * Dedicated pointer to the IN index
 *
 *
 * description:
 *    Return the pointer to the dedicated IN index. If the index is shared, get the
 *  mirrored index, else, get the local one.
 *
 *****************************************************************************************
 */
__INLINE struct _CO_SF_BLOCK_IDX *CO_SharedFifoGetInBlockIdxPtr(struct _CO_SHARED_FIFO *const SharedFifoPtr,
    const unsigned char BlockID)
{
    /* Embedded processor */
    return &(SharedFifoPtr->IdxTab[BlockID]);
}

/*
 *****************************************************************************************
 * function   : CO_SharedFifoPick
 *
 * parameters:
 *    struct _CO_SHARED_FIFO *SharedFifoPtr  Pointer to the shared FIFO to deal with
 *    unsigned char  BlockID  Identifier of the block
 *
 * returns:
 *     void
 *
 * description:
 *    Pick 1 elements from the queue and return a pointer on the first element.
 *    The returned element is not removed from the cosharedfifo (use CO_SharedFifoGet()
 *    to do so).
 *
 *****************************************************************************************
 */
__INLINE unsigned char * CO_SharedFifoPick
    (struct _CO_SHARED_FIFO *const SharedFifoPtr, const unsigned char BlockID)
{
    // ASSERT(CO_SharedFifoNbElt(SharedFifoPtr, BlockID) > 0) ;
    return (unsigned char*)((unsigned long)(SharedFifoPtr->EltBasePtr)
                 + (unsigned int)(SharedFifoPtr->IdxTab[BlockID].Out * SharedFifoPtr->EltSize));
}
#endif //CO_SHARED_FIFO_H
