# Projektname

## Hardware
- ESP32 Board:	TTGO T5 V2.3 + GDEM0213B74 (SSD1680) + DHT22 + Deep Sleep + Wake Button
				© 2025 – stromsparende Variante mit manuellem Aufwachen
				Getestet mit GxEPD2 1.6.5 und DHT Sensor Library 1.4.6

				Info:
				https://lilygo.cc/products/t5-v2-3-1?srsltid=AfmBOops9zqapfBA03_gsPr8TM6WmtTbr4zUauHAhwKXpLUF-_hDIHp1
- Sensoren: 	AM2302
- Verkabelung: 	
				🟢 Kurz gesagt:
				Ein Bein des Tasters an GPIO13 (VN)
				Das andere Bein an GND

## Funktion
   - misst Temperatur & Luftfeuchtigkeit
   - zeigt Werte groß auf dem 2.13" E-Paper an
   - geht 5 Minuten in Deep Sleep
   - kann zusätzlich durch Button (GPIO13) geweckt werden
   
   🧩 Grundprinzip
		Der ESP32 wacht aus Deep Sleep auf, wenn der konfigurierte GPIO39 (VN)
		→ auf LOW gezogen wird (also gegen GND).

Das bedeutet:
		Der Pin GPIO13 (VN) liegt standardmäßig HIGH (intern)
		Ein Taster gegen GND löst den Wake-up aus

## Verwendung

