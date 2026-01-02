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

## 2. Vision produit (figée)

Le projet vise à développer une **machine audio temps réel de type Octatrack**, pensée pour :

* **Performance live**
* **Robustesse absolue**
* **Latence faible et déterministe**
* **Architecture modulaire par cartouches**

La machine se comporte comme une **console audio + séquenceur**, capable de piloter et d’intégrer des **modules DSP externes (cartouches Ksoloti)**, de manière fiable et reproductible, sans dépendre d’un ordinateur.

> Objectif assumé :
> *« un musicien doit pouvoir monter sur scène avec cette machine en toute confiance »*

---

## 3. Architecture audio globale (verrouillée)

### Paramètres audio

* Fréquence : **48 kHz**
* Taille de bloc : **64 samples**
* Audio **hard real-time**, prioritaire sur toute autre tâche
* Aucun traitement audio critique dans des threads RTOS classiques

### Pilotage audio

* Audio déclenché par **DMA SAI (IRQ half / full)**
* Traitement audio dans une fonction dédiée appelée à cadence fixe
* UI, MIDI, LEDs, etc. sont **soft real-time**

---

## 4. Pistes audio (décision réaliste)

### Audio interne (STM32H7)

* **8 pistes stéréo garanties simultanément**
* FX **globaux en sends** (pas par piste)
* Mixage, routing et looper local

### Audio externe (cartouches Ksoloti)

* Les cartouches assurent :

  * DSP lourd
  * synthèse
  * FX complexes
* Le STM32H7 agit comme :

  * séquenceur
  * routeur
  * mixeur léger

### Nombre de cartouches audio actives

* **1 à 2 cartouches audio actives simultanément** (temps réel garanti)
* Plus possible techniquement, mais **non garanti live**
* Les autres cartouches peuvent rester connectées pour :

  * paramètres
  * contrôle
  * états

---

## 5. Bus cartouches — SPI-link (choix final)

### Décision

> ❗ **Le bus SPI-link est conservé comme bus audio et contrôle des cartouches.**

Raisons :

* Ksoloti est nativement esclave
* Le protocole SPI-link est déjà éprouvé
* Éviter toute modification lourde du firmware Ksoloti
* Le SPI-link est utilisé **à la limite de ses capacités**, mais de manière consciente

### Contraintes assumées

* Pas de parallélisme réel entre plusieurs SPI audio
* Latence et charge **cumulatives**
* SPI utilisé comme **bus audio synchrone**, pas comme bus de contrôle best-effort

### Règles SPI-link

* SPI-link déclenché **sur chaque bloc audio**
* DMA obligatoire
* Double buffer RX/TX
* Aucune logique UI dans le chemin SPI audio
* SPI-link = **audio critique**, priorité élevée

---

## 6. Mémoire

### SDRAM externe

* Référence : **W9825G6KH-6I (256 Mbit / 32 MB, x16)**
* Driver : **ChibiOS-Contrib HAL SDRAM + FSMCv1**
* État : ✅ **SDRAM fonctionnelle (validation atomique OK)**

#### Règles d’utilisation (NON négociables)

* ✅ Accès **32-bit uniquement** (`uint32_t*`)
* ❌ Interdit : accès `uint16_t*` / `uint8_t*` (aliasing / writes non fiables sur STM32H7)
* ⚠️ Particularité STM32H7 + SDRAM x16 : **les demi-mots sont inversés** sur les accès 32-bit  
  Exemple : écrire `0x11223344` → relire `0x33441122` (normal).
* ✅ Pour travailler “comme d’habitude”, utiliser un wrapper qui applique un **swap16** à l’écriture/lecture.

#### Module projet (config bétonnée)

* Fichiers : `sdram_ext.c / sdram_ext.h`
* API :
  * `sdram_ext_init()` (appelle `sdramInit()` + `sdramStart(&SDRAMD1, &sdram_cfg)`)
  * `sdram_ext_write32(index, value)` / `sdram_ext_read32(index)` (gèrent le swap16)
* Base : `SDRAM_EXT_BASE = 0xC0000000`

#### Usage prévu (audio uniquement)

* buffers
* looper
* delay
* granular
* ❌ Pas de heap
* ❌ Pas d’objets UI
* ❌ Pas de structures système

### SRAM interne

* UI
* contrôle
* états
* drivers

---
## 7. Interface utilisateur

### Entrées

* Switchs tactiles classiques
* **16 pads à capteurs Hall Effect analogiques**

  * 2 multiplexeurs 8 canaux
  * calcul de vélocité par mesure temporelle (Δt)
* 1 multiplexeur 8 canaux séparé pour :

  * potentiomètres assignables
* 4 encodeurs rotatifs

  * décodage logiciel
  * pas de quadrature HW
  * pas d’EXTI (audio prioritaire)

### Sorties

* OLED SPI 2.4"
* **25 LEDs adressables WS2812C-2020**

  * pilotage DMA + timer
  * mises à jour asynchrones
  * jamais dans le chemin audio

---

## 8. Audio I/O interne

* **SAI2A** :

  * maître horloge
  * 2 codecs ADC en daisy-chain
  * jusqu’à 4 entrées stéréo
* **SAI2B** :

  * codec DAC
  * sorties stéréo
* 2 sorties stéréo indépendantes et routables

---

## 9. Connectivité

* MIDI DIN :

  * UART5 RX/TX
* USB :

  * device MIDI
  * audio USB envisagé plus tard (hors scope immédiat)
* USB host :

  * MIDI class compliant

---

## 10. État global actuel — VALIDÉ

### Boot / Clock

* Boot CPU validé
* Séquence :

```
Reset → halInit() → chSysInit() → main()
```

* Clock **HSI + PLL1** (mode debug sûr)
* Fréquences vérifiées via UART :

  * SYS_CK
  * HCLK
  * PCLK1–4

### Debug

* UART1 (USART1 / SD1) 100 % fonctionnel
* Baudrate configuré explicitement
* UART = **outil central de validation**

### LED debug

* LED sur **PH7**, active-low
* Fonctionnelle
* Preuve visuelle minimale de vie système

---

## 11. FMC / SDRAM — règle non négociable

* Pins FMC définies dans `board.h`
* ❌ **Aucune initialisation SDRAM dans `boardInit()`**
* SDRAM initialisée :

  * dans **`sdram_ext.c`** (module dédié)
  * explicitement depuis `main()` ou un thread contrôlé
  * avec logs UART (étapes de bring-up)
  * en gardant le chemin d’init simple : `sdramInit()` → `sdramStart(&SDRAMD1, &sdram_cfg)`

* Validation minimale de référence (doit toujours passer) :
  * écrire 1 mot : `0x11223344`
  * relire : `0x33441122` (swap demi-mots normal sur x16)

---

## 12. Cache / MPU — politique projet

* Objectif actuel : **validation fonctionnelle** → garder les tests *simples et déterministes*.
* ⚠️ Attention STM32H7 : si le **D-Cache** est activé par le startup, une SDRAM externe peut donner des tests “faux FAIL” tant que le **MPU** n’a pas marqué la zone SDRAM avec les bons attributs.
* Politique projet :

  * ✅ Phase bring-up SDRAM : **cache OFF** (ou SDRAM marquée non-cacheable via MPU)
  * ✅ Phase production : cache ON + MPU (Write-Back/Write-Allocate) + **maintenance cache explicite** sur buffers DMA (clean/invalidate par adresse)

---

## 13. Philosophie projet (non négociable)

* ❌ Pas de bidouille HAL
* ❌ Pas d’init lourde avant `main()`
* ❌ Pas de logique audio dans l’UI
* ❌ Pas de heap dynamique dans l’architecture finale
* ✅ Tout périphérique critique est initialisé explicitement
* ✅ Toute init est observable via UART
* ✅ Fiabilité live > performance brute
* ✅ Ksoloti = pilier central du concept “cartouche”

---

## 14. Prochaines étapes recommandées

1. ✅ SDRAM : module `sdram_ext.c / sdram_ext.h` (config verrouillée + helpers 32-bit)
2. ✅ SDRAM : test atomique de référence (write/read 1 mot) OK
3. 🔜 Ajouter un **test SDRAM plus long** (patterns + pseudo-random) en mode *cache OFF* ou SDRAM *non-cacheable MPU*
4. Finaliser drivers UI (ADC, MUX, OLED, LEDs)
5. Mettre en place le **squelette audio DMA**
6. Intégrer SPI-link audio **1 cartouche**
7. Étendre prudemment à 2 cartouches audio
8. Plus tard :

   * MPU + cache (mode production, cohérence DMA)
   * optimisation
   * USB audio (optionnel)

---
## 15. État final actuel

> 🟢 Socle CPU / clock / debug sain
> 🟢 Vision produit claire et réaliste
> 🧱 Architecture audio et cartouches verrouillée
> 🧠 Compromis techniques assumés et documentés

---

