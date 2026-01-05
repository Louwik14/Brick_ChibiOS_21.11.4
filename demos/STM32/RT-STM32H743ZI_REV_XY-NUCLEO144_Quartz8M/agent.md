# Project State — STM32H743 / ChibiOS (Simple / Deterministic Baseline)
## Audio workstation modulaire temps réel (approche *live‑safe first*)

---

## 1. Contexte général (STABLE)

* MCU : **STM32H743** (single‑core Cortex‑M7)
* Package : **LQFP176**
* RTOS : **ChibiOS 21.11.x**
* Toolchain : **ARM GCC**
* Debug : **ST‑Link (SWD)** + OpenOCD
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


> ⚠️ Le firmware vise d’abord la **simplicité, la lisibilité et le déterminisme**.  
> Toute optimisation est secondaire.

---

## 2. Vision produit (FIGÉE)

Machine audio autonome orientée **performance live**, avec priorité absolue à :

* Stabilité
* Latence déterministe
* Absence de glitch
* Compréhension long terme du code

> *Le firmware doit rester compréhensible même sans son auteur.*

---

## 3. Architecture audio (RÈGLES DE BASE)

### Paramètres

* 48 kHz
* 64 samples par bloc
* Audio **hard real‑time**

### Modèle d’exécution

* Audio déclenché par **DMA SAI**
* IRQ DMA (half / full) = **métronome audio**
* Le traitement audio :
  - est court
  - non bloquant
  - ne dépend pas du RTOS

UI / MIDI / LEDs / USB = **soft real‑time**

---

## 4. Principe fondamental

> **Le son n’attend jamais.**

Aucune tâche, IRQ ou DMA non audio ne doit :
* bloquer l’audio
* retarder son IRQ
* accéder à ses buffers en même temps

---

## 5. DMA — règles SIMPLES

* DMA = transport uniquement
* Pas de logique métier dans les IRQ DMA
* Buffers DMA :
  - statiques
  - clairement identifiés
  - jamais partagés sans règle explicite

---

## 6. Mémoire — modèle SIMPLE (Phase actuelle)

### Cache / MPU (état actuel)

* **I‑Cache : ON**
* **D‑Cache : OFF**
* **MPU : NON utilisé**
* Objectif : **aucune ambiguïté CPU/DMA**

---

### DTCM (0x2000_0000)

* Code audio critique
* États audio
* ❌ DMA interdit

---

### SRAM AXI / D2

* Tous buffers DMA :
  - SAI
  - SPI audio
  - SD
  - LEDs
* Alignement ≥ 32 bytes
* Pas de calcul audio critique

---

### SDRAM externe (FMC)

* Audio uniquement :
  - delays
  - loopers
  - buffers longs
* Accès **32‑bit uniquement**
* ❌ Heap
* ❌ UI
* ❌ Structures système

---

## 7. SPI cartouches audio (INSPIRÉ AKSO)

* SPI utilisé comme **bus audio synchrone**
* Déclenché par le rythme audio
* DMA obligatoire
* Double buffering strict
* Aucune logique UI dans ce chemin

---

## 8. Interface utilisateur

* Entrées lentes (humain)
* Traitement différé
* Aucun impact sur l’audio

Jamais :
* dans une IRQ audio
* dans une IRQ DMA audio

---

## 9. Debug & observabilité

### Outils autorisés

* UART : états, validation fonctionnelle
* GPIO debug :
  - début IRQ audio
  - fin IRQ audio
  - DMA audio terminé
* Analyseur logique recommandé

### Interdit

* printf dans IRQ audio
* logs continus temps réel

---

## 10. Philosophie de développement (CONTRAT)

* ❌ Pas de heap dynamique
* ❌ Pas d’optimisation prématurée
* ❌ Pas de cache data tant que tout n’est pas maîtrisé
* ✅ Règles simples > performance brute
* ✅ Architecture figée avant features

---

## 11. Évolution future (NON ACTIVE)

Ces points sont **explicitement hors scope actuel** :

* D‑Cache ON
* MPU avancé
* Optimisation mémoire fine

Ils ne seront abordés que si :
1. Le système est stable
2. Le timing est mesuré
3. Le besoin est réel

---

## 12. Règle d’or finale

> **Si une décision ne peut pas être expliquée simplement,  
> elle est trop complexe pour ce stade du projet.**

---

## OLED Display Driver — Current Status (Validated)

### Hardware
- Display: SSD1309 / SSD1306 compatible OLED
- Resolution: 128x64
- Interface: SPI5 (STM32H743)
- Control lines: CS / DC / RESET (board-defined aliases)
- SPI mode: Polled (spiPolledExchange), no DMA

### Driver Architecture
- Full framebuffer in RAM
- Dedicated refresh thread (~30 Hz)
- Manual CS/DC control
- Runtime forcing of SPI5 pins to AF5 (STM32H7 robustness)

### Performance & Safety
- Audio-safe by design (no IRQ masking, no DMA, no mutex contention)
- Deterministic SPI usage
- Optimized refresh using:
  - Global dirty flag
  - Per-page dirty bitmap (8 pages)
- No full refresh when screen is static

### Drawing Capabilities
- Text rendering with font support
- Numbers and baseline-aligned text
- Pixel-level drawing API
- Rectangle primitives:
  - draw_rect (outline)
  - fill_rect
  - clear_rect
- Suitable for complex UI elements:
  - keyboards (stylized, chromatic)
  - pads / steps
  - bargraphs
  - animated waveforms (sine, noise, hybrid)

### Visual Demos Implemented
- Animated text
- Stylized keyboard (Elektron-style)
- Animated sine waveform (LUT-based, integer only)
- Animated pseudo-random / noise waveform (chaos controlled)

### Validation Status
✅ Hardware validated  
✅ Driver validated  
✅ Performance validated  
✅ Ready for integration with audio engine  

The OLED driver is considered **stable and complete** for the current product scope.
No further changes required unless new UI requirements arise.

