# 📄 `agent.md`

```markdown
# Project State — STM32H743 / ChibiOS 21.11.4
## Audio workstation modulaire type *Octatrack-like* avec cartouches Ksoloti

---

## 1. Contexte général

* MCU : **STM32H743** (rev XY)
* Package réel : **LQFP176**
* RTOS : **ChibiOS 21.11.4**
* Toolchain : **ARM GCC**
* Flash / Debug : **ST-Link + OpenOCD**
* IDE : **ChibiStudio**
* Debug runtime principal : **UART1 (USART1 / SD1)**

### Projet basé sur

```

ChibiOS_21.11.4/demos/STM32/RT-STM32H743ZI_REV_XY-NUCLEO144

```

### Board custom

```

os/hal/boards/STM32H743_LQFP176_CUSTOM

```

---

## 2. Vision produit (FIGÉE)

Le projet vise à développer une **machine audio temps réel de type Octatrack**, pensée pour :

* Performance live
* Robustesse absolue
* Latence faible et déterministe
* Architecture modulaire par cartouches

La machine fonctionne comme une **console audio + séquenceur autonome**, sans dépendance à un ordinateur.

> Objectif produit :
> *« Un musicien doit pouvoir monter sur scène avec cette machine en toute confiance. »*

---

## 3. Architecture audio globale (FIGÉE)

### Paramètres audio

* Fréquence : **48 kHz**
* Taille de bloc : **64 samples**
* Audio **hard real-time**
* Aucun traitement audio critique dans des threads RTOS

### Pilotage

* Audio déclenché par **DMA SAI (IRQ half / full)**
* Traitement audio dans une fonction dédiée à cadence fixe
* UI, MIDI, LEDs = **soft real-time**

---

## 4. Pistes audio

### Audio interne (STM32H7)

* **8 pistes stéréo garanties**
* FX globaux en sends
* Mixage, routing, looper local

### Audio externe (cartouches Ksoloti)

* DSP lourd
* Synthèse
* FX complexes

Le STM32H7 agit comme :
* séquenceur
* routeur
* mixeur léger

### Nombre de cartouches actives

* **1 à 2 cartouches audio actives simultanément**
* Plus possible techniquement, mais non garanti live

---

## 5. Bus cartouches — SPI-link (FIGÉ)

> ❗ **Le bus SPI-link est conservé comme bus audio et contrôle.**

### Contraintes assumées

* Pas de parallélisme SPI audio
* Latence cumulative
* SPI utilisé comme **bus audio synchrone**

### Règles SPI-link

* Déclenché à chaque bloc audio
* DMA obligatoire
* Double buffer RX/TX
* Aucune logique UI dans le chemin audio
* Priorité élevée

---

## 6. Mémoire

### SDRAM externe (FMC)

* Référence : **W9825G6KH-6I** (32 MB, x16)
* Driver : ChibiOS-Contrib HAL SDRAM
* État : ✅ fonctionnelle

#### Règles NON NÉGOCIABLES

* Accès **32-bit uniquement**
* ❌ Accès 8/16-bit interdits
* Demi-mots inversés (x16)
* Wrapper `swap16` obligatoire

#### Usage autorisé

* buffers audio
* delay
* looper
* granular

#### Usage interdit

* heap
* UI
* structures système

Base : `0xC0000000`

---

## 7. SRAM interne

* UI
* contrôle
* états
* drivers

---

## 8. Interface utilisateur

### Entrées

* Switchs tactiles
* 16 pads Hall Effect (ADC + MUX)
* Potentiomètres multiplexés
* 4 encodeurs rotatifs (software)

### Sorties

* OLED SPI
* 25 LEDs WS2812 (DMA + timer)

Jamais dans le chemin audio.

---

## 9. Audio I/O interne

* SAI2A : ADC (4 entrées stéréo)
* SAI2B : DAC
* 2 sorties stéréo indépendantes

---

## 10. Connectivité

* MIDI DIN (UART5)
* USB device MIDI
* USB host MIDI

---

## 11. Boot / Debug (VALIDÉ)

* Séquence :
```

Reset → halInit() → chSysInit() → main()

```

* Clock : HSI + PLL1 (debug-safe)
* UART1 = outil principal de validation
* LED debug sur PH7

---

## 12. FMC / SDRAM — règles STRICTES

* Pins définies dans `board.h`
* ❌ Aucune init SDRAM dans `boardInit()`
* Init uniquement via `sdram_ext.c`
* Validation minimale :
  * write `0x11223344`
  * read `0x33441122`

---

## 13. Philosophie projet (NON NÉGOCIABLE)

* ❌ Pas de bidouille HAL
* ❌ Pas d’init lourde avant `main()`
* ❌ Pas de heap dynamique
* ❌ Pas de logique audio dans l’UI
* ✅ Tout périphérique critique est initialisé explicitement
* ✅ Toute init est observable via UART
* ✅ Fiabilité live > performance brute
* ✅ Ksoloti = pilier du concept

---

## 14. État actuel

> 🟢 Boot / clock / debug sains  
> 🟢 SDRAM validée  
> 🧱 Architecture verrouillée  
> 🧠 Compromis techniques assumés  

---

# 15. MEMORY / CACHE / MPU CONTRACT  
## (Codex / ChatGPT — SOURCE DE VÉRITÉ)

Cette section définit **le contrat absolu** que toute aide automatisée doit respecter.

---

## 15.1 Invariants globaux (TOUJOURS vrais)

* MCU : STM32H743 (single-core M7)
* RTOS : ChibiOS 21.11.4
* Audio temps réel déterministe
* Aucune allocation dynamique
* Aucune décision mémoire implicite
* Aucune optimisation sans demande explicite

---

## 15.2 Modèle mémoire STRUCTUREL (fixe)

### DTCM (0x20000000)
* Code audio critique
* États audio
* ❌ DMA interdit

### SRAM AXI / D2
* Tous buffers DMA (SAI, SPI, SDMMC, LEDs)
* Alignement ≥ 32 bytes
* Pas de logique audio critique

### SDRAM FMC
* Audio uniquement
* Accès 32-bit
* Wrapper obligatoire
* ❌ heap / UI / système

---

## 15.3 Phases cache / MPU (STRICTES)

### Phase 1 — Bring-up
* I-Cache : ON
* D-Cache : OFF
* MPU : minimal
* SDRAM : non-cacheable
* Objectif : fonctionnement simple

### Phase 2 — Validation
* SDMMC
* SDRAM
* DMA simples
* Toujours sans cache data

### Phase 3 — Audio
* SAI + DMA
* SPI audio
* Buffers strictement contrôlés

### Phase 4 — Production
* D-Cache : ON
* MPU affiné
* Maintenance cache explicite

❗ Aucune phase ne peut être sautée.

---

## 15.4 Règles DMA

* Buffer DMA :
  - déclaré explicitement
  - aligné 32 bytes
  - non-cacheable OU maintenance explicite
* ❌ DMA sur DTCM interdit

---

## 15.5 Attentes envers Codex / ChatGPT

Lors d’une assistance :

1. Identifier la phase actuelle
2. Respecter STRICTEMENT cette phase
3. Décrire clairement :
   - mémoire utilisée
   - mémoire interdite
   - cache / MPU
4. Générer du code uniquement sur demande
5. Ne jamais modifier ce contrat

---

## 15.6 Objectif final

> Garantir un firmware STM32H743 :
> - déterministe
> - live-safe
> - compréhensible dans le temps
> - robuste malgré génération automatique
```

