/*******************************************************************************
* File Name: cyfitter_cfg.h
* 
* PSoC Creator  4.4
*
* Description:
* This file provides basic startup and mux configuration settings
* This file is automatically generated by PSoC Creator.
*
********************************************************************************
* Copyright (c) 2007-2020 Cypress Semiconductor.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
********************************************************************************/

#ifndef CYFITTER_CFG_H
#define CYFITTER_CFG_H

#include "cytypes.h"

extern void cyfitter_cfg(void);

/* Analog Set/Unset methods */
extern void SetAnalogRoutingPumps(uint8 enabled);
extern void AMux_Set(uint8 channel);
extern void AMux_Unset(uint8 channel);
extern void ADC_AMux_Set(uint8 channel);
extern void ADC_AMux_Unset(uint8 channel);
/* ADC_AMux__addrTable is an implementation detail of the AMux.
   Code that depends on it may be incompatible with other versions
   of PSoC Creator. */
extern uint8 CYXDATA * const CYCODE ADC_AMux__addrTable[2];
/* ADC_AMux__maskTable is an implementation detail of the AMux.
   Code that depends on it may be incompatible with other versions
   of PSoC Creator. */
extern const uint8 CYCODE ADC_AMux__maskTable[2];


#endif /* CYFITTER_CFG_H */

/*[]*/
