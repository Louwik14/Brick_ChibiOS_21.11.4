```md
# Project State — STM32H743 / ChibiOS 21.11.4

## 1. Contexte général

- MCU : **STM32H743** (rev XY)
- Package réel : **LQFP176**
- RTOS : **ChibiOS 21.11.4**
- Projet basé sur le demo :
```

ChibiOS_21.11.4/demos/STM32/RT-STM32H743ZI_REV_XY-NUCLEO144

```
- Board utilisée :
```

os/hal/boards/STM32H743_LQFP176_CUSTOM

````
- Toolchain : ARM GCC (OK)
- Flash/debug : **ST-Link + OpenOCD**
- Debug runtime : **UART1 (USART1 / SD1)**

Objectif du projet :  
> Plateforme audio temps réel (SAI2 maître), avec SDRAM externe (FMC), USB FS, SPI multiples, SDMMC, ADC, et forte exigence de stabilité clock.

---

## 2. État actuel : VALIDÉ

### 2.1 Démarrage CPU & Clock

- Le **CPU démarre correctement**
- La séquence `halInit() → chSysInit() → main()` est atteinte
- La **clock système est fonctionnelle et cohérente**
- Validation runtime via UART avec affichage :
- `SYS_CK`
- `HCLK`
- `PCLK1..4`

👉 Une clock de debug **HSI + PLL1** peut-etre utilisée pour validation.  
👉 Le retour vers une clock HSE/quartz est possible.

---

### 2.2 UART Debug (USART1 / SD1)

- UART1 **100 % fonctionnel**
- Baudrate explicitement configuré (ex: 115200)
- Sortie UART validée dans **ChibiStudio Terminal**
- Permet :
- logs de boot
- validation clock
- debug runtime

UART = **outil principal de validation logicielle** à ce stade.

---

### 2.3 LED Debug (PH7)

- LED câblée sur **PH7 (active-low)**
- GPIO configuré via `board.h` + redéfini dans `main()`
- LED clignote correctement
- Sert de **watchdog visuel minimal**

---

### 2.4 Board Custom

- Le **board custom est bien celui compilé**
- `board.mk` du dossier `STM32H743_LQFP176_CUSTOM` est inclus dans le Makefile
- Aucun mélange avec l’ancienne board Nucleo

---

## 3. FMC / SDRAM — ÉTAT ACTUEL

- **La SDRAM n’est plus initialisée au niveau board**
- Raison : l’initialisation SDRAM dans `boardInit()` bloquait le CPU avant `main()`
- Les **pins FMC/SDRAM restent configurées en AF12** dans `board.h` (pinmux conservé)
- **Aucun accès registre FMC** n’est effectué côté board
- Le système reste stable

👉 **Conclusion actuelle** :
La SDRAM **ne doit pas être initialisée dans `boardInit()`**.

Décision future :

* déplacer l’init SDRAM dans :

  * un module dédié (`sdram.c`)
  * appelé explicitement **après** boot + debug validé

---

## 4. Périphériques (état résumé)

| Périphérique | État                       |
| ------------ | -------------------------- |
| UART1        | ✅ Validé                   |
| GPIO / LED   | ✅ Validé                   |
| Clock tree   | ✅ Validé (debug mode)      |
| FMC / SDRAM  | ❌ Isolé (cause identifiée) |
| SAI2         | ⏸️ Non activé              |
| USB FS       | ⏸️ Non activé              |
| SPI          | ⏸️ Non activé              |
| SDMMC        | ⏸️ Non activé              |
| ADC          | ⏸️ Non activé              |

---

## 5. Main de référence (actuel)

Le projet dispose d’un **main minimal de validation** qui :

* force la config UART
* affiche les clocks
* fait clignoter la LED
* prouve que le système est sain

Ce main est la **base de confiance** du projet.

---

## 6. Prochaines étapes prévues (ordre recommandé)

1. **Refactor FMC/SDRAM**

   * sortir l’init SDRAM du board
   * créer un module SDRAM explicite
   * init après boot

2. Validation SDRAM par test mémoire simple

3. Réintégration progressive :

   * SAI2 (audio maître)
   * DMA
   * SDMMC
   * USB FS

4. Retour éventuel à une clock HSE/quartz définitive

---

## 7. Règles de projet (importantes)

* ❌ Pas de bidouille HAL
* ❌ Pas de code bloquant avant `main()`
* ✅ Tout périphérique critique doit être initialisé **explicitement**

---

## 8. État global

> 🟢 **Le socle CPU + clock + debug est sain et validé.**
> 🔧 **Le problème FMC est identifié et maîtrisé.**
> 🚀 Le projet peut continuer de manière structurée.

```
