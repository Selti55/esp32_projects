# 🌤️ Wetterstation ESP32 mit Solar-Betrieb (Arduino IDE)

Eine komplette Wetterstation mit Außen- und Innenstation, die per ESP-NOW kommuniziert.

## 📋 Projektübersicht

### Außenstation
- **ESP32** mit Deep Sleep (15 Minuten Zyklus)
- **BME280** Sensor (Temperatur, Luftfeuchtigkeit, Luftdruck)
- **Solarbetrieben** mit 18650 Akku
- **ESP-NOW** Kommunikation zur Innenstation

### Innenstation  
- **ESP32** mit 3.5" TFT Touch Display
- **Anzeige** von Wetterdaten, BTC-Kurs und Blockzeit
- **USB-Stromversorgung**

## 🛒 Hardware

Siehe [hardware/bom_aliexpress.html](hardware/bom_aliexpress.html) für die komplette Einkaufsliste.

**Gesamtkosten:** ~48-71€

## ⚡ Stromverbrauch Außenstation

- **Deep Sleep:** 10μA
- **Aktive Phase:** 80mA (15 Sekunden alle 15 Minuten)  
- **Täglicher Verbrauch:** ~5mAh
- **Akku-Laufzeit:** 600 Tage (theoretisch)

## 🔧 Entwicklungsumgebung

- **Arduino IDE 2.x** empfohlen
- **ESP32 Board Support** installiert
- **Benötigte Bibliotheken** siehe docs/libraries.md

## 🚀 Schnellstart

1. **Hardware** nach Schaltplan aufbauen
2. **Arduino IDE** mit ESP32 Support einrichten
3. **Bibliotheken** installieren
4. **Sketch** uploaden
5. **ESP-NOW** Paarung durchführen

## 📁 Projektstruktur
