# Projektname

## Hardware
- ESP32 Board:   TTGO T5 V2.3 + GDEM0213B74 (SSD1680) + DHT22 + RemoteXY WiFi AP
				© 2025 – stromsparende Variante ohne RemoteXY
				Getestet mit GxEPD2 1.6.5 und DHT Sensor Library 1.4.6

				Info:
				https://lilygo.cc/products/t5-v2-3-1?srsltid=AfmBOops9zqapfBA03_gsPr8TM6WmtTbr4zUauHAhwKXpLUF-_hDIHp1
- Sensoren: 	AM2302
- Verkabelung: 	siehe Sketch

## Funktion
So funktioniert’s
Phase	        Beschreibung
Start	        ESP32 wacht aus Deep-Sleep auf
Messung	      	DHT22 liefert Temperatur & Luftfeuchtigkeit
Anzeige	      	Werte werden auf dem E-Paper aktualisiert
Schlafmodus	  	Gerät schläft 5 Minuten und misst dann erneut

## Verwendung