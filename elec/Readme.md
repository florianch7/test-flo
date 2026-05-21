# Elec2025-26
elec 2025 2026 comme dans le titre en fait

# Pinout ESP32-Devkit

Ce guide fournit une référence exhaustive pour l'utilisation des broches du module **ESP32-DevKitC-32D**. Il récapitule les fonctions de chaque broche, classe les GPIO par niveau de sécurité, et liste les règles de conception essentielles pour éviter les comportements erratiques au démarrage (boot) ou le blocage du téléversement.
Lien vers un sheets qui est plutôt clair : https://docs.google.com/spreadsheets/d/1OCtBKPCofokwxsqIV4q3fY71EqnTtfXIetnAtifBfJc/edit?usp=sharing

---

## 🗺️ 1. Synthèse Globale du Pinout

> [!CAUTION]
> **DANGER CRITIQUE POUR LE MATÉRIEL :**
> **NE JAMAIS ENVOYER PLUS DE 3.3V A AUCUNE DES PINS GPIO de l'ESP32.**
> Les broches ne sont pas tolérantes au 5V. Toute tension supérieure à 3.3V détruira instantanément la broche ou la puce entière. Utilisez un pont diviseur ou un Level Shifter pour les signaux autres.

### 🔌 Broches d'Alimentation et Contrôle Physique
Ces broches ne sont pas programmables. Elles gèrent l'infrastructure électrique et logique de la carte.

| Nom sur Schéma | Fonction Matérielle | Recommandations de Conception |
| :--- | :--- | :--- |
| **VIN** | Entrée d'alimentation principale (5V) | Liée au régulateur de tension interne. À utiliser pour alimenter la carte sans câble USB. |
| **3V3** | Sortie ou Entrée d'alimentation 3.3V | Sortie du régulateur interne ou entrée régulée. Ne jamais injecter plus de 3.3V sous peine de destruction. |
| **GND / GND1** | Masse de référence (0V) | Point commun pour tous les signaux et alimentations du système. |
| **EN** | Équivalent Hardware Reset | Maintenu à 3.3V en interne. Forcer à la masse (GND) éteint la puce. Une transition bas ➔ haut déclenche un Hard Reset. |

---

## 🚦 2. Classification des GPIO par Niveau de Sécurité

### 🟢 Liste Verte : 100% Sûres (Aucune restriction)
Ces broches sont entièrement bidirectionnelles, acceptent le mode `INPUT`, `OUTPUT`, les interruptions matérielles, ainsi que les résistances de tirage (*pull-up* / *pull-down*) internes. **À utiliser en priorité absolue.**

| Nom | N° GPIO | Fonctions Spécifiques / Alternatives |
| :--- | :--- | :--- |
| **IO13** | GPIO 13 | Entrée/Sortie standard |
| **IO14** | GPIO 14 | Sort du PWM au boot (Léger glitch possible à la mise sous tension) |
| **IO16** | GPIO 16 | Entrée/Sortie standard |
| **IO17** | GPIO 17 | Entrée/Sortie standard |
| **IO19** | GPIO 19 | Entrée/Sortie standard (Souvent utilisé pour SPI MISO) |
| **IO21** | GPIO 21 | Bus I2C Matériel : **SDA** (Par défaut) |
| **IO22** | GPIO 22 | Bus I2C Matériel : **SCL** (Par défaut) |
| **IO23** | GPIO 23 | Entrée/Sortie standard (Souvent utilisé pour SPI MOSI) |
| **IO25** | GPIO 25 | Intègre un DAC (Convertisseur Numérique-Analogique matériel) |
| **IO26** | GPIO 26 | Intègre un DAC (Convertisseur Numérique-Analogique matériel) |
| **IO27** | GPIO 27 | Entrée/Sortie standard |
| **IO32** | GPIO 32 | Entrée/Sortie standard |
| **IO33** | GPIO 33 | Entrée/Sortie standard |

### 🟡 Liste Orange : Entrées Uniquement (*Input Only*)
Ces broches sont uniquement capables de lire un signal électrique. La commande `pinMode(X, OUTPUT)` n'a aucun effet physique dessus.

| Nom | N° GPIO | Caractéristiques Critiques |
| :--- | :--- | :--- |
| **IO34** | GPIO 34 | **Pas de pull-up/pull-down interne**. État flottant sans résistance externe. |
| **IO35** | GPIO 35 | **Pas de pull-up/pull-down interne**. État flottant sans résistance externe. |
| **VP** | GPIO 36 | Sensor VP. **Pas de pull-up/pull-down interne**. Entrée idéal analogique. |
| **VN** | GPIO 39 | Sensor VN. **Pas de pull-up/pull-down interne**. Entrée idéal analogique. |

> 💡 **Le Super-Pouvoir de l'ADC1 :** Contrairement aux broches de l'ADC2, les entrées analogiques de l'ADC1 (**GPIO 34, 35, 36, 39**) restent parfaitement fonctionnelles et stables lorsque le module Wi-Fi de l'ESP32 est actif. Utilisez-les en priorité pour vos capteurs critiques (potentiomètres, mesures analogiques, joysticks).

### 🔴 Liste Rouge : Les Strapping Pins (Risque de blocage au boot)
Ces broches agissent comme des configurateurs matériels au moment précis de l'allumage. **Règle d'or : Ne jamais les câbler comme entrées reliées à des commutateurs ou capteurs actifs au démarrage.** Si nécessaire, utilisez-les préférentiellement en **Sortie (Output)** pour des composants non critiques.

| Nom | N° GPIO | Rôle au Démarrage / Comportement Attendu | Risque en cas d'erreur matérielle |
| :--- | :--- | :--- | :--- |
| **IO0** | GPIO 0 | **Boot Mode Select**. Haut = Exécution Normale. Bas = Mode Flashage/Téléchargement. | Bloque le téléversement ou empêche le code utilisateur de s'exécuter de manière autonome. |
| **IO2** | GPIO 2 | Doit être maintenu à l'état **BAS** (ou flottant) pour entrer en mode Flash. | Conjointement au GPIO 0, si forcé à l'état haut au boot, le flashage échoue. |
| **IO5** | GPIO 5 | Configure le timing du mode esclave SDIO. | Peut empêcher le processeur d'initialiser correctement certains bus. |
| **IO12** | GPIO 12 | **Tension Flash (VDD_SDIO)**. Haut = 1.8V. Bas = 3.3V (Nécessaire pour le module WROOM). | **Boot fails if high** : Si tiré vers le haut par un composant au démarrage, la mémoire flash manque de tension ➔ crash immédiat. |
| **IO15** | GPIO 15 | Contrôle l'émission des logs de debug de démarrage sur UART0. | Émet des micro-impulsions au boot (peut faire brièvement clignoter une LED ou commuter un transistor). |

### ❌ Liste Noire : Interdiction Absolue (Mémoire Interne SPI)
**Ne tentez jamais de les déclarer ou de les manipuler dans votre code.**
* **GPIO 1 (TX0) & GPIO 3 (RX0) :** Réservées au convertisseur USB-Série de la carte de développement. Les utiliser bloque le téléversement du code et casse la communication avec le Moniteur Série.

### Liste a éviter : Pins high au boot
Ces broches sont high (à 3.3V) pendant le boot & le setup. Connectez un composant (un moteur par exemple) dessus le fera démarrer à chaque upload du code. C'est chiant, on évite si c'est un composant qui reçoit du courant et qui produit une action. Un interrupteur, une led c'est ok par exemple
* [1 | 3 | 5 | 14 | 15] **GPIO 1 (TX0) & GPIO 3 (RX0) & GPIO 5 & GPIO 14 & GPIO 15 :** Sont high au boot & envoie des signaux pwm ou high

<p align="center">
  <img src="https://external-content.duckduckgo.com/iu/?u=https%3A%2F%2Fi0.wp.com%2Frandomnerdtutorials.com%2Fwp-content%2Fuploads%2F2018%2F08%2FESP32-DOIT-DEVKIT-V1-Board-Pinout-30-GPIOs-Copy.png&f=1&nofb=1&ipt=6c1dc10194768ea43f24a796ffc2759b4e598877d3e1bd53caf7a5593b3b71ef" alt="Pinout ESP32 DOIT DevKit V1" width="600">
</p>


---