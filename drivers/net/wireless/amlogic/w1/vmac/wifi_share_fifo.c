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
#ifdef HAL_SIM_VER
#ifdef FW_NAME
namespace FW_NAME
{
#endif
#endif

#include "wifi_hal_com.h"

/*
 ****************************************************************************************
 * function : CO_SharedFifoInit
 *
 * parameters:
 *    struct _CO_SHARED_FIFO *SharedFifoPtr  Pointer to the shared FIFO to deal with
 *    dma_addr_t  BufPhy    Physical address of the buffer pool
 *    void  *BufPtr  Pointer to the buffer pool
 *    unsigned int EltNbr Number of available elements
 *    unsigned int EltSize Size of one element
 *    unsigned char BlockNbr Number of block sharing the FIFO
 *
 * returns:
 *    none
 *
 * description:
 *    Initialize a shared FIFO Control Block
 *
 ****************************************************************************************
 */
void CO_SharedFifo_Dump(struct _CO_SHARED_FIFO* const  SharedFifoPtr, const unsigned char BlockID)
{
    struct _CO_SF_BLOCK_IDX *BlockOutIdxPtr;
    BlockOutIdxPtr = &SharedFifoPtr->IdxTab[ BlockID ];
    pr_debug("b%d I%d,O%d\n",BlockID,BlockOutIdxPtr->In,BlockOutIdxPtr->Out);
}

void qfifo_mng_info_dump(struct _CO_SHARED_FIFO* const  SharedFifoPtr)
{
    int i_block = 0;
    for (i_block = 0; i_block < 3; i_block++)
        CO_SharedFifo_Dump(SharedFifoPtr, i_block);
}

void CO_SharedFifoInit(struct _CO_SHARED_FIFO  *SharedFifoPtr, SYS_TYPE EltBasePhy, void *EltBasePtr, unsigned int EltNbr,
    unsigned int EltSize, unsigned char BlockNbr)
{
    int i; /* Loop counter*/

    /* Initialize the main features of the shared FIFO */
    /* Physical address of the start of the buffer pool,*/
    /* Pointer to the start of the buffer pool, Length of each page, number of page*/
    SharedFifoPtr->EltBasePhy = EltBasePhy;
    SharedFifoPtr->EltBasePtr = EltBasePtr;
    SharedFifoPtr->EltNbr     = EltNbr;
    SharedFifoPtr->EltSize    = EltSize;
    SharedFifoPtr->BlockNbr   = BlockNbr;

    /* Initialize all block indexes*/
    /* All indexes are set to the value 1*/
    for (i = CO_SF_PROD_ID; i < BlockNbr; i++)
    {
        SharedFifoPtr->IdxTab[i].Flag = 0;
        SharedFifoPtr->IdxTab[i].In = 1;
        SharedFifoPtr->IdxTab[i].Out = 1;
    }

    /* Reset the index IN of the producer to let one page empty (unused) all the time.  */
    /* Note that all tests (to know if an element is available) will be done easier     */
    SharedFifoPtr->IdxTab[CO_SF_PROD_ID].In = 0;
}

/*
 *****************************************************************************************
 * function : CO_SharedFifoGetPtr
 *
 * parameters:
 *    struct _CO_SHARED_FIFO *SharedFifoPtr  Pointer to the shared FIFO to deal with
 *    unsigned int  Idx Index to be converted
 *
 * returns:
 *    unsigned char * Computed pointer
 *
 * description:
 *    Knowing the index of a buffer, return a pointer to it ( virtual for the host )
 *
 *****************************************************************************************
 */
static unsigned char *CO_SharedFifoGetPtr(const struct _CO_SHARED_FIFO *const SharedFifoPtr, const unsigned int Idx)
{
    ASSERT(Idx < SharedFifoPtr->EltNbr) ;

    /* Calculate and return the physical address of the corresponding buffer */
    return (unsigned char*)((unsigned long)(SharedFifoPtr->EltBasePtr)
                 + (unsigned long)(Idx * SharedFifoPtr->EltSize));
}

/*
 *****************************************************************************************
 * function      : CO_SharedFifoGet
 *
 * parameters:
 *    struct _CO_SHARED_FIFO   *SharedFifoPtr  Pointer to the shared FIFO to deal with
 *    unsigned char BlockID Identifier of the block
 *    unsigned char NbElt  Number of element
 *    unsigned char**  ReturnEltPtr   Pointer to the buffer allocated (OUT)
 *
 * returns:
 *    unsigned short   CO_BAD_PARAM2 / CO_NO_ELT_IN_USE / CO_STATUS_OK
 *
 * description:
 *    Get N elements from the queue and return a pointer on the first element.
 *    This function doesn't manage the buffer wrap. So, return an error code if NbElt is
 *    bigger than the number of elements owned by the block.
 *
 *****************************************************************************************
 */
unsigned short CO_SharedFifoGet(struct _CO_SHARED_FIFO* const SharedFifoPtr, const unsigned char BlockID,
    const unsigned int  NbElt, unsigned char**  ReturnEltPtr )
{
    unsigned short Status;

    AL_BEGIN_CRITICAL_PATH;
    Status = CO_SharedFifoGet_NoCritSect( SharedFifoPtr, BlockID, NbElt, ReturnEltPtr );
    AL_END_CRITICAL_PATH;

    return Status;
}

/*
 *****************************************************************************************
 * function      : CO_SharedFifoGet_NoCritSect
 *
 * parameters:
 *    struct _CO_SHARED_FIFO   *SharedFifoPtr  Pointer to the shared FIFO to deal with
 *    unsigned char                BlockID        Identifier of the block
 *    unsigned char                NbElt          Number of element
 *    unsigned char**              ReturnEltPtr   Pointer to the buffer allocated (OUT)
 *
 * returns:
 *    unsigned short   CO_BAD_PARAM2 / CO_NO_ELT_IN_USE / CO_STATUS_OK
 *
 * description:
 *    Get N elements from the queue and return a pointer on the first element.
 *    This function doesn't manage the buffer wrap. So, return an error code if NbElt is
 *    bigger than the number of elements owned by the block.
 *
 *    The function can be called from an interrupt handler or inside a critical section.
 *    The function can be called from background task if the previous "block" performs
 *    a Put in background task.
 *
 *****************************************************************************************
 */

unsigned short
CO_SharedFifoGet_NoCritSect(struct _CO_SHARED_FIFO* const  SharedFifoPtr, const unsigned char BlockID,
    const unsigned int  NbElt, unsigned char**  ReturnEltPtr)
{
    struct _CO_SF_BLOCK_IDX* BlockOutIdxPtr;// Block indexes of the specified block
    unsigned short Status;// Returned status
    unsigned int TmpIdx;// temp shifted index
    unsigned int NbBlocElt; // Number of element owned by the block
    {
        BlockOutIdxPtr = &SharedFifoPtr->IdxTab[ BlockID ];

        NbBlocElt = CO_SharedFifoNbEltCont( SharedFifoPtr, BlockID );
        if ( NbElt > NbBlocElt )
        {
            Status = CO_NO_ELT_IN_USE;
            *ReturnEltPtr =NULL;
        }
        else
        {
            // Get the pointer from the OUT index of the producer block
            *ReturnEltPtr = CO_SharedFifoGetPtr( SharedFifoPtr, BlockOutIdxPtr->Out );
            // Shift the OUT index
            TmpIdx = BlockOutIdxPtr->Out + NbElt;
            // If the OUT index is greater than the number of page, jump to the index 0
            if ( TmpIdx >= SharedFifoPtr->EltNbr )
            {
                BlockOutIdxPtr->Out = TmpIdx - SharedFifoPtr->EltNbr;
            }
            else
            {
                BlockOutIdxPtr->Out = TmpIdx;
            }
            Status = CO_STATUS_OK ;
        }
    }
    return Status;
}

unsigned short CO_SharedFifo_Pre_Get(struct _CO_SHARED_FIFO* const SharedFifoPtr, const unsigned char BlockID,
     const unsigned int NbElt, unsigned char** ReturnEltPtr)
{
    unsigned short Status;

    AL_BEGIN_CRITICAL_PATH;
    Status = CO_SharedFifo_Pre_Get_NoCritSect( SharedFifoPtr, BlockID, NbElt, ReturnEltPtr );
    AL_END_CRITICAL_PATH;

    return Status;
}

unsigned short CO_SharedFifo_Pre_Get_NoCritSect(struct _CO_SHARED_FIFO* const  SharedFifoPtr, const unsigned char BlockID,
    const unsigned int NbElt,  unsigned char** ReturnEltPtr )
{
    struct _CO_SF_BLOCK_IDX* BlockOutIdxPtr;// Block indexes of the specified block
    unsigned short Status; // Returned status
    unsigned int NbBlocElt; // Number of element owned by the block
    {
        // Recover specific block indexes
        BlockOutIdxPtr = &SharedFifoPtr->IdxTab[ BlockID ];

        NbBlocElt = CO_SharedFifoNbEltCont( SharedFifoPtr, BlockID);
        // Check there's enough contiguous elements enqueued in this block

        if ( NbElt > NbBlocElt )
        {
            Status = CO_NO_ELT_IN_USE;
            *ReturnEltPtr =NULL;
        }
        else
        {
            if (BlockOutIdxPtr->Out ==0)
                Status = CO_IDX_OUT_OF_RANGE ;
            else
                Status = CO_STATUS_OK ;
        }
    }
    return Status;
}

/*
 *****************************************************************************************
 * function      : CO_SharedFifoGetNElt
 *
 * parameters:
 *    struct _CO_SHARED_FIFO   *SharedFifoPtr  Pointer to the shared FIFO to deal with
 *    unsigned char BlockID Identifier of the block
 *    unsigned char NbElt Number of element
 *    unsigned char** ReturnEltPtr   Pointer to the buffer allocated (OUT)
 *
 * returns:
 *    unsigned short   CO_BAD_PARAM2 / CO_NO_ELT_IN_USE / CO_STATUS_OK
 *
 * description:
 *    Get N elements from the queue and return a pointer on the first element.
 *    This function doesn't manage the buffer wrap. So, return an error code if NbElt is
 *    bigger than the number of elements owned by the block.
 *
 *****************************************************************************************
 */
void CO_SharedFifoGetNElt(struct _CO_SHARED_FIFO *const SharedFifoPtr, const unsigned char BlockID,
    const unsigned int NbElt, unsigned char **ReturnEltPtr)
{
    struct _CO_SF_BLOCK_IDX  *BlockOutIdxPtr;   /* Block indexes of the specified block */
    unsigned int  TmpIdx;  /* temp shifted index */

    /* Critical section as we can't be preempted between the test and the pointer's */
    /* modification*/
    AL_BEGIN_CRITICAL_PATH;

    /* Recover specific block indexes */
    BlockOutIdxPtr = &(SharedFifoPtr->IdxTab[BlockID]);

    /* Get the pointer from the OUT index of the producer block */
    *ReturnEltPtr = CO_SharedFifoGetPtr(SharedFifoPtr, BlockOutIdxPtr->Out);

    /* Shift the OUT index */
    TmpIdx = BlockOutIdxPtr->Out + NbElt;

    /* If the OUT index is greater than the number of page, jump to the index 0*/
    if (TmpIdx >= SharedFifoPtr->EltNbr)
    {
        BlockOutIdxPtr->Out = TmpIdx - SharedFifoPtr->EltNbr;
    }
    else
    {
        /* Update the OUT index */
        BlockOutIdxPtr->Out = TmpIdx;
    }

    AL_END_CRITICAL_PATH;
}

/*
 *****************************************************************************************
 * function      : CO_SharedFifoPut
 *
 * parameters:
 *    struct _CO_SHARED_FIFO   *SharedFifoPtr  Pointer to the shared FIFO to deal with
 *    unsigned char  BlockID Identifier of the block
 *    unsigned char  NbElt Number of element
 *
 * returns:
 *    unsigned short   CO_BAD_PARAM2 / CO_STATUS_OK
 *
 * description:
 *    Let available N elements of the shared FIFO for the next block.
 *
 *****************************************************************************************
 */
unsigned short CO_SharedFifoPut(struct _CO_SHARED_FIFO * const SharedFifoPtr, unsigned char const BlockID, unsigned int const NbElt)
{
    unsigned short Status; /* Returned status */

    /* start preventing concurrent accesses */
    AL_BEGIN_CRITICAL_PATH;

    /* invoke the no critical section function */
    Status = CO_SharedFifoPut_NoCritSect(SharedFifoPtr, BlockID, NbElt);

    /* finish preventing concurrent accesses */
    AL_END_CRITICAL_PATH;

    return Status;
}

/*
 *****************************************************************************************
 * function      : CO_SharedFifoPut_NoCritSect
 *
 * parameters:
 *    struct _CO_SHARED_FIFO   *SharedFifoPtr  Pointer to the shared FIFO to deal with
 *    unsigned char BlockID  Identifier of the block
 *    unsigned char NbElt  Number of element
 *
 * returns:
 *    unsigned short   CO_BAD_PARAM2 / CO_STATUS_OK
 *
 * description:
 *    Let available N elements of the shared FIFO for the next block.
 *
 *    The function can be called from an interrupt handler or inside a critical section.
 *    The function can be called from background task if the next "block" performs
 *    a Get in background task.
 *
 *****************************************************************************************
 */
unsigned short CO_SharedFifoPut_NoCritSect(struct _CO_SHARED_FIFO * const SharedFifoPtr,
    unsigned char const BlockID, unsigned int const NbElt)
{
    struct _CO_SF_BLOCK_IDX  *BlockInIdxPtr; /* Block indexes of the specified block */
    unsigned int TmpIdx; /* Temp shifted index */
    unsigned short Status = CO_STATUS_OK; /* Returned status */
    unsigned char NextBlockID = BlockID + 1; /* Next block ID (concerned by the Put) */

    /* Avoid putting zero element */
    //ASSERT(NbElt != 0);

    if (NextBlockID == SharedFifoPtr->BlockNbr)
    {
        NextBlockID = CO_SF_PROD_ID;
    }
    /* Recover specific block indexes */
    BlockInIdxPtr = CO_SharedFifoGetInBlockIdxPtr(SharedFifoPtr, NextBlockID);

    /* Shift the IN index */
    TmpIdx = BlockInIdxPtr->In + NbElt;

    /* If the IN index is greater than the number of page, restart at the index 0   */
    if (TmpIdx >= SharedFifoPtr->EltNbr)
    {
        BlockInIdxPtr->In = TmpIdx - SharedFifoPtr->EltNbr;
    }
    else
    {
        /* Update the IN index */
        BlockInIdxPtr->In = TmpIdx;
    }

    return Status;
}

/*
 *****************************************************************************************
 * function      : CO_SharedFifoNbElt
 *
 * parameters:
 *    struct _CO_SHARED_FIFO *SharedFifoPtr   Pointer to the shared FIFO to deal with
 *    unsigned char BlockID Identifier of the block
 *
 * returns:
 *    unsigned int
 *
 * description:
 *    Compute the number of element owned by the specified block.
 *  Note that for the producer block, compute the number of elements available (not owned)
 *
 *****************************************************************************************
 */
unsigned int CO_SharedFifoNbElt(struct _CO_SHARED_FIFO *SharedFifoPtr, unsigned char BlockID)
{
    /* Embedded Processor */
    unsigned int NbElt;/* Returned value */
    struct _CO_SF_BLOCK_IDX *BlockIdxPtr; /* block indexes  */

    BlockIdxPtr = &(SharedFifoPtr->IdxTab[BlockID]);

    /* Compute the number of pages belonging to the specified block*/
    NbElt = SharedFifoPtr->EltNbr + BlockIdxPtr->In - BlockIdxPtr->Out;

    if (NbElt >= SharedFifoPtr->EltNbr)
    {
        NbElt -= SharedFifoPtr->EltNbr;
    }

    return NbElt;
}

/*
 *****************************************************************************************
 * function      : CO_SharedFifoNbEltCont
 *
 * parameters:
 *    struct _CO_SHARED_FIFO *SharedFifoPtr   Pointer to the shared FIFO to deal with
 *    unsigned char  BlockID Identifier of the block
 *
 * returns:
 *    unsigned int
 *
 * description:
 *    Compute the number of contiguous element owned by the specified block
 *  Note that for the producer block, compute the number of elements available (not owned)
 *
 *****************************************************************************************
 */
unsigned int CO_SharedFifoNbEltCont_NonRound(struct _CO_SHARED_FIFO *SharedFifoPtr, unsigned char BlockID)
{
    /* Embedded Processor */
    unsigned int NbElt; /* Returned value */
    struct _CO_SF_BLOCK_IDX *BlockIdxPtr; /* block indexes */

    BlockIdxPtr = &(SharedFifoPtr->IdxTab[BlockID]);
    /* Compute the number of elements belonging to the specified block */
    if (BlockIdxPtr->In >= BlockIdxPtr->Out)
    {
        /* Elements belonged by the block are contiguous*/
        NbElt = BlockIdxPtr->In - BlockIdxPtr->Out;
    }
    else
    {
        /* Elements belonged by the block are "cut" by the end of the buffer*/
        NbElt = SharedFifoPtr->EltNbr - BlockIdxPtr->Out;
    }

    return NbElt;
}

unsigned int CO_SharedFifoNbEltCont(struct _CO_SHARED_FIFO *SharedFifoPtr, unsigned char BlockID)
{
    /* Embedded Processor */
    unsigned int NbElt; /* Returned value */
    struct _CO_SF_BLOCK_IDX *BlockIdxPtr; /* block indexes */

    BlockIdxPtr = &(SharedFifoPtr->IdxTab[BlockID]);
    /* Compute the number of elements belonging to the specified block  */
    if (BlockIdxPtr->In >= BlockIdxPtr->Out)
    {
        /* Elements belonged by the block are contiguous*/
        NbElt = BlockIdxPtr->In - BlockIdxPtr->Out;
    }
    else
    {
        /* Elements belonged by the block are "cut" by the end of the buffer        */
        NbElt = SharedFifoPtr->EltNbr + (BlockIdxPtr->In - BlockIdxPtr->Out);
    }

    return NbElt;
}

/*
 *****************************************************************************************
 * function  : CO_SharedFifoEmpty
 *
 * parameters:
 *    struct _CO_SHARED_FIFO *SharedFifoPtr   Pointer to the shared FIFO to deal with
 *    unsigned char BlockID   Identifier of the block
 *
 * returns:
 *    unsigned int
 *
 * description:
 *    Return true if there is at least 1 FREE element
 *
 *****************************************************************************************
 */
unsigned char CO_SharedFifoEmpty
    (struct _CO_SHARED_FIFO *SharedFifoPtr, unsigned char BlockID)
{
    unsigned char is_empty;
    struct _CO_SF_BLOCK_IDX *BlockIdxPtr = &(SharedFifoPtr->IdxTab[BlockID]);  /* block indexes*/

    AL_BEGIN_CRITICAL_PATH;
    is_empty = (BlockIdxPtr->In != BlockIdxPtr->Out);
    AL_END_CRITICAL_PATH;

    return is_empty;
}

#ifdef HAL_SIM_VER
#ifdef FW_NAME
}
#endif
#endif
