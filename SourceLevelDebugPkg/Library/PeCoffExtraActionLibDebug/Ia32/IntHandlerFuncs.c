/** @file
  Ia32 arch functions to access IDT vector.

  Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PeCoffExtraActionLib.h>

/**
  Read IDT entry to check if IDT entries are setup by Debug Agent.

  @param[in]  IdtDescriptor      Pointer to IDT Descriptor.

  @retval  TRUE     IDT entries were setup by Debug Agent.
  @retval  FALSE    IDT entries were not setuo by Debug Agent.

**/
BOOLEAN 
CheckDebugAgentHandler (
  IN  IA32_DESCRIPTOR            *IdtDescriptor
  )
{
  IA32_IDT_GATE_DESCRIPTOR   *IdtEntry;
  UINTN                      InterruptHandler;

  IdtEntry = (IA32_IDT_GATE_DESCRIPTOR *) IdtDescriptor->Base;
  if (IdtEntry == NULL) {
    return FALSE;
  }

  InterruptHandler = IdtEntry[0].Bits.OffsetLow + (IdtEntry[0].Bits.OffsetHigh << 16);
  if (InterruptHandler >= 4 &&  *(UINT32 *)(InterruptHandler - 4) == AGENT_HANDLER_SIGNATURE) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Save IDT entry for INT1 and update it. 

  @param[in]  IdtDescriptor      Pointer to IDT Descriptor.
  @param[out] SavedIdtEntry      Original IDT entry returned.

**/
VOID
SaveAndUpdateIdtEntry1 (
  IN  IA32_DESCRIPTOR            *IdtDescriptor,
  OUT IA32_IDT_GATE_DESCRIPTOR   *SavedIdtEntry
  )
{
  IA32_IDT_GATE_DESCRIPTOR   *IdtEntry;
  UINT16                     CodeSegment;
  UINTN                      InterruptHandler;
  
  IdtEntry = (IA32_IDT_GATE_DESCRIPTOR *) IdtDescriptor->Base;
  CopyMem (SavedIdtEntry, &IdtEntry[1], sizeof (IA32_IDT_GATE_DESCRIPTOR));

    //
  // Use current CS as the segment selector of interrupt gate in IDT
  //
  CodeSegment = AsmReadCs ();

  InterruptHandler = (UINTN) &AsmInterruptHandle;
  IdtEntry[1].Bits.OffsetLow  = (UINT16)(UINTN)InterruptHandler;
  IdtEntry[1].Bits.OffsetHigh = (UINT16)((UINTN)InterruptHandler >> 16);
  IdtEntry[1].Bits.Selector   = CodeSegment;
  IdtEntry[1].Bits.GateType   = IA32_IDT_GATE_TYPE_INTERRUPT_32;
}

/**
  Restore IDT entry for INT1. 

  @param[in]  IdtDescriptor      Pointer to IDT Descriptor.
  @param[in]  RestoredIdtEntry   IDT entry to be restored.

**/
VOID
RestoreIdtEntry1 (
  IN  IA32_DESCRIPTOR            *IdtDescriptor,
  IN  IA32_IDT_GATE_DESCRIPTOR   *RestoredIdtEntry
  )
{
  IA32_IDT_GATE_DESCRIPTOR   *IdtEntry;
  
  IdtEntry = (IA32_IDT_GATE_DESCRIPTOR *) IdtDescriptor->Base;
  CopyMem (&IdtEntry[1], RestoredIdtEntry, sizeof (IA32_IDT_GATE_DESCRIPTOR));
}
