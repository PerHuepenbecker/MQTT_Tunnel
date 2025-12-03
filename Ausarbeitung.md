# Tunneln von Nachrichten innerhalb von Message-Broker-basierten Protokollen  
## Fachpraktikum 63585 – IT-Sicherheit, IT-Forensik und Datenschutz

### Gruppenaufgabe  
Per Huepenbecker  
Carsten Lenzen  
Robin Wetzlar

---

# Inhaltsverzeichnis

- [Aufgabenstellung](#aufgabenstellung)  
- [Protokolle im Überblick](#protokolle-im-überblick)  
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
- [Entscheidung](#entscheidung)  
- [Quellenverzeichnis](#quellenverzeichnis)

---

# Aufgabenstellung

Es gibt verschiedene Wege und Protokolle, über die sich ein Tunnel realisieren lässt. Empfehlenswert ist ein Protokoll, das mit hoher Wahrscheinlichkeit trotz Firewalls oder Proxies in einem Netzwerk verfügbar ist. Dadurch reduziert sich die Auswahl möglicher Kandidaten.

Typische und häufig genutzte Protokolle wären:

- HTTP / HTTPS  
- DNS  

Diese sollen im Rahmen dieses Praktikums **nicht genutzt** werden. Stattdessen soll ein „Exot“ ausgewählt werden: ein Protokoll, das **einen Message-Broker verwendet**, wie:

- **MQTT**,  
- **CORBA**,  
- oder ein anderes brokerbasiertes Protokoll.

Die Aufgabenstellung fordert:

1. Auswahl eines geeigneten Trägerprotokolls und Begründung dieser Entscheidung.  
2. Entscheidung, ob allgemeine TCP/UDP/IP-Verbindungen oder nur HTTP(S) getunnelt werden sollen.  
3. Implementierung eines funktionierenden Tunnels.  
4. Dokumentation aller Design- und Implementationsentscheidungen.  
5. Test des Tunnels mittels z. B. Wireshark oder tcpdump.  
6. Dokumentation der Gruppenbeiträge.

---

# Protokolle im Überblick

Im Folgenden werden die beiden vorgeschlagenen Broker-basierten Protokolle **CORBA** und **MQTT** vorgestellt. Anschließend wird begründet, warum eines davon für die weitere praktische Umsetzung ausgewählt wurde.

---

# 1. CORBA

## 1.1 Überblick

CORBA (Common Object Request Broker Architecture) ist ein Standard für verteilte, objektorientierte Middleware. Er ermöglicht Kommunikation zwischen Softwarekomponenten unabhängig von Programmiersprache, Plattform und Betriebssystem [1][2][3].  

Für die Netzwerkkommunikation verwendet CORBA das **Internet Inter-ORB Protocol (IIOP)** auf Basis von TCP/IP.

## 1.2 Funktionsweise

Das zentrale Element der CORBA-Architektur ist der **Object Request Broker (ORB)**. Dieser vermittelt sämtliche Anfragen und Antworten zwischen Clients und Servern.

Ablauf der Kommunikation:

1. **Schnittstellendefinition (IDL)**: Mittels Interface Definition Language, sprachneutral beschrieben.  
2. **Generierung von Stubs und Skeletons**:  
   - *Stubs* auf Client-Seite kapseln Netzwerkdetails.  
   - *Skeletons* auf Server-Seite decodieren die Anfragen.  
3. **Objektreferenz und Naming Service**: Der Server registriert seine Objekte, Clients erhalten Referenzen.  
4. **ORB als Vermittler**: Weiterleitung der Methodenaufrufe anhand der Referenzen.  
5. **Datenübertragung über IIOP** (Marshalling + TCP/IP).  
6. **Antwort an den Client** über denselben Weg.

CORBA ermöglicht damit **Transparenz der Lokalität** — aus Sicht des Clients wirken entfernte Objekte wie lokale [1][2][3].

## 1.3 Typische Einsatzgebiete

CORBA wird heute vor allem eingesetzt in:

- Integration von **Legacy-Systemen** [5][6]  
- **Echtzeit- und eingebetteten Systemen** (z. B. Luft- und Raumfahrt) [5]  
- **Großen Unternehmensinfrastrukturen** (Finanz-/Versicherungssektor) [7]  
- Industrieller **Automatisierung** und Steuerungssystemen [8]

Es wird primär dort genutzt, wo Migration zu modernen Alternativen aufwändig oder riskant wäre [5][6][7][8].

## 1.4 Typische Paketgrößen

Die Nachrichtengröße hängt stark vom Anwendungskontext ab und ist nicht standardisiert:

- **Anwendungsdaten** bestimmen primär die Größe  
- **MTU** limitiert Paketgröße (typ. 1500 Byte)  
- ORBs nutzen interne **Fragmentierung** (oft bei 1024 Byte pro Fragment) [10]  
- **Header-Overhead**:  
  - GIOP/IIOP: 12 Byte  
  - TCP: ~20–32 Byte  
  - IP: ~20 Byte

Nachrichten können wenige Bytes bis mehrere Megabyte umfassen [1][2][10].

---

# 2. MQTT

## 2.1 Überblick

MQTT (Message Queuing Telemetry Transport) ist ein leichtgewichtiges Publish/Subscribe-Protokoll für Machine-to-Machine- und IoT-Kommunikation. Es ist optimiert für geringe Bandbreite, hohe Latenz und unzuverlässige Netzwerke [11][12].

## 2.2 Funktionsweise

MQTT basiert auf einer **Broker-Architektur** mit folgenden Rollen:

- **Publisher**: sendet Nachrichten zu einem Topic  
- **Subscriber**: empfängt Nachrichten durch Abonnement  
- **Broker**: vermittelt und verteilt Nachrichten  

Sender und Empfänger sind zeitlich und räumlich vollständig entkoppelt [11][13].

## 2.3 Typische Einsatzgebiete

MQTT findet Anwendung in:

- IoT allgemein  
- **Smart Home** und Gebäudeautomation (z. B. Home Assistant, OpenHAB)  
- **Industrieller Automation (IIoT)**  
- Telemetrie über unzuverlässige/teure Leitungen (z. B. Satellit)  
- Logistik & Asset Tracking  
- Wearables & Gesundheitswesen  
- Cloud-Plattformen (AWS IoT, Azure, Google Cloud) [11][14][15][16][17]

Gründe für die hohe Verbreitung:

- sehr geringer Overhead  
- effizientes Publish/Subscribe-Modell  
- verschiedene QoS-Stufen zur Zuverlässigkeitssicherung

## 2.4 Typische Paketgrößen

Ein MQTT-Paket besteht aus:

- **Fixed Header** (mind. 2 Byte)  
- **Variable Header**  
- **Payload** (Sensorwerte, JSON-Daten etc.)

Typische Größen:

- Sensorwerte: wenige Dutzend Bytes  
- JSON-Daten: wenige KB  
- Maximalgröße (MQTT 3.1.1): 256 MB [11]

Große Daten werden aufgrund der **MTU** fragmentiert (typ. 1500 Byte).

---

# Entscheidung

Die Gruppe hat sich auf Basis der analysierten Protokolleigenschaften für **MQTT** entschieden.

Begründung:

- modernes, leichtgewichtiges Protokoll  
- hohe Effizienz durch geringen Overhead  
- hervorragende Eignung für verteilte Systeme  
- breite Verbreitung in IoT, Smart Home, Industrie  
- robuste Broker-Infrastruktur und gute Client-Bibliotheken  
- einfache Integration in eigene Softwareprojekte

Insbesondere durch die starke Nutzung in Smart-Home- und IoT-Umgebungen ist MQTT praktisch relevant und sehr gut geeignet als Trägerprotokoll für das Tunneln von Nachrichten.

---
# 3. Systemdesign des MQTT-Tunnels

## 3.1 Zielsetzung

## 3.2 Architekturübersicht

## 3.3 Kommunikationsmodell

## 3.4 Protokolldefinition und Datenformat

## 3.5 Einsatz eines TUN-Devices

## 3.6 Sicherheitsbetrachtung

---

# 4. Implementierung

## 4.1 Entwicklungsumgebung

## 4.2 Aufbau der Software

## 4.3 TUN-Device-Anbindung

## 4.4 MQTT-Integration

## 4.5 Programmablauf

---

# 5. Tests und Analyse

## 5.1 Funktionale Tests

## 5.2 Performance und Latenz

## 5.3 Wireshark-Analyse

## 5.4 Fehlerfälle

---

# 6. Fazit

---

# Quellenverzeichnis

1. https://docs.oracle.com/cd/A97335_02/apps.102/a83722/overvie3.htm  
2. https://www.ibm.com/docs/en/app-connect/11.0.0?topic=corba-common-object-request-broker-architecture  
3. https://refubium.fu-berlin.de/bitstream/handle/fub188/4102/12_chap12.pdf  
4. https://learn.microsoft.com/de-de/dotnet/standard/native-interop/type-marshalling  
5. https://www-ois-com.translate.goog/index.php/about-corba  
6. https://www.elpassion.com/de/glossary/what-is-common-object-request-broker-architecture-corba  
7. https://www.omg.org/corba/faq.htm  
8. https://hilfe.comarch.de/cee/index.php/documentation/corba-schnittstelle  
9. https://www.arl.wustl.edu/Publications/2000-04/comp00fgk.pdf  
10. https://umu.diva-portal.org/smash/get/diva2:1987951/FULLTEXT01.pdf  
11. https://docs.oasis-open.org/mqtt/mqtt/v5.0/mqtt-v5.0.html  
12. https://iot.telekom.com/de/blog/mqtt-protokoll-funktionsweise-anwendungen-und-vorteile-im-iot  
13. https://www.opc-router.de/was-ist-mqtt  
14. https://aws-amazon-com.translate.goog/what-is/mqtt  
15. https://www.all-electronics.de/automatisierung/basiswissen-mqtt-was-kann-das-iot-kommunikations-protokoll  
16. https://www.pubnub.com/blog/what-is-mqtt-use-cases  
17. https://www.ibm.com/docs/de/ibm-mq/9.3.x?topic=overview-mq-telemetry-transport-protocol
