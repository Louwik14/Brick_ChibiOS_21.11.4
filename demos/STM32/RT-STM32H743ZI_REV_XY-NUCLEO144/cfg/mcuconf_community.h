#ifndef MCUCONF_COMMUNITY_H
#define MCUCONF_COMMUNITY_H

/*
 * FSMC/FMC driver system settings.
 */
#define STM32_FSMC_USE_FSMC1                TRUE
#define STM32_FSMC_FSMC1_IRQ_PRIORITY       10
#define STM32_FSMC_DMA_CHN                  0x03010201

/*
 * FSMC SDRAM driver system settings.
 * SDRAM connected on Bank1 (NE0) => SDRAM1.
 */
#define STM32_SDRAM_USE_SDRAM1              TRUE
#define STM32_SDRAM_USE_SDRAM2              FALSE

/*
 * Disable unused FSMC sub-drivers.
 */
#define STM32_NAND_USE_NAND1                FALSE
#define STM32_NAND_USE_NAND2                FALSE
#define STM32_SRAM_USE_SRAM1                FALSE
#define STM32_SRAM_USE_SRAM2                FALSE
#define STM32_SRAM_USE_SRAM3                FALSE
#define STM32_SRAM_USE_SRAM4                FALSE

#endif /* MCUCONF_COMMUNITY_H */
