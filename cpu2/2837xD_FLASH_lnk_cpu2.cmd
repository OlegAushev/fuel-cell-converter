
MEMORY
{
PAGE 0 :
   /* BEGIN is used for the "boot to SARAM" bootloader mode   */

   BEGIN           	: origin = 0x080000, length = 0x000002
   RAMM0           	: origin = 0x0000A2, length = 0x00035E
   RAMD0           	: origin = 0x00B000, length = 0x000800
   RAMLS0          	: origin = 0x008000, length = 0x000800
   RAMLS1          	: origin = 0x008800, length = 0x000800
   RAMLS2      		: origin = 0x009000, length = 0x000800
   RAMLS3      		: origin = 0x009800, length = 0x000800
   RAMLS4      		: origin = 0x00A000, length = 0x000800
   RAMGS14          : origin = 0x01A000, length = 0x001000     /* Only Available on F28379D, F28377D, F28375D devices. Remove line on other devices. */
   RAMGS15          : origin = 0x01B000, length = 0x000FF8     /* Only Available on F28379D, F28377D, F28375D devices. Remove line on other devices. */

//   RAMGS15_RSVD     : origin = 0x01BFF8, length = 0x000008    /* Reserve and do not use for code as per the errata advisory "Memory: Prefetching Beyond Valid Memory" */

   RESET       		: origin = 0x3FFFC0, length = 0x000002

   /* Flash sectors */
   FLASHA           : origin = 0x080002, length = 0x001FFE	/* on-chip Flash */
   FLASHB           : origin = 0x082000, length = 0x002000	/* on-chip Flash */
   FLASHC           : origin = 0x084000, length = 0x002000	/* on-chip Flash */
   FLASHD           : origin = 0x086000, length = 0x002000	/* on-chip Flash */
   FLASHE           : origin = 0x088000, length = 0x008000	/* on-chip Flash */
   FLASHF           : origin = 0x090000, length = 0x008000	/* on-chip Flash */
   FLASHG           : origin = 0x098000, length = 0x008000	/* on-chip Flash */
   FLASHH           : origin = 0x0A0000, length = 0x008000	/* on-chip Flash */
   FLASHI           : origin = 0x0A8000, length = 0x008000	/* on-chip Flash */
   FLASHJ           : origin = 0x0B0000, length = 0x008000	/* on-chip Flash */
   FLASHK           : origin = 0x0B8000, length = 0x002000	/* on-chip Flash */
   FLASHL           : origin = 0x0BA000, length = 0x002000	/* on-chip Flash */
   FLASHM           : origin = 0x0BC000, length = 0x002000	/* on-chip Flash */
   FLASHN           : origin = 0x0BE000, length = 0x001FF0	/* on-chip Flash */

//   FLASHN_RSVD     : origin = 0x0BFFF0, length = 0x000010    /* Reserve and do not use for code as per the errata advisory "Memory: Prefetching Beyond Valid Memory" */

PAGE 1 :

   BOOT_RSVD       : origin = 0x000002, length = 0x0000A0     /* Part of M0, BOOT rom will use this for stack */
   RAMM1           : origin = 0x000400, length = 0x0003F8     /* on-chip RAM block M1 */
//   RAMM1_RSVD      : origin = 0x0007F8, length = 0x000008     /* Reserve and do not use for code as per the errata advisory "Memory: Prefetching Beyond Valid Memory" */
   RAMD1           : origin = 0x00B800, length = 0x000800

   RAMLS5          : origin = 0x00A800, length = 0x000800

   RAMGS0          : origin = 0x00C000, length = 0x001000
   RAMGS1          : origin = 0x00D000, length = 0x001000
   // USER BEGIN
   RAMGS7      : origin = 0x013000, length = 0x001000
   RAMGS8      : origin = 0x014000, length = 0x001000
   RAMGS9      : origin = 0x015000, length = 0x001000
   RAMGS10     : origin = 0x016000, length = 0x001000
   // USER END
   CPU2TOCPU1RAM   : origin = 0x03F800, length = 0x000400
   CPU1TOCPU2RAM   : origin = 0x03FC00, length = 0x000400
}

SECTIONS
{
   /* Allocate program areas: */
   .cinit              : > FLASHJ      PAGE = 0, ALIGN(8)
   .text               : >> FLASHB | FLASHC | FLASHD | FLASHE | FLASHI      PAGE = 0, ALIGN(8)
   codestart           : > BEGIN       PAGE = 0, ALIGN(8)

   /* Allocate uninitalized data sections: */
   .stack              : > RAMM1        PAGE = 1

   /* Initalized sections go in Flash */
   .switch             : > FLASHB      PAGE = 0, ALIGN(8)
   .reset           : > RESET,     PAGE = 0, TYPE = DSECT /* not used, */
   
#if defined(__TI_EABI__)
   .init_array         : > FLASHB,       PAGE = 0,       ALIGN(8)
   .bss                : > RAMLS5 | RAMGS8,       PAGE = 1
   .bss:output         : > RAMLS3,       PAGE = 0
   .bss:cio            : > RAMLS5,       PAGE = 1
   .data               : > RAMLS5,       PAGE = 1
   .sysmem             : > RAMLS5,       PAGE = 1
   /* Initalized sections go in Flash */
   .const              : >> FLASHF | FLASHG | FLASHH,       PAGE = 0,       ALIGN(8)
#else
   .pinit              : > FLASHB,       PAGE = 0,       ALIGN(8)
   .ebss               : >> RAMLS5 | RAMGS8,    PAGE = 1
   .esysmem            : > RAMLS5,       PAGE = 1
   .cio                : > RAMLS5,       PAGE = 1
   /* Initalized sections go in Flash */
   .econst             : >> FLASHF       PAGE = 0, ALIGN(8)
#endif

   SHARERAMGS0		: > RAMGS0,		PAGE = 1
   SHARERAMGS1		: > RAMGS1,		PAGE = 1
   // USER BEGIN
   GROUP : > RAMGS7
   {
      SYSTEMCONFIG
   }

   GROUP : > RAMGS9
   {
      SHARED_CONVERTER
      SHARED_CANA_TPDO_DATA
      SHARED_MCO_CAN1_TSDO_DATA
      SHARED_MCO_CAN2_TSDO_DATA
      SHARED_SYSLOG_DATA_CPU1
      SHARED_SYSLOG_MESSAGES
   }

   GROUP : > RAMGS10
   {
      SHARED_MCO_RPDO_DATA
      SHARED_MCO_CAN1_RSDO_DATA
      SHARED_MCO_CAN2_RSDO_DATA
      SHARED_SYSLOG_DATA_CPU2
      SHARED_SYSLOG_MESSAGE_CPU2
   }
   // USER END

#ifdef __TI_COMPILER_VERSION__
    #if __TI_COMPILER_VERSION__ >= 15009000
        #if defined(__TI_EABI__)
            .TI.ramfunc : {} LOAD = FLASHD,
                                 RUN = RAMLS0,
                                 LOAD_START(RamfuncsLoadStart),
                                 LOAD_SIZE(RamfuncsLoadSize),
                                 LOAD_END(RamfuncsLoadEnd),
                                 RUN_START(RamfuncsRunStart),
                                 RUN_SIZE(RamfuncsRunSize),
                                 RUN_END(RamfuncsRunEnd),
                                 PAGE = 0, ALIGN(8)
        #else
            .TI.ramfunc : {} LOAD = FLASHD,
                             RUN = RAMLS0,
                             LOAD_START(_RamfuncsLoadStart),
                             LOAD_SIZE(_RamfuncsLoadSize),
                             LOAD_END(_RamfuncsLoadEnd),
                             RUN_START(_RamfuncsRunStart),
                             RUN_SIZE(_RamfuncsRunSize),
                             RUN_END(_RamfuncsRunEnd),
                             PAGE = 0, ALIGN(8)
        #endif
    #else
   ramfuncs            : LOAD = FLASHD,
                         RUN = RAMLS0,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         PAGE = 0, ALIGN(8)
    #endif
#endif

   /* The following section definitions are required when using the IPC API Drivers */
    GROUP : > CPU2TOCPU1RAM, PAGE = 1
    {
        PUTBUFFER
        PUTWRITEIDX
        GETREADIDX
    }

    GROUP : > CPU1TOCPU2RAM, PAGE = 1
    {
        GETBUFFER :    TYPE = DSECT
        GETWRITEIDX :  TYPE = DSECT
        PUTREADIDX :   TYPE = DSECT
    }
}

/*
//===========================================================================
// End of file.
//===========================================================================
*/
