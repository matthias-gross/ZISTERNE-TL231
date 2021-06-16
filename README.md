# ZISTERNE - TL231

Zielsetzung:
Es soll gemessen werden, wie der aktuelle Füllstand der Zisterne ist.
Hierfür wird ein TL231 verwendet.

Der TL231 wird mit 24 Volt betrieben und liefert je nach Füllstand der Zisterne eine unterschiedliche Stromstärke zurück.
* Bei   0cm Wasserstand werden  4mA zurück geliefert
* Bei 500cm Wasserstand werden 20mA zurück geliefert

![Skizze](https://github.com/matthias-gross/ZISTERNE-TL231/blob/master/Skizze.jpeg)

In Release v1.0 wurde die Grundlogik für ESP32, ESP8266 und Arduino Nano implementiert

Plan für Release 2.0:
* Fokussierung auf ESP32
* Integration Wlan
* Integration MQTT
