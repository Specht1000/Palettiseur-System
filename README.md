# 🏭 Palettiseur System – Factory I/O & STM32

## 📌 Description du projet

Ce projet implémente un **système automatisé de palettisation** simulé sous **Factory I/O**, piloté par un **microcontrôleur STM32F072RB** exécutant **FreeRTOS**.  
L’objectif est de contrôler le flux de cartons, la constitution de couches, la gestion d’un élévateur, d’une porte et d’un système de clamp afin de déposer **deux couches de 6 cartons** sur une palette.

Le système communique avec Factory I/O via un **bridge logiciel (Factory IO VS Bridge)** et repose sur une architecture **temps réel multitâche**.

---

## 🎯 Objectifs pédagogiques

- Mise en œuvre d’un système **temps réel embarqué**
- Utilisation de **FreeRTOS** (tasks, sémaphores, files de messages)
- Synchronisation entre capteurs et actionneurs
- Communication PC ↔ microcontrôleur (UART + DMA)
- Modélisation industrielle sous **Factory I/O**
- Gestion d’un cycle de palettisation réaliste

---

## ⚙️ Fonctionnement global

### 🔄 Cycle de palettisation

1. Génération et acheminement des cartons
2. Constitution de groupes de **2 cartons**
3. Formation de **6 cartons (3 poussées)**
4. Arrivée des 6 cartons sur la porte fermée
5. **Clamp activé pendant que les cartons sont encore sur la porte**
6. Ouverture de la porte → chute des cartons sur la palette
7. Ajustement par le clamp
8. Descente partielle de l’élévateur
9. Fermeture de la porte
10. Arrivée de la seconde couche de 6 cartons
11. Clamp avant ouverture
12. Ouverture de la porte → chute sur la première couche
13. Descente finale de l’élévateur jusqu’au RDC
14. Évacuation de la palette

---

## 🧵 Architecture logicielle

Le projet est structuré autour de **plusieurs tâches FreeRTOS** :

| Tâche | Rôle |
|-----|------|
| `vTaskBoxGenerator` | Génération périodique des cartons |
| `vTaskGateAndPusher` | Gestion de la barrière et du chargement par paire |
| `vTaskPoussoir2Boxes` | Poussée de 2 cartons (3 fois = 6 cartons) |
| `vTaskAscenseur` | Gestion complète de l’élévateur, porte et clamp |
| `vTaskPalette` | Distribution et évacuation des palettes |
| `vTaskRead` | Lecture des capteurs Factory I/O |
| `vTaskWrite` | Envoi des commandes actionneurs (UART + DMA) |

La synchronisation est assurée par :
- **Sémaphores binaires**
- **Files de messages**
- **Abonnements dynamiques aux capteurs**

---

## 🧱 Technologies utilisées

- **STM32F072RB**
- **FreeRTOS**
- **Factory I/O**
- **C (bare-metal + RTOS)**
- **UART + DMA**
- **Tracealyzer / TraceRecorder**
- **STM32CubeIDE**

---

## 📂 Structure du projet

```text
FactoryIO_Final-First-Version/
├── app/                    # Code applicatif (tasks FreeRTOS, main)
├── bsp/                    # Board Support Package
├── FreeRTOS/               # Noyau FreeRTOS
├── cmsis/                  # CMSIS & startup STM32
├── TraceRecorder/          # Outils de traçage temps réel (Tracealyzer)
├── factoryio_vsbridge-main/ # Bridge de communication Factory I/O
├── .project
├── .cproject
└── STM32F072RBTx_FLASH.ld
```

---

## ▶️ Exécution du projet

1. Lancer **Factory I/O**
2. Charger la scène de palettisation
3. Démarrer le **Factory IO VS Bridge**
4. Flasher le STM32 avec le firmware
5. Lancer la simulation

---

## 📊 Debug & analyse

- Traces temps réel via **Tracealyzer**
- Visualisation des états des tâches
- Analyse des sémaphores et latences
- Debug possible via **STM32CubeIDE**

---

## 👥 Auteurs

- **Andrew Santos Machado**
- **Guilherme Martins Specht**

---

## 📜 Licence

Projet académique — utilisation pédagogique et démonstrative.

---

## 📝 Remarques

- Le clamp est intentionnellement activé **avant l’ouverture de la porte** afin d’assurer la stabilité des cartons.
- Le comportement est synchronisé pour éviter tout chevauchement mécanique.
- Le système est conçu pour être **robuste face aux délais et à l’ordre d’arrivée des capteurs**.

---
