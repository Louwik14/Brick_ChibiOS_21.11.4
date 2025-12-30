# STM32H743ZI clock plan (audio-centric)

## Goals
- Audio is the musical time master (SAI2 TDM @ 48 kHz, MCLK 256×Fs = 12.288 MHz).
- USB FS clock is independent of SYSCLK and audio.
- ADC and SDMMC clocks are independent of audio.
- PLL1 dedicated to core/system clocks only.

## Clock tree summary

```
HSE 25 MHz
├─ PLL1 (SYSCLK domain)
│  ├─ M=5, N=160 → VCO=800 MHz
│  └─ P=2 → SYSCLK = 400 MHz
│     Q=10 → 80 MHz (SDMMC1 kernel)
│
├─ PLL2 (audio domain)
│  ├─ M=5, N=98, FRACN=2494 → VCO=491.52 MHz
│  └─ P=10 → 49.152 MHz → SAI2 kernel
│      SAI MCKDIV=3 ⇒ MCLK = 49.152 / (3+1) = 12.288 MHz
│
└─ PLL3 (USB/ADC/SPI domain)
   ├─ M=25, N=288 → VCO=288 MHz (integer-only)
   ├─ Q=6 → 48 MHz  → USB FS kernel (independent)
   ├─ R=8 → 36 MHz  → ADC kernel
   └─ P=4 → 72 MHz  → SPI123 kernel
```

## Macro ↔ function mapping

- **PLL1 (SYSCLK)**
  - `STM32_PLL1_*` in `cfg/mcuconf.h`
  - `STM32_SW = STM32_SW_PLL1_P_CK` selects SYSCLK source.

- **PLL2 (audio)**
  - `STM32_PLL2_*` in `cfg/mcuconf.h`
  - `STM32_SAI23SEL = STM32_SAI23SEL_PLL2_P_CK`

- **PLL3 (USB/ADC/SPI)**
  - `STM32_PLL3_*` in `cfg/mcuconf.h`
  - `STM32_USBSEL = STM32_USBSEL_PLL3_Q_CK`
  - `STM32_ADCSEL = STM32_ADCSEL_PLL3_R_CK`
  - `STM32_SPI123SEL = STM32_SPI123SEL_PLL3_P_CK`

- **SDMMC1**
  - `STM32_SDMMCSEL = STM32_SDMMCSEL_PLL1_Q_CK`

## Equations and constraints

- **SYSCLK**
- VCO1 = 25 MHz / 5 × 160 = 800 MHz
- SYSCLK = 800 / 2 = 400 MHz

- **Audio (SAI2)**
  - VCO2 = 25 MHz / 5 × (98 + 2494/8192) = 491.52 MHz
  - SAI kernel = 491.52 / 10 = 49.152 MHz
  - MCLK = 49.152 / 4 = 12.288 MHz (MCKDIV=3)

- **USB FS** (integer only)
  - VCO3 = 25 MHz / 25 × 288 = 288 MHz
  - USB = 288 / 6 = 48 MHz (PLL3 Q)

- **ADC**
  - ADC kernel = 288 / 8 = 36 MHz (PLL3 R)
  - ADC clock mode uses ADCCK (no HCLK coupling).
  - ADC clock remains independent from audio PLL.

- **SPI123**
  - SPI123 kernel = 288 / 4 = 72 MHz (PLL3 P)

## Guarantees

- Audio PLL (PLL2) is dedicated and not shared with USB or SYSCLK.
- USB FS clock is derived from PLL3_Q only (integer divisor, independent).
- ADC and SPI are on PLL3 (no audio coupling).
- SDMMC1 uses PLL1_Q = 80 MHz (valid kernel without audio coupling).
- All PLL outputs are within ChibiOS STM32H7 clock checks.

## Notes

- If SDMMC requires a different kernel frequency, adjust `STM32_PLL2_DIVR_VALUE`
  while preserving USB (48 MHz) and ADC (120 MHz) integer outputs.
- The SAI MCKDIV setting must be configured in the SAI driver to yield
  MCLK=12.288 MHz from the 49.152 MHz kernel.
