/*******************************************************************************
* File Name: Valve2_POS_reading.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_Valve2_POS_reading_H) /* Pins Valve2_POS_reading_H */
#define CY_PINS_Valve2_POS_reading_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "Valve2_POS_reading_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 Valve2_POS_reading__PORT == 15 && ((Valve2_POS_reading__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    Valve2_POS_reading_Write(uint8 value);
void    Valve2_POS_reading_SetDriveMode(uint8 mode);
uint8   Valve2_POS_reading_ReadDataReg(void);
uint8   Valve2_POS_reading_Read(void);
void    Valve2_POS_reading_SetInterruptMode(uint16 position, uint16 mode);
uint8   Valve2_POS_reading_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the Valve2_POS_reading_SetDriveMode() function.
     *  @{
     */
        #define Valve2_POS_reading_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define Valve2_POS_reading_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define Valve2_POS_reading_DM_RES_UP          PIN_DM_RES_UP
        #define Valve2_POS_reading_DM_RES_DWN         PIN_DM_RES_DWN
        #define Valve2_POS_reading_DM_OD_LO           PIN_DM_OD_LO
        #define Valve2_POS_reading_DM_OD_HI           PIN_DM_OD_HI
        #define Valve2_POS_reading_DM_STRONG          PIN_DM_STRONG
        #define Valve2_POS_reading_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define Valve2_POS_reading_MASK               Valve2_POS_reading__MASK
#define Valve2_POS_reading_SHIFT              Valve2_POS_reading__SHIFT
#define Valve2_POS_reading_WIDTH              1u

/* Interrupt constants */
#if defined(Valve2_POS_reading__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in Valve2_POS_reading_SetInterruptMode() function.
     *  @{
     */
        #define Valve2_POS_reading_INTR_NONE      (uint16)(0x0000u)
        #define Valve2_POS_reading_INTR_RISING    (uint16)(0x0001u)
        #define Valve2_POS_reading_INTR_FALLING   (uint16)(0x0002u)
        #define Valve2_POS_reading_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define Valve2_POS_reading_INTR_MASK      (0x01u) 
#endif /* (Valve2_POS_reading__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define Valve2_POS_reading_PS                     (* (reg8 *) Valve2_POS_reading__PS)
/* Data Register */
#define Valve2_POS_reading_DR                     (* (reg8 *) Valve2_POS_reading__DR)
/* Port Number */
#define Valve2_POS_reading_PRT_NUM                (* (reg8 *) Valve2_POS_reading__PRT) 
/* Connect to Analog Globals */                                                  
#define Valve2_POS_reading_AG                     (* (reg8 *) Valve2_POS_reading__AG)                       
/* Analog MUX bux enable */
#define Valve2_POS_reading_AMUX                   (* (reg8 *) Valve2_POS_reading__AMUX) 
/* Bidirectional Enable */                                                        
#define Valve2_POS_reading_BIE                    (* (reg8 *) Valve2_POS_reading__BIE)
/* Bit-mask for Aliased Register Access */
#define Valve2_POS_reading_BIT_MASK               (* (reg8 *) Valve2_POS_reading__BIT_MASK)
/* Bypass Enable */
#define Valve2_POS_reading_BYP                    (* (reg8 *) Valve2_POS_reading__BYP)
/* Port wide control signals */                                                   
#define Valve2_POS_reading_CTL                    (* (reg8 *) Valve2_POS_reading__CTL)
/* Drive Modes */
#define Valve2_POS_reading_DM0                    (* (reg8 *) Valve2_POS_reading__DM0) 
#define Valve2_POS_reading_DM1                    (* (reg8 *) Valve2_POS_reading__DM1)
#define Valve2_POS_reading_DM2                    (* (reg8 *) Valve2_POS_reading__DM2) 
/* Input Buffer Disable Override */
#define Valve2_POS_reading_INP_DIS                (* (reg8 *) Valve2_POS_reading__INP_DIS)
/* LCD Common or Segment Drive */
#define Valve2_POS_reading_LCD_COM_SEG            (* (reg8 *) Valve2_POS_reading__LCD_COM_SEG)
/* Enable Segment LCD */
#define Valve2_POS_reading_LCD_EN                 (* (reg8 *) Valve2_POS_reading__LCD_EN)
/* Slew Rate Control */
#define Valve2_POS_reading_SLW                    (* (reg8 *) Valve2_POS_reading__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define Valve2_POS_reading_PRTDSI__CAPS_SEL       (* (reg8 *) Valve2_POS_reading__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define Valve2_POS_reading_PRTDSI__DBL_SYNC_IN    (* (reg8 *) Valve2_POS_reading__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define Valve2_POS_reading_PRTDSI__OE_SEL0        (* (reg8 *) Valve2_POS_reading__PRTDSI__OE_SEL0) 
#define Valve2_POS_reading_PRTDSI__OE_SEL1        (* (reg8 *) Valve2_POS_reading__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define Valve2_POS_reading_PRTDSI__OUT_SEL0       (* (reg8 *) Valve2_POS_reading__PRTDSI__OUT_SEL0) 
#define Valve2_POS_reading_PRTDSI__OUT_SEL1       (* (reg8 *) Valve2_POS_reading__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define Valve2_POS_reading_PRTDSI__SYNC_OUT       (* (reg8 *) Valve2_POS_reading__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(Valve2_POS_reading__SIO_CFG)
    #define Valve2_POS_reading_SIO_HYST_EN        (* (reg8 *) Valve2_POS_reading__SIO_HYST_EN)
    #define Valve2_POS_reading_SIO_REG_HIFREQ     (* (reg8 *) Valve2_POS_reading__SIO_REG_HIFREQ)
    #define Valve2_POS_reading_SIO_CFG            (* (reg8 *) Valve2_POS_reading__SIO_CFG)
    #define Valve2_POS_reading_SIO_DIFF           (* (reg8 *) Valve2_POS_reading__SIO_DIFF)
#endif /* (Valve2_POS_reading__SIO_CFG) */

/* Interrupt Registers */
#if defined(Valve2_POS_reading__INTSTAT)
    #define Valve2_POS_reading_INTSTAT            (* (reg8 *) Valve2_POS_reading__INTSTAT)
    #define Valve2_POS_reading_SNAP               (* (reg8 *) Valve2_POS_reading__SNAP)
    
	#define Valve2_POS_reading_0_INTTYPE_REG 		(* (reg8 *) Valve2_POS_reading__0__INTTYPE)
#endif /* (Valve2_POS_reading__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_Valve2_POS_reading_H */


/* [] END OF FILE */
