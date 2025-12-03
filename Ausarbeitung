# Protokolle im Überblick

## Inhaltsverzeichnis

- [1. CORBA](#1-corba)
  - [1.1 Überblick](#11-überblick)
  - [1.2 Funktionsweise](#12-funktionsweise)
  - [1.3 Typische Einsatzgebiete](#13-typische-einsatzgebiete)
  - [1.4 Typische Paketgrößen](#14-typische-paketgrößen)
- [2. MQTT](#2-mqtt)
  - [2.1 Überblick](#21-überblick)
  - [2.2 Funktionsweise](#22-funktionsweise)
  - [2.3 Typische Einsatzgebiete](#23-typische-einsatzgebiete)
  - [2.4 Typische Paketgrößen](#24-typische-paketgrößen)
- [3. Entscheidung](#3-entscheidung)

---

Um eine fundierte Entscheidungsgrundlage zu schaffen, werden die beiden vorgeschlagenen Protokolle mit Message-Broker-Charakter in Kurzform dargestellt. Dabei werden die wesentlichen Funktionsweisen, typischen Anwendungsbereiche sowie markanten Merkmale der jeweiligen Protokolle beschrieben.

Im Anschluss wird die Entscheidung erläutert, welches der beiden Protokolle für die weitere Bearbeitung der Aufgabenstellung verwendet wird. Diese Entscheidung wird anhand der aufgeführten Eigenschaften begründet.

---

# 1. CORBA

## 1.1 Überblick

CORBA (Common Object Request Broker Architecture) ist kein einzelnes Protokoll im klassischen Sinne, sondern ein Standard für eine Middleware-Architektur, die die Kommunikation zwischen verteilten Softwarekomponenten ermöglicht. Sie ist unabhängig von Programmiersprachen, Betriebssystemen und Hardwareplattformen.  
Das konkrete Netzwerkprotokoll, das CORBA zur Interoperabilität nutzt, heißt **Internet Inter-ORB Protocol (IIOP)** und basiert auf TCP/IP.

## 1.2 Funktionsweise

Das zentrale Element der CORBA-Architektur ist der **Object Request Broker (ORB)**. Er vermittelt sämtliche Anfragen und Antworten zwischen Clients (Nutzern eines Dienstes) und Servern (Anbietern eines Dienstes).

Der Kommunikationsprozess umfasst folgende Schritte:

1. **Schnittstellendefinition (IDL)**  
   Die Schnittstelle eines Server-Objekts wird mittels der Interface Definition Language (IDL) definiert. Die Beschreibung ist sprachneutral und legt fest, welche Operationen verfügbar sind.

2. **Generierung von Stubs und Skeletons**  
   Aus der IDL entstehen sprachspezifische Codefragmente:  
   - **Stubs (Client)**: verbergen die Netzwerkkommunikation und ermöglichen entfernte Methodenaufrufe wie lokale Aufrufe.  
   - **Skeletons (Server)**: empfangen Anfragen, entpacken Daten und rufen die eigentliche Implementierung auf.

3. **Objektreferenz und Naming Service**  
   Der Server registriert seine Objekte beim Naming Service. Clients erhalten anhand dessen die notwendige Objektreferenz.

4. **ORB als Vermittler**  
   Der Client ruft eine entfernte Methode über den Stub auf. Der ORB leitet die Anfrage anhand der Objektreferenz weiter.

5. **Datenübertragung über IIOP**  
   Die Anfrage wird serialisiert (marshalled) und über IIOP zum Server übertragen.

6. **Ausführung und Rückgabe**  
   Der Server verarbeitet die Anfrage, die Antwort wird denselben Weg zurück übermittelt.

Durch diesen Mechanismus entsteht **Transparenz der Lokalität**: Aus Sicht des Entwicklers verhalten sich entfernte Objekte wie lokale Instanzen.

## 1.3 Typische Einsatzgebiete

CORBA wird heute vor allem in spezialisierten oder historisch gewachsenen Systemen genutzt:

- **Legacy-Systeme**  
  Weiterbetrieb und Integration älterer CORBA-basierter Software.

- **Echtzeit- und eingebettete Systeme**  
  z. B. Luft- und Raumfahrt (Hubble-Teleskop, Flugzeugsteuerungen).

- **Große Unternehmensinfrastrukturen**  
  heterogene Landschaften in Finanz- und Versicherungssektor.

- **Industrielle Automatisierung**  
  Vernetzung von Maschinen und Steuerungssystemen.

## 1.4 Typische Paketgrößen

CORBA definiert keine feste Paketgröße. Die tatsächliche Größe hängt ab von:

- **Anwendungsdaten**  
  (von wenigen Bytes bis zu großen Arrays/Strukturen)

- **Netzwerk-MTU**  
  typischerweise 1500 Byte → größere Nachrichten werden fragmentiert

- **ORB-Konfiguration**  
  z. B. Standard-GIOP-Fragmentgröße: ca. 1024 Byte

- **Protokoll-Overhead**  
  GIOP/IIOP-Header (12 Byte) + TCP- und IP-Header

In der Praxis reichen CORBA-Nachrichten von wenigen Dutzend Bytes bis zu mehreren Megabyte (inkl. Fragmentierung).

---

# 2. MQTT

## 2.1 Überblick

MQTT (Message Queuing Telemetry Transport) ist ein leichtgewichtiges, offenes Publish/Subscribe-Protokoll und wurde speziell für Machine-to-Machine-Kommunikation (M2M) und das Internet der Dinge (IoT) entwickelt.  
Es ist optimiert für Umgebungen mit geringer Bandbreite, hoher Latenz oder unzuverlässigen Netzwerkverbindungen.

## 2.2 Funktionsweise

MQTT basiert auf einer zentralen Broker-Architektur:

- **Publisher**  
  sendet Nachrichten zu einem Topic.

- **Subscriber**  
  empfängt Nachrichten, indem er Topics abonniert.

- **Broker**  
  vermittelt, filtert und verteilt Nachrichten.

Sender und Empfänger sind zeitlich und räumlich entkoppelt.

## 2.3 Typische Einsatzgebiete

MQTT ist heute ein De-facto-Standard im IoT und wird in vielen Bereichen eingesetzt:

- Allgemeines IoT / Sensorik  
- Smart Home und Gebäudeautomation  
- Industrielle Automation (IIoT)  
- Telemetrie über langsame oder instabile Verbindungen  
- Logistik und Tracking  
- Wearables und Medizintechnik  
- Cloud-Plattformen (AWS IoT, Azure IoT, Google Cloud)

Die Beliebtheit von MQTT begründet sich durch:

- sehr geringen Protokoll-Overhead  
- effizientes und skalierbares Publish/Subscribe-Modell  
- verschiedene QoS-Stufen für zuverlässige Nachrichtenübermittlung

## 2.4 Typische Paketgrößen

Ein MQTT-Paket besteht aus:

- **Fixed Header** (mind. 2 Byte)  
- **Variable Header** (größenabhängig)  
- **Payload** (0 Byte bis mehrere MB)

Typische Größen:

- **kleine Status/Sensorwerte**: wenige Dutzend Bytes  
- **JSON-Nachrichten**: einige 100 Byte bis wenige KB  
- **Maximalgröße**: 256 MB (MQTT-Spezifikation)

Wie bei allen TCP/IP-basierten Protokollen werden große MQTT-Pakete auf Netzwerkebene fragmentiert (MTU typ. 1500 Byte).

---

# 3. Entscheidung

Auf Basis der analysierten Protokolleigenschaften hat sich die Gruppe entschieden, **MQTT** als Trägerprotokoll für die Umsetzung des Tunnels zu verwenden.

Die wichtigsten Gründe sind:

- **Breites und modernes Anwendungsspektrum**, insbesondere im IoT  
- **Effiziente, leichtgewichtige Nachrichtenstruktur**  
- **Einfache Integration** über zahlreiche Bibliotheken und Programmiersprachen  
- **Hohe Verbreitung** im privaten und industriellen Umfeld  
- **Zeitgemäße Architektur**, ideal für verteilte Systeme

Insbesondere die starke Präsenz von MQTT im Smart-Home-Bereich und in IoT-Plattformen (z. B. Home Assistant, ioBroker) zeigt die praktische Relevanz und Zukunftsfähigkeit dieses Protokolls.

---
