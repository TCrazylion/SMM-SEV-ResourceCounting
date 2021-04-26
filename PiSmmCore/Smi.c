/** @file
  SMI management.

  Copyright (c) 2009 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include "PiSmmCore.h"
#include <unistd.h>
#include <linux/types.h>

__u64 getcr3()
{
	__u64 value;

	__asm__ __volatile__(
	    "movq %%cr3, %0"
	    : "=r" (value));
	return (value);
}

__u64 rdtsc()
{
    __u32 lo, hi;

    __asm__ __volatile__ ("rdtsc": "=a"(lo), "=d"(hi));
    return (__u64)hi << 32 | lo;
}
//得到寄存器rdtsc的值

//得到全局页表CR3
/*
void write_tmp_t0()
{
	__u64 value;

	__asm__ __volatile__(
	    "movq %%dr3, %0"
	    : "=r" (value));
	return (value);
}
__u64 get_tmp_t0()
{
	__u64 value;

	__asm__ __volatile__(
	    "movq %%dr3, %0"
	    : "=r" (value));
	return (value);
}
//起始时间t（随便找的寄存器）

__u64 calculate_frequency(){
    __u64 t1 = rdtsc();
    usleep(1000000); //1s
    __u64 t2 = rdtsc();
    return (t2-t1)/1000000;
}
//计算CPU时钟周期
*/

// #define MY_BASE 0x8000
// #define MY_BS (*(volatile) unsigned int *)MY_BASE)
void my_smm(){
    __u64 t1 = rdtsc();
    //__u64 frequency=cal_fre();
    __u64 frequency=2800; //Mhz
    __u64 t2 = rdtsc();
    DEBUG((DEBUG_INIT, "execution time:%lld μs \n",(t2-t1)/(frequency)));
    __u64 cr3 = getcr3();
    DEBUG((DEBUG_INIT, "CR3:%lld \n",cr3));



  //u_int64_t t1 = rdtsc();
  //u_int64_t t2 = rdtsc();
  //DEBUG((DEBUG_INIT, "rdtsc:%d sec \n",(t2-t1)/1000000000));
  
  //FILE *fp = NULL;
  //fp=fopen("/tmp/test.txt","w+");
  //fprintf(fp,"test---fprintf");
  //fputs("test---fputs",fp);
  //fclose(fp);
}


LIST_ENTRY  mSmiEntryList       = INITIALIZE_LIST_HEAD_VARIABLE (mSmiEntryList);

SMI_ENTRY   mRootSmiEntry = {
  SMI_ENTRY_SIGNATURE,
  INITIALIZE_LIST_HEAD_VARIABLE (mRootSmiEntry.AllEntries),
  {0},
  INITIALIZE_LIST_HEAD_VARIABLE (mRootSmiEntry.SmiHandlers),
};

/**
  Finds the SMI entry for the requested handler type.

  @param  HandlerType            The type of the interrupt
  @param  Create                 Create a new entry if not found

  @return SMI entry

**/
SMI_ENTRY  *
EFIAPI
SmmCoreFindSmiEntry (
  IN EFI_GUID  *HandlerType,
  IN BOOLEAN   Create
  )
{
  LIST_ENTRY  *Link;
  SMI_ENTRY   *Item;
  SMI_ENTRY   *SmiEntry;

  //
  // Search the SMI entry list for the matching GUID
  //
  DEBUG((DEBUG_INIT, "Hello!TRZ! @entry\n"));
  
  SmiEntry = NULL;
  for (Link = mSmiEntryList.ForwardLink;
       Link != &mSmiEntryList;
       Link = Link->ForwardLink) {

    Item = CR (Link, SMI_ENTRY, AllEntries, SMI_ENTRY_SIGNATURE);
    if (CompareGuid (&Item->HandlerType, HandlerType)) {
      //
      // This is the SMI entry
      //
      SmiEntry = Item;
      break;
    }
  }

  //
  // If the protocol entry was not found and Create is TRUE, then
  // allocate a new entry
  //
  if ((SmiEntry == NULL) && Create) {
    SmiEntry = AllocatePool (sizeof(SMI_ENTRY));
    if (SmiEntry != NULL) {
      //
      // Initialize new SMI entry structure
      //
      SmiEntry->Signature = SMI_ENTRY_SIGNATURE;
      CopyGuid ((VOID *)&SmiEntry->HandlerType, HandlerType);
      InitializeListHead (&SmiEntry->SmiHandlers);

      //
      // Add it to SMI entry list
      //
      InsertTailList (&mSmiEntryList, &SmiEntry->AllEntries);
    }
  }
  return SmiEntry;
}

/**
  Manage SMI of a particular type.

  @param  HandlerType    Points to the handler type or NULL for root SMI handlers.
  @param  Context        Points to an optional context buffer.
  @param  CommBuffer     Points to the optional communication buffer.
  @param  CommBufferSize Points to the size of the optional communication buffer.

  @retval EFI_WARN_INTERRUPT_SOURCE_PENDING  Interrupt source was processed successfully but not quiesced.
  @retval EFI_INTERRUPT_PENDING              One or more SMI sources could not be quiesced.
  @retval EFI_NOT_FOUND                      Interrupt source was not handled or quiesced.
  @retval EFI_SUCCESS                        Interrupt source was handled and quiesced.
**/

EFI_STATUS
EFIAPI
SmiManage (
  IN     CONST EFI_GUID  *HandlerType,
  IN     CONST VOID      *Context         OPTIONAL,
  IN OUT VOID            *CommBuffer      OPTIONAL,
  IN OUT UINTN           *CommBufferSize  OPTIONAL
  )
{
  LIST_ENTRY   *Link;
  LIST_ENTRY   *Head;
  SMI_ENTRY    *SmiEntry;
  SMI_HANDLER  *SmiHandler;
  BOOLEAN      SuccessReturn;
  EFI_STATUS   Status;
  

  my_smm();


  Status = EFI_NOT_FOUND;
  SuccessReturn = FALSE;
  if (HandlerType == NULL) {
    //
    // Root SMI handler
    //
    SmiEntry = &mRootSmiEntry;
  } else {
    //
    // Non-root SMI handler
    //
    SmiEntry = SmmCoreFindSmiEntry ((EFI_GUID *) HandlerType, FALSE);
    if (SmiEntry == NULL) {
      //
      // There is no handler registered for this interrupt source
      //
      return Status;
    }
  }
  Head = &SmiEntry->SmiHandlers;

  for (Link = Head->ForwardLink; Link != Head; Link = Link->ForwardLink) {
    SmiHandler = CR (Link, SMI_HANDLER, Link, SMI_HANDLER_SIGNATURE);

    Status = SmiHandler->Handler (
               (EFI_HANDLE) SmiHandler,
               Context,
               CommBuffer,
               CommBufferSize
               );

    switch (Status) {
    case EFI_INTERRUPT_PENDING:
      //
      // If a handler returns EFI_INTERRUPT_PENDING and HandlerType is not NULL then
      // no additional handlers will be processed and EFI_INTERRUPT_PENDING will be returned.
      //
      if (HandlerType != NULL) {
        return EFI_INTERRUPT_PENDING;
      }
      break;

    case EFI_SUCCESS:
      //
      // If at least one of the handlers returns EFI_SUCCESS then the function will return
      // EFI_SUCCESS. If a handler returns EFI_SUCCESS and HandlerType is not NULL then no
      // additional handlers will be processed.
      //
      if (HandlerType != NULL) {
        return EFI_SUCCESS;
      }
      SuccessReturn = TRUE;
      break;

    case EFI_WARN_INTERRUPT_SOURCE_QUIESCED:
      //
      // If at least one of the handlers returns EFI_WARN_INTERRUPT_SOURCE_QUIESCED
      // then the function will return EFI_SUCCESS.
      //
      SuccessReturn = TRUE;
      break;

    case EFI_WARN_INTERRUPT_SOURCE_PENDING:
      //
      // If all the handlers returned EFI_WARN_INTERRUPT_SOURCE_PENDING
      // then EFI_WARN_INTERRUPT_SOURCE_PENDING will be returned.
      //
      break;

    default:
      //
      // Unexpected status code returned.
      //
      ASSERT (FALSE);
      break;
    }
  }

  if (SuccessReturn) {
    Status = EFI_SUCCESS;
  }
  
  //DEBUG((DEBUG_INIT, "Status: %p \n",Status));

  return Status;
}

/**
  Registers a handler to execute within SMM.

  @param  Handler        Handler service function pointer.
  @param  HandlerType    Points to the handler type or NULL for root SMI handlers.
  @param  DispatchHandle On return, contains a unique handle which can be used to later unregister the handler function.

  @retval EFI_SUCCESS           Handler register success.
  @retval EFI_INVALID_PARAMETER Handler or DispatchHandle is NULL.

**/
EFI_STATUS
EFIAPI
SmiHandlerRegister (
  IN  EFI_SMM_HANDLER_ENTRY_POINT2  Handler,
  IN  CONST EFI_GUID                *HandlerType  OPTIONAL,
  OUT EFI_HANDLE                    *DispatchHandle
  )
{
  SMI_HANDLER  *SmiHandler;
  SMI_ENTRY    *SmiEntry;
  LIST_ENTRY   *List;
  
  //int i=1;
  /*
  while (i>0){
  	i=1;
  }
  */
  
  //DEBUG((DEBUG_INIT, "Hello!TRZ! @1"));

  if (Handler == NULL || DispatchHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  SmiHandler = AllocateZeroPool (sizeof (SMI_HANDLER));
  if (SmiHandler == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  SmiHandler->Signature = SMI_HANDLER_SIGNATURE;
  SmiHandler->Handler = Handler;
  SmiHandler->CallerAddr = (UINTN)RETURN_ADDRESS (0);
  if (HandlerType == NULL) {
    //
    // This is root SMI handler
    //
    DEBUG((DEBUG_INIT, "Hello!TRZ! @r0 \n"));
    SmiEntry = &mRootSmiEntry;
  } else {
    //
    // None root SMI handler
    //
    DEBUG((DEBUG_INIT, "Hello!TRZ! @r1 \n"));
    SmiEntry = SmmCoreFindSmiEntry ((EFI_GUID *) HandlerType, TRUE);
    if (SmiEntry == NULL) {
	return EFI_OUT_OF_RESOURCES;
    }
  }
  List = &SmiEntry->SmiHandlers;

  SmiHandler->SmiEntry = SmiEntry;
  InsertTailList (List, &SmiHandler->Link);

  *DispatchHandle = (EFI_HANDLE) SmiHandler;
  
  return EFI_SUCCESS;
}

/**
  Unregister a handler in SMM.

  @param  DispatchHandle  The handle that was specified when the handler was registered.

  @retval EFI_SUCCESS           Handler function was successfully unregistered.
  @retval EFI_INVALID_PARAMETER DispatchHandle does not refer to a valid handle.

**/
EFI_STATUS
EFIAPI
SmiHandlerUnRegister (
  IN EFI_HANDLE  DispatchHandle
  )
{
  SMI_HANDLER  *SmiHandler;
  SMI_ENTRY    *SmiEntry;
  LIST_ENTRY   *EntryLink;
  LIST_ENTRY   *HandlerLink;

  if (DispatchHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Look for it in root SMI handlers
  //
  SmiHandler = NULL;
  for ( HandlerLink = GetFirstNode (&mRootSmiEntry.SmiHandlers)
      ; !IsNull (&mRootSmiEntry.SmiHandlers, HandlerLink) && ((EFI_HANDLE) SmiHandler != DispatchHandle)
      ; HandlerLink = GetNextNode (&mRootSmiEntry.SmiHandlers, HandlerLink)
      ) {
    SmiHandler = CR (HandlerLink, SMI_HANDLER, Link, SMI_HANDLER_SIGNATURE);
  }

  //
  // Look for it in non-root SMI handlers
  //
  for ( EntryLink = GetFirstNode (&mSmiEntryList)
      ; !IsNull (&mSmiEntryList, EntryLink) && ((EFI_HANDLE) SmiHandler != DispatchHandle)
      ; EntryLink = GetNextNode (&mSmiEntryList, EntryLink)
      ) {
    SmiEntry = CR (EntryLink, SMI_ENTRY, AllEntries, SMI_ENTRY_SIGNATURE);
    for ( HandlerLink = GetFirstNode (&SmiEntry->SmiHandlers)
        ; !IsNull (&SmiEntry->SmiHandlers, HandlerLink) && ((EFI_HANDLE) SmiHandler != DispatchHandle)
        ; HandlerLink = GetNextNode (&SmiEntry->SmiHandlers, HandlerLink)
        ) {
      SmiHandler = CR (HandlerLink, SMI_HANDLER, Link, SMI_HANDLER_SIGNATURE);
    }
  }

  if ((EFI_HANDLE) SmiHandler != DispatchHandle) {
    return EFI_INVALID_PARAMETER;
  }

  SmiEntry = SmiHandler->SmiEntry;

  RemoveEntryList (&SmiHandler->Link);
  FreePool (SmiHandler);

  if (SmiEntry == NULL) {
    //
    // This is root SMI handler
    //
    return EFI_SUCCESS;
  }

  if (IsListEmpty (&SmiEntry->SmiHandlers)) {
    //
    // No handler registered for this interrupt now, remove the SMI_ENTRY
    //
    RemoveEntryList (&SmiEntry->AllEntries);

    FreePool (SmiEntry);
  }

  return EFI_SUCCESS;
}
