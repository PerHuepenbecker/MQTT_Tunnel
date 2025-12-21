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
- [3. Systemdesign des MQTT-Tunnels](#3-systemdesign-des-mqtt-tunnels)
  - [3.1 Zielsetzung](#31-zielsetzung)
  - [3.2 Architekturübersicht](#32-architekturübersicht)
  - [3.3 Kommunikationsmodell](#33-kommunikationsmodell)
  - [3.4 Protokolldefinition und Datenformat](#34-protokolldefinition-und-datenformat)
  - [3.5 Einsatz eines TUN-Devices](#35-einsatz-eines-tun-devices)
  - [3.6 Sicherheitsbetrachtung](#36-sicherheitsbetrachtung)
- [4. Implementierung](#4-implementierung)
  - [4.1 Entwicklungsumgebung](#41-entwicklungsumgebung)
  - [4.2 Aufbau der Software](#42-aufbau-der-software)
  - [4.3 TUN-Device-Anbindung](#43-tun-device-anbindung)
  - [4.4 MQTT-Integration](#44-mqtt-integration)
  - [4.5 Programmablauf](#45-programmablauf)
- [5. Tests und Analyse](#5-tests-und-analyse)
  - [5.1 Funktionale Tests](#51-funktionale-tests)
  - [5.2 Performance und Latenz](#52-performance-und-latenz)
  - [5.3 tcpdump-Analyse](#53-tcpdump-analyse)
  - [5.4 Fehlerfälle](#54-fehlerfälle)
- [6. Fazit](#6-fazit)
- [Quellenverzeichnis](#quellenverzeichnis)

---

# Aufgabenstellung

Es gibt verschiedene Wege und Protokolle, über die sich ein Tunnel realisieren lässt. Allerdings ist es empfehlenswert, einen Tunnel über ein Protokoll zu wählen, das mit einer hohen Wahrscheinlichkeit in einem Netzwerk trotz Firewalling und vielleicht sogar irgendwelcher Proxies verfügbar ist. Das schränkt die Auswahl schon etwas ein.

Vielversprechend und entsprechend häufig genutzt sind:

- HTTP / HTTPS  
- DNS  

Die wollen wir in diesem Praktikum daher nicht nutzen, das wäre ja langweilig – stattdessen wollen wir einen „Exoten“ nutzen: Ein Protokoll, das einen Message-Broker nutzt, wie:

- MQTT  
- CORBA  

In Ihrer Gruppe wählen Sie eines dieser Protokolle oder ein weiteres broker-basiertes Protokoll aus, dokumentieren das „Warum“ (es gibt kein „richtig“ und „falsch“, aber Ihre Beweggründe sind interessant) und entwickeln, möglichst unter Zuhilfenahme vorhandener Tools / Frameworks / bekannter Lösungen eine Implementierung, um einen Tunnel zu ermöglichen.

Im Idealfall können Sie dann durch diesen Tunnel z. B. mit SSH, OpenVPN oder Wireguard alles weitere durchtunneln, alternativ beschränken Sie sich darauf, dass Ihr Tunnel HTTP(S)-Anfragen durchleitet. Was Sie anbieten, ist eine Design-Entscheidung. Auch dafür sollten Sie die Argumente wieder dokumentieren. Auch hier geht es um die Überlegungen, nicht um ein „richtig“ oder „falsch“.

Ihre Aufgaben für die Gruppenarbeit sind also:

1. Erörtern Sie, welches Protokoll Ihr Trägerprotokoll wird.  
2. Erörtern Sie, ob Sie allgemein alle TCP- oder UDP- oder IP-Verbindungen tunneln wollen oder nur HTTP(S).  
3. Implementieren Sie Ihren Tunnel. Dokumentieren Sie Implementationsentscheidungen.  

Hinweis: Dazu sollten Sie auch die Anforderungen und Lösungsideen gründlich analysieren, der Anspruch ist, dass die Beschreibung zwar abstrakt, aber konkret genug ist, dass auf Ihr Darstellung aufbauend jemand die Implementierung erfolgreich starten kann. Der Teufel liegt bekanntlich im Detail, das vielleicht auch erst bei Tests und der Umsetzung auffällt, das müssen Sie nicht entdecken. Die Ergebnisse sollten sich in Ihrem Paper wiederfinden.

4. Testen Sie Ihren Tunnel gründlich. Dazu gehört auch, den Tunnel mal mit tcpdump, Wireshark o. ä. aufzuzeichnen und zu prüfen, was ein Dritter sieht.  
5. Dokumentieren Sie in Ihrer Abgabe, welche Gruppenmitglieder an welchem Teil der Aufgabe mitgewirkt haben, wer für welchen Teil „den Hut auf hatte“ und welchen Anteil welches Gruppenmitglied an dem Teil hatte. Im Idealfall sollten im Ergebnis alle Gruppenmitglieder etwa gleich viel geleistet haben.

Die Aufgaben bearbeiten Sie in Gruppen von bis zu vier Personen. (Jedes Schild hat eine Geschichte, so auch der Hinweis, dass eine Personengruppe mindestens zwei Personen umfasst.)

Das Paper (also der gedanklich-wissenschaftliche Hintergrund) geben Sie als .pdf-Datei über Moodle ab, die Implementierung im (ggf. compilierbaren) Quellcode als .tar.gz.

Sie sind frei in der Wahl der Programmiersprache(n), Sie dürfen auch auf z. B. den Code von iodine oder anderen Frameworks aufsetzen, wenn das für Sie sinnvoll erscheint. Das sind Design-Entscheidungen, die Sie in Ihrem Paper diskutieren und erläutern sollten.

---

# Protokolle im Überblick

Um eine Entscheidungsgrundlage zu gewährleisten, werden im ersten Schritt die beiden vorgeschlagenen Protokolle mit Messagebrokercharakter in Kürze dargestellt. Dabei werden die wesentlichen Funktionsweisen, typische Anwendungsbereiche und markante Merkmale der jeweiligen Protokolle dargestellt.

Im weiteren Verlauf wird die letztendlich getroffene Entscheidung, welches der beiden eingangs genannten Protokolle für die weitere Bearbeitung der Aufgabenstellung Relevanz findet. Diese Entscheidung wird anhand der aufgezeigten Charakteristika begründet.

---

# Corba

CORBA (Common Object Request Broker Architecture) ist kein einzelnes Protokoll im herkömmlichen Sinne, sondern kann als Standard für eine Middlewarearchitektur, welche eine Kommunikation zwischen verteilten Softwarekomponenten ermöglicht, verstanden werden. Diese ist dabei unabhängig von der jeweilig gewählten Programmiersprache, Betriebssystem oder Hardwareplattform. Das spezifische Netzwerkprotokoll, das CORBA für die Interoperabilität verwendet, heißt Internet Inter-ORB Protocol (IIOP) und basiert dabei auf TCP/IP. Es ist eine objektorientierte Middleware für verteilte heterogene Systeme, die von der Object Management Group entworfen und spezifiziert wurde [1,2,3].

## Funktionsweise

Das Kernstück der CORBA-Architektur ist der Object Request Broker (ORB). Der ORB ist verantwortlich für die Vermittlung aller Anfragen und Antworten zwischen Clients (Nutzern eines Dienstes) und Servern (Anbietern eines Dienstes).

Der Prozess der Kommunikation funktioniert in mehreren Schritten:

1. **Schnittstellendefinition (IDL):**  
   Die Schnittstelle des Server-Objekts wird mithilfe der Interface Definition Language (IDL) getreu den Spezifikationen der Object Management Group festgelegt. Diese sind sprachneutral beschrieben und zeigen genau auf, welche Operationen das Objekt bereitstellt und welche Parameter benötigt werden.

2. **Generierung von Stubs und Skeletons:**  
   Aus der IDL-Definition werden sprachspezifische Codefragmente generiert:
   - **Stubs auf der Client-Seite:** Sie verbergen die Netzwerkkommunikation vor dem Client, sodass sich der Aufruf einer entfernten Methode wie der Aufruf einer lokalen Methode anfühlt.  
   - **Skeletons auf der Server-Seite:** Sie nehmen die Anfragen entgegen, entpacken die Daten und rufen die tatsächliche Implementierung der Server-Logik auf.

3. **Objektreferenz und Naming Service:**  
   Der Server erstellt das Objekt und registriert es bei einem Naming Service. Der Client kann dann über diesen Dienst eine eindeutige Objektreferenz (ähnlich einer Adresse) für das benötigte Objekt erhalten.

4. **Der ORB als Vermittler:**  
   Wenn der Client eine Methode aufruft (über den Stub), fängt der lokale ORB diese Anfrage ab. Er verwendet die Objektreferenz, um das entfernte Server-Objekt im Netzwerk zu lokalisieren.

5. **Datenübertragung mit IIOP:**  
   Die Anfrage (einschließlich Methodennamen und Parameter) wird vom Client-ORB serialisiert (gemarshallt) und über das Netzwerk mithilfe des IIOP an den Server-ORB gesendet. IIOP verwendet hierfür TCP/IP.

6. **Ausführung und Antwort:**  
   Der Server-ORB empfängt die Daten, leitet sie an das Skeleton weiter, das die tatsächliche Methode auf dem Server-Objekt ausführt. Die Antwort wird dann auf dem gleichen Weg zurück an den Client gesendet.

Zusammenfassend ermöglicht CORBA die Transparenz der Lokalität, sodass Entwickler verteilte Anwendungen schreiben können und dabei die Annahme treffen können, dass sich alle Objekte verhalten, als ob sie sich im selben Speicherraum befinden. [1,2,3]

## Typische Einsatzgebiete

Das CORBA-Protokoll wird heutzutage vor allem in spezifischen Bereichen und für die Integration von älteren Systemen (Altsystemen) genutzt. Obwohl es in neuen Softwareentwicklungen oft durch modernere Alternativen wie Webservices oder Microservices ersetzt wird, findet es in folgenden Bereichen weiterhin Anwendung:

- **Altsysteme (Legacy-Systeme):** Integration bestehender, älterer Software.
- **Echtzeit- und eingebettete Systeme:** z. B. Luft- und Raumfahrt.
- **Große Unternehmensinfrastrukturen:** z. B. Finanz- und Versicherungssektor.
- **Industrielle Automatisierung:** Verbindung heterogener Maschinen und Systeme.

Da die Echtzeitfähigkeit für die Aufgabenstellung keine Relevanz darstellt, wird an dieser Stelle nicht weiter auf RT-CORBA eingegangen.

## Typische Paketgröße

Es gibt keine feste „typische“ Größe für CORBA-Datenpakete. Die Paketgröße hängt vor allem ab von:

- Art und Größe der übergebenen Datenstrukturen  
- Fragmentierung durch Netzwerk-MTU (typisch 1500 Bytes)  
- ORB-spezifischer Fragmentierungslogik  
- Header-Overhead (IIOP-Header + TCP/IP)  

Kleine Pakete können nur wenige Dutzend Bytes groß sein, während große Datenübertragungen mehrere Megabyte umfassen können. [1,2,10]

---

# MQTT

MQTT (Message Queuing Telemetry Transport) ist ein schlankes, offenes Nachrichtenprotokoll, das speziell für die Machine-to-Machine (M2M)-Kommunikation im Internet of Things (IoT) entwickelt wurde. Es ist darauf ausgelegt, Daten effizient in Umgebungen mit begrenzter Bandbreite, hoher Latenz oder unzuverlässigen Netzwerkverbindungen zu übertragen. [11,12]

## Funktionsweise

MQTT basiert auf dem **Publish/Subscribe-Modell**. Die Architektur umfasst:

- **Publisher** – sendet Nachrichten zu einem Topic  
- **Subscriber** – empfängt Nachrichten zu abonnierten Topics  
- **Broker** – zentrale Vermittlungsinstanz  

Sender und Empfänger sind vollständig entkoppelt. [11,13]

## Typische Einsatzgebiete

MQTT ist einer der wichtigsten Standards in:

- IoT  
- Smart Home  
- industrieller Automatisierung  
- Telemetrie und Sensornetzwerken  
- Logistik  
- Wearables  
- Cloud-Plattformen  

Gründe für seine Beliebtheit:

- geringer Overhead  
- effiziente Architektur  
- Zuverlässigkeit durch QoS-Stufen

## Typische Paketgröße

Die MQTT-Paketgröße wird bestimmt durch:

- Fixed Header (immer 2 Bytes)  
- Variable Header (topicabhängig)  
- Payload (0 Bytes bis mehrere MB)  

Typischerweise sind MQTT-Pakete sehr klein (Dutzende Bytes). Maximale Größe: 256 MB. Fragmentierung erfolgt über IP-MTU. [11]

---

# Entscheidung

Auf Grund der dargestellten Informationslage wurde sich innerhalb der Gruppenarbeit auf die Nutzung des MQTT-Protokolls geeinigt.

Die Begründung liegt dabei hauptsächlich in dem breiteren Anwendungsspektrum gepaart mit dem gleichzeitig effizient gestalteten Nachrichtenaufbau. Zeitgleich dient die prominente Verbreitung des MQTT-Protokolls innerhalb der IoT-Thematik vermehrte Berührungspunkte im alltäglichen Leben.  

Der stetig steigende Trend beim Einsatz sogenannter Smarthome-Komponenten in Verbindung mit den verschiedensten Steuerungszentralen, welche zum Beispiel auf OpenSource-Plattformen wie Home-Assistant oder IO-Broker basieren, birgt weiteres Potential zur gesteigerten Verbreitung des MQTT-Protokolls und damit größeren Einfluss im privaten Bereich.

# Segmentierung
Trotz der enormen Flexibilität im Bezug auf die Nachrichtengröße innerhalb der Spezifikationen des MQTT-Protokolls empfiehlt es sich beim Tunneln von großen Datenmengen diese in kleinere Pakete aufzuteilen. [18] Diese sogenannte Segmentierung bietet mehrere Vor- aber auch Nachteile.

# Vorteile der Segmentierung
Ressourcenbeschränkung
MQTT dient oftmals als Kommunikationsprotokoll für Sensoren und Aktuatoren. Diese Geräte sind meistens mit ressourcenbeschränkten Komponenten wie Mikrocontrollern ausgestattet, die auf Grund mangelnden Zwischenspeichers mit großen Nachrichten nur bedingt umgehen können. Ein Überschreiten der hardwarebedingten Grenzen kann zu Verbindungsabbrüchen oder dem Verwerfen der Nachricht durch den Broker führen. Darüber hinaus kann eine Überlastung des Speichers dazu führen, dass die jeweiligen Komponenten zu viel Zeit für das Verarbeiten der Nachricht benötigen, wodurch das Abarbeiten der ordinär zugeordneten Hauptaufgabe beeinträchtigt oder zumindest verzögert erfolgt.
Netzwerkstabilität
Große, einzelne Übertragungen sind anfälliger für Netzwerkinstabilitäten. Angenommen es tritt während der Übertragung einer großen Nachricht ein Verbindungsabbruch auf. In diesem Falle müsste die gesamte Nachricht erneut versandt werden, da MQTT keinen Mechanismus zur Fortsetzung unterbrochener Datenströme bietet. Im Falle von kleineren Segmente, muss lediglich der fehlerhaft übertragene Datenframe erneut gesendet werden.

# Nachteile Segmentierung
Die Nachteile der Datensegmentierung umfassen eine Reihe von Herausforderungen, die von hohen Kosten und größerem Aufwand in der Implementierung bis hin zu mangelhafter Datenqualität reichen.
Kosten und Ressourcenbedarf
Der Begriff Kosten wird im weiteren mit der Belastung der Kapazität des Übertragungskanals gleichgesetzt. Erhöhte Kosten zeichnen sich für den Endanwender durch eine verminderte Übertragungsgeschwindigkeit relevanter Daten ab.
Mit jedem Datensegment, welches versendet wird, müssen zusätzliche Informationen an das versandte Element angehängt werden. Diese bieten für den Nutzer keinen Mehrwert. Ein sogenannter Overhead entsteht, welcher für die erhöhten Kosten verantwortlich ist.
Dieser Overhead enthält unter anderem Informationen zur originalen Nachricht, Segmentnummer oder Details zu nachfolgenden Paketen, wie z.B. letztes Segment der Ursprungsnachricht.
Wird die originale Nachricht zu fein unterteilt, steigt die Anzahl der Segmente und damit die Menge des damit verbundenen Overheads proportional. Allgemein wird dieses Phänomen als Übersegmentierung bezeichnet.
Ist die Segmentierung zu grob, treten möglicherweise die gewünschten Effekte nicht auf und es wird lediglich ein erhöhter Datenstrom verzeichnet.
Planung und Umsetzung
Wie bereits erwähnt sind sowohl die zu grobe, wie auch eine zu feine Segmentierung nicht wünschenswert. Demnach kann der hinterlegte Aufteilungsprozess sehr komplex sein und erfordert sorgfältige Planung.
Datenqualität
Wie bereits im Vorfeld erwähnt, bietet MQTT keinen eigenen Mechanismus zur Fortsetzung unterbrochener Datenströme. Im Falle von fehlerhaften Segmenten können demnach vereinzelte Nachrichteninhalte verloren gehen. Wird dies nicht von den entsprechenden Anwendungen erkannt oder gar korrigiert, endet die Nachrichtenübertragung in einer verminderten Datenqualität. 

---
# 3. Systemdesign des MQTT-Tunnels

## 3.1 Zielsetzung

Ziel dieses Projekts ist die Entwicklung und Analyse eines Netzwerktunnels, der TCP-basierte Kommunikation über ein Message-Broker-basiertes Protokoll transportiert. Als Trägerprotokoll wird MQTT eingesetzt.

Der Tunnel soll es ermöglichen, TCP-Verbindungen transparent über MQTT zu kapseln, sodass für einen externen Beobachter ausschließlich MQTT-Kommunikation zwischen Client und Broker sichtbar ist. Die eigentlichen TCP-Verbindungen sollen dabei auf Netzwerkebene verborgen bleiben.

Der Schwerpunkt der Arbeit liegt auf der technischen Umsetzung des Tunnels, den zugrunde liegenden Architekturentscheidungen sowie der Analyse der Sichtbarkeit des resultierenden Datenverkehrs mittels Netzwerkanalysewerkzeugen wie tcpdump und Wireshark.


## 3.2 Architekturübersicht

Der Kernel behandelt die Datenpakete der TUN-Device genauso, als würden diese von einem echten physischen Gerät abstammen. Sämtliche netzwerkrelevanten Funktionen wie Routing, Fragmentierung oder die Verarbeitung von TCP- und UDP-Daten erfolgen dabei im regulären Netzwerk-Stack des Betriebssystems. Die TUN-Device selbst dient ausschließlich als virtuelle Ein- und Ausgabeschnittstelle für IP-Pakete, welche von der Tunnelanwendung verarbeitet werden. Aus diesem Grund kann an dieser Stelle auf weitere Details zu diesem Thema verzichtet werden.


## 3.3 Kommunikationsmodell

Für die bidirektionale Kommunikation zwischen Tunnel-Client und Tunnel-Server werden bewusst getrennte MQTT-Topics für Steuer- und Nutzdaten verwendet.

Diese Entscheidung wurde aus folgenden Gründen getroffen:

- sie verhindert zuverlässig, dass ein Client seine eigenen Nachrichten wieder empfängt  
- viele verbreitete MQTT-Broker unterstützen das MQTT-5-Flag `no_local` nicht oder verarbeiten es inkonsistent  
- dadurch ist die Lösung auch mit **MQTT 3.1.1** vollständig kompatibel  
- das Verhalten bleibt deterministisch und unabhängig vom Broker  
- die Implementierung wird robuster gegenüber unterschiedlichen Broker-Implementierungen (Mosquitto, EMQX, HiveMQ, VerneMQ usw.)

Durch diese klare Trennung der Datenrichtungen wird ein stabiler und vorhersagbarer Tunnelbetrieb gewährleistet.

In der implementierten Testumgebung erfolgt der initiale Sitzungsaufbau über einen zentralen Command-Channel. Nach erfolgreichem Handshake werden sitzungsspezifische Topics verwendet, die anhand der Client-ID eindeutig zugeordnet sind. Für jede Sitzung existieren dabei zwei separate Topics, welche die jeweilige Kommunikationsrichtung zwischen Client und Server abbilden.

## 3.4 Protokolldefinition und Datenformat

Der Tunnel kapselt Netzwerkverkehr, indem er die über das TUN-Interface anfallenden IP-Pakete (Layer 3) als Nutzlast in MQTT-Nachrichten überträgt. Dadurch wird aus Sicht der Anwendungen eine normale IP-Verbindung bereitgestellt, während der Transport zwischen den Tunnelendpunkten über den MQTT-Broker erfolgt.

Die Kommunikation ist logisch in zwei Phasen aufgeteilt:

Handshake / Sitzungsaufbau (Command-Channel)
Zu Beginn wird eine Sitzung zwischen Client und Server aufgebaut. Dabei werden Parameter wie Client-ID, die zu verwendenden Topics sowie die Tunnel-IP-Konfiguration abgestimmt. Der Server verwaltet anschließend eine Zuordnung „Client-ID ↔ Tunnel-IP“ und akzeptiert Datenpakete nur für aktive Sitzungen.

Datentransfer (Data-Channel)
Nach erfolgreichem Handshake werden die rohen IP-Pakete als Payload über MQTT ausgetauscht. In der implementierten Testumgebung erfolgt dies über sitzungsspezifische Topics, z. B.:

mqtt_tunnel/commands/<client-id>/A (Richtung Server → Client)

mqtt_tunnel/commands/<client-id>/B (Richtung Client → Server)

Das Datenformat der Payload ist dabei bewusst schlank gehalten: Es werden keine zusätzlichen Protokollheader wie Sequenznummern oder Checksummen auf Anwendungsebene eingeführt. Die Integrität und Reihenfolge werden in der Praxis primär durch die darunterliegenden Protokollmechanismen (TCP-Verkehr innerhalb der IP-Pakete sowie MQTT-Transport) beeinflusst.

## 3.5 Einsatz eines TUN-Devices

Für die Tunnelumsetzung wird auf beiden Endpunkten ein TUN-Device verwendet. Ein TUN-Device stellt eine virtuelle Netzwerkschnittstelle auf Layer 3 bereit und liefert bzw. akzeptiert komplette IP-Pakete. Für den Linux-Kernel wirkt diese Schnittstelle wie ein normales Netzwerkinterface: Pakete können über Routing-Regeln an das TUN-Interface gesendet werden und erscheinen dort zur Verarbeitung durch die Tunnelanwendung.

Der Ablauf ist dabei grundsätzlich:

Ausgehender Verkehr:
Ein Programm (z. B. nc/SSH) erzeugt TCP-Verkehr, der vom System als IP-Pakete geroutet wird. Statt über eine physische Netzwerkkarte werden diese Pakete über das konfigurierte Routing an tun0 übergeben. Die Tunnelanwendung liest die Pakete aus tun0 und sendet sie als MQTT-Payload an den Broker.

Eingehender Verkehr:
Die Tunnelanwendung empfängt MQTT-Nachrichten, entnimmt die Payload (IP-Paket) und schreibt diese in tun0. Der Kernel verarbeitet die Pakete anschließend wie regulären Netzwerkverkehr, sodass die lokale Anwendung die Antworten transparent erhält.

Der Einsatz eines TUN-Devices ermöglicht es, beliebigen IP-basierten Verkehr durch den Tunnel zu leiten, ohne dass Anwendungen speziell angepasst werden müssen. Gleichzeitig wird vermieden, Ethernet-Frames (Layer 2) nachbilden zu müssen, was die Implementierung deutlich vereinfacht.

## 3.6 Sicherheitsbetrachtung

Bei dem Protokoll MQTT liegt das Hauptaugenmerk auf eine ressourcenschonende Option zum Informationsaustausch zwischen einzelnen IoT-Geräten. Selbst auf der offiziellen Webseite von MQTT wird das Thema Sicherheit sehr vorsichtig formuliert.

You can pass a user name and password with an MQTT packet in V3.1 of the protocol. Encryption across the network can be handled with SSL, independently of the MQTT protocol itself (it is worth noting that SSL is not the lightest of protocols, and does add significant network overhead). Additional security can be added by an application encrypting data that it sends and receives, but this is not something built-in to the protocol, in order to keep it simple and lightweight. [Originaler Auszug https://mqtt.org/faq/ unter dem Punkt "Does MQTT support security?" 08.12.2025]

Es wird prinzipiell die Möglichkeit zur Einbindung einer SSL-/TLS-Verschlüsslung ermöglicht, jedoch muss diese vom Nutzer erst aktiviert werden. Zeitgleich wird darauf hingewiesen, dass der Einsatz dieser Verschlüsselung entgegen des Hauptvorteils von MQTT wirkt und somit der Einsatz mit bedacht gewählt werden soll.

---

# 4. Implementierung

## 4.1 Entwicklungsumgebung

Die Implementierung und Erprobung des MQTT-basierten Tunnels erfolgte in einer virtualisierten Testumgebung auf Basis von Oracle VirtualBox. Hierzu wurden zwei virtuelle Maschinen mit Debian Linux eingesetzt, die identisch konfiguriert wurden.

Die virtuellen Maschinen wurden in VirtualBox unter den Namen **mqtt-vm-a** und **mqtt-vm-b** angelegt und betrieben.

Beide virtuellen Maschinen verfügen über zwei Netzwerkschnittstellen. Eine Schnittstelle ist als NAT-Interface ausgeführt und ermöglicht den Zugriff auf externe Netzwerke. Die zweite Schnittstelle ist als Host-Only-Netzwerk konfiguriert und dient der direkten Kommunikation zwischen den virtuellen Maschinen sowie dem Hostsystem.

Die IP-Adressierung im Host-Only-Netzwerk ist statisch konfiguriert:

- **mqtt-vm-a:** 192.168.56.101  
- **mqtt-vm-b:** 192.168.56.102  

Der administrative Zugriff auf die Systeme erfolgt über SSH.

Für die Durchführung der Tests wurden folgende Benutzerkonten verwendet:

- **mqtt-vm-a:** Benutzer **usera**
- **mqtt-vm-b:** Benutzer **userb**

## Einrichtung der Entwicklungsumgebung (Debian)

```bash
# Schritt 1: Grundlegende Build- und Systemwerkzeuge
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  git \
  pkg-config \
  libssl-dev \
  libgtest-dev \
  libcereal-dev \
  libspdlog-dev

# Schritt 2: Google Test kompilieren und installieren
cd /usr/src/googletest
sudo cmake -S . -B build
sudo cmake --build build -j$(nproc)
sudo cmake --install build
sudo ldconfig

# Schritt 3: Eclipse Paho MQTT (C-Bibliothek) installieren
cd ~
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
cmake -S . -B build -DPAHO_WITH_SSL=ON -DPAHO_BUILD_SHARED=ON
cmake --build build -j$(nproc)
sudo cmake --install build
sudo ldconfig

# Schritt 4: Eclipse Paho MQTT (C++-Wrapper) installieren
cd ~
git clone https://github.com/eclipse/paho.mqtt.cpp.git
cd paho.mqtt.cpp
cmake -S . -B build
cmake --build build -j$(nproc)
sudo cmake --install build
sudo ldconfig

# Schritt 5: Projekt kompilieren
cd ~/MQTT_Tunnel
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

## 4.2 Aufbau der Software

## 4.3 TUN-Device-Anbindung

## 4.4 MQTT-Integration

## 4.5 Programmablauf

---

# 5. Tests und Analyse

## 5.1 Funktionale Tests

Zur Überprüfung der grundlegenden Funktionsfähigkeit des MQTT-Tunnels wurden mehrere funktionale Tests durchgeführt. Ziel dieser Tests war es, nachzuweisen, dass TCP-basierte Kommunikation erfolgreich über den MQTT-basierten Tunnel übertragen werden kann und für die beteiligten Anwendungen transparent funktioniert.

### Aufbau des Testszenarios

Der Tunnel-Server wurde auf der virtuellen Maschine **mqtt-vm-a** gestartet, der Tunnel-Client auf **mqtt-vm-b**. Nach erfolgreichem Verbindungsaufbau waren auf beiden Systemen jeweils ein TUN-Interface (`tun0`) mit folgenden IP-Adressen aktiv:

- Tunnel-Server (mqtt-vm-a): `10.0.0.1/24`
- Tunnel-Client (mqtt-vm-b): `10.0.0.2/24`

Die Verbindung wurde zuvor über den MQTT-Broker erfolgreich etabliert, einschließlich des vollständigen Handshakes zwischen Client und Server.

### Test 1: Erreichbarkeit über den Tunnel (Ping)

Als erster Funktionstest wurde die grundlegende Erreichbarkeit über den Tunnel überprüft. Dazu wurde vom Tunnel-Client ein ICMP-Ping an die Tunnel-IP des Servers gesendet:

```bash
ping 10.0.0.1
```
Die ICMP-Pakete wurden erfolgreich übertragen und beantwortet, was bestätigt, dass IP-Verkehr korrekt über das TUN-Interface gelesen, über MQTT transportiert und auf der Gegenseite wieder in das Netzwerk eingespeist wurde.

### Test 2: TCP-Kommunikation über den Tunnel

Im nächsten Schritt wurde überprüft, ob auch TCP-basierte Kommunikation zuverlässig über den Tunnel funktioniert.

Auf dem Tunnel-Server wurde ein einfacher TCP-Listener mit `netcat` gestartet:

```bash
nc -l -p 12345
```
Anschließend wurde vom Tunnel-Client aus eine TCP-Verbindung über die Tunnel-IP aufgebaut und eine Testnachricht gesendet:

```bash
printf '%s\n' 'TESTNACHRICHT' | nc 10.0.0.1 12345
```
Die Testnachricht wurde auf der Serverseite korrekt empfangen und angezeigt. Damit konnte nachgewiesen werden, dass TCP-Verbindungen vollständig über den MQTT-Tunnel übertragen werden können, ohne dass die beteiligten Anwendungen angepasst werden müssen.

### Ergebnis der funktionalen Tests

Die durchgeführten Tests zeigen, dass der implementierte MQTT-Tunnel in der Lage ist, sowohl ICMP- als auch TCP-Verkehr zuverlässig zu transportieren. Für die Anwendungen erscheint die Verbindung wie eine normale IP-basierte Punkt-zu-Punkt-Verbindung, während der eigentliche Datentransport vollständig über MQTT erfolgt.

Damit ist die grundlegende Funktionalität des Tunnels nachgewiesen.


## 5.2 Performance und Latenz

## 5.3 tcpdump-Analyse

Zur Analyse der Sichtbarkeit des MQTT-Tunnels aus Sicht eines externen Beobachters wurden Netzwerkaufzeichnungen mit `tcpdump` durchgeführt. Ziel war es, zu untersuchen, welche Art von Netzwerkverkehr auf den beteiligten Schnittstellen sichtbar ist und ob der transportierte TCP-Verkehr als solcher erkennbar bleibt.

### Analyse auf dem TUN-Interface

Zunächst wurde der Netzwerkverkehr direkt auf dem TUN-Interface (`tun0`) des Tunnel-Servers aufgezeichnet. Auf dieser Schnittstelle sind die durch den Tunnel transportierten IP-Pakete sichtbar, da sie hier vom Kernel an die Tunnelanwendung übergeben bzw. von dieser wieder eingespeist werden.

Die Aufzeichnung zeigt unter anderem den vollständigen TCP-Verbindungsaufbau (SYN, SYN-ACK, ACK) sowie die Übertragung der Nutzdaten. Die Testnachricht „TESTNACHRICHT“ wird dabei als Teil eines regulären TCP-Datenpakets sichtbar. Dies bestätigt, dass innerhalb des Tunnels echter TCP-Verkehr transportiert wird und der Tunnel aus Sicht des Betriebssystems wie eine normale IP-Verbindung funktioniert.

### Analyse auf der physischen Netzwerkschnittstelle (ohne MQTT)

Anschließend wurde der Netzwerkverkehr auf der physischen Netzwerkschnittstelle (`enp0s8`) des Tunnel-Servers aufgezeichnet, wobei MQTT-Verkehr explizit ausgeschlossen wurde (`not port 1883`). In dieser Aufzeichnung waren keine Pakete sichtbar, die auf eine direkte TCP-Verbindung zwischen Tunnel-Client und Tunnel-Server hindeuten.

Insbesondere war weder die Testnachricht noch ein TCP-Handshake in Richtung der Tunnel-IP-Adressen (`10.0.0.0/24`) erkennbar. Für einen Beobachter auf dieser Schnittstelle ist somit kein Hinweis auf die tatsächlich durch den Tunnel transportierte TCP-Kommunikation vorhanden.

### Analyse des MQTT-Verkehrs

In einem weiteren Schritt wurde gezielt der MQTT-Verkehr auf Port 1883 aufgezeichnet. In dieser Aufzeichnung sind ausschließlich TCP-Verbindungen zwischen Tunnel-Client, Tunnel-Server und dem MQTT-Broker sichtbar. Die übertragenen Daten erscheinen dabei als regulärer MQTT-Datenverkehr mit Payloads unterschiedlicher Länge.

Der Inhalt der MQTT-Nachrichten lässt ohne zusätzliche Kontextinformationen keinen direkten Rückschluss auf den darin gekapselten TCP-Verkehr zu. Insbesondere sind weder Zieladressen noch Portnummern der eigentlichen TCP-Verbindungen auf Netzwerkebene unmittelbar erkennbar.

### Ergebnis der Analyse

Die Analyse zeigt deutlich, dass der eigentliche TCP-Verkehr ausschließlich auf dem TUN-Interface sichtbar ist. Auf der physischen Netzwerkschnittstelle ist für einen externen Beobachter lediglich MQTT-Kommunikation zwischen den beteiligten Systemen und dem Broker erkennbar.

Damit erfüllt der Tunnel das Ziel, TCP-Verkehr effektiv in MQTT zu kapseln und auf Netzwerkebene zu verbergen. Ein Dritter kann zwar feststellen, dass MQTT genutzt wird, jedoch nicht ohne weiteres erkennen, welche Art von Anwendungen oder Protokollen innerhalb des Tunnels transportiert werden.

## 5.4 Fehlerfälle

Neben den erfolgreichen Funktionstests wurden auch verschiedene Fehlerfälle betrachtet, um das Verhalten des MQTT-Tunnels bei nicht idealen Bedingungen zu analysieren. Ziel war es zu überprüfen, wie sich der Tunnel bei fehlender Verbindung, nicht verfügbaren Diensten oder unterbrochenem Tunnelbetrieb verhält.

### Fehlerfall 1: TCP-Verbindung ohne aktiven Tunnel

Wird versucht, vom Tunnel-Client aus eine TCP-Verbindung zur Tunnel-IP des Servers aufzubauen, ohne dass der Tunnel-Server aktiv ist, schlägt der Verbindungsaufbau fehl. Der Client erhält in diesem Fall eine Timeout- oder Verbindungsfehler-Meldung, da keine Gegenstelle erreichbar ist.

Dieses Verhalten entspricht dem einer normalen IP-Verbindung ohne erreichbaren Zielhost und zeigt, dass der Tunnel keine unerwarteten Fallback-Mechanismen verwendet.

### Fehlerfall 2: Ping bei nicht aktivem Tunnel

Ein ICMP-Ping an die Tunnel-IP-Adresse des Servers (`10.0.0.1`) führt bei nicht aktivem Tunnel zu einem vollständigen Paketverlust. Es werden keine ICMP-Antworten empfangen.

Auch dieses Verhalten entspricht dem erwarteten Verhalten einer nicht vorhandenen oder unterbrochenen Netzwerkverbindung und bestätigt, dass der Tunnelverkehr ausschließlich über den aktiven MQTT-Tunnel abgewickelt wird.

### Fehlerfall 3: Unterbrochene TCP-Sitzung

Wird eine bestehende TCP-Verbindung über den Tunnel unterbrochen, beispielsweise durch das Beenden des TCP-Servers oder des Tunnel-Servers, schlägt ein erneuter Verbindungsversuch fehl. Der Client erhält in diesem Fall eine entsprechende Fehlermeldung (z. B. „Connection refused“).

Der Tunnel verhält sich hierbei transparent: Die Fehlermeldungen stammen aus dem regulären TCP/IP-Stack des Betriebssystems und werden unverändert an die Anwendung weitergereicht.

### Zusammenfassung der Fehlerfälle

Die untersuchten Fehlerfälle zeigen, dass sich der MQTT-Tunnel bei Störungen oder Fehlkonfigurationen wie eine reguläre IP-basierte Verbindung verhält. Fehler werden nicht verborgen oder abgefangen, sondern korrekt an die jeweiligen Anwendungen weitergegeben.

Dies erleichtert sowohl die Fehlersuche als auch die Bewertung des Tunnelverhaltens und trägt zu einem transparenten und nachvollziehbaren Systemverhalten bei.


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
18. http://www.bbs-1.de/bbs1/umat/netze/netz8.html
19. https://www.kernel.org/doc/html/latest/networking/tuntap.html

# Anhang

## Anhang A: Terminalausgaben – Tunnelbetrieb

### A1: Start des MQTT-Tunnel-Servers (mqtt-vm-a)
```bash
usera@mqtt-vm-a:~$ cd ~/MQTT_Tunnel/build
sudo ./mqtt_tunnel \
  --mode server \
  --broker 192.168.56.101 \
  --client-id tunnel-server-a
[sudo] password for usera:
[2025-12-21 18:51:04.937] [info] Creating TUN device: tun0
[2025-12-21 18:51:04.942] [info] TUN device tun0 created
[2025-12-21 18:51:04.979] [info] TUN device configured with IP: 10.0.0.1
[2025-12-21 18:51:04.979] [info] Server: Connecting to command channel...
[2025-12-21 18:51:05.000] [info] Server: Connected! Subscribing to topic: mqtt_tunnel/commands_RX
[2025-12-21 18:51:05.003] [info] Server: Subscribed successfully
[2025-12-21 18:51:05.003] [info] Command channel connected
Data channel connected
[2025-12-21 18:51:06.005] [info] Tunnel server started
[2025-12-21 18:51:06.006] [warning] No active session for destination IP: 96.164.82.55
[2025-12-21 18:51:09.556] [warning] No active session for destination IP: 96.164.82.55
[2025-12-21 18:51:12.616] [info] Consumed message from command channel
[2025-12-21 18:51:12.617] [info] Handling client handshake message
[2025-12-21 18:51:12.617] [info] Received Client Hello from client ID: tunnel-client-b
[2025-12-21 18:51:12.660] [info] Sent Server Hello to client ID: tunnel-client-b
[2025-12-21 18:51:12.709] [info] Received Client ACK from client ID: tunnel-client-b
[2025-12-21 18:51:12.752] [info] Sent Server ACK to client ID: tunnel-client-b
[2025-12-21 18:51:12.767] [info] Session established for client ID: tunnel-client-b with IP: 10.0.0.2
[2025-12-21 18:51:14.858] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:51:14.858] [info] Wrote 48 bytes to TUN device
[2025-12-21 18:51:17.270] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:51:17.270] [info] Wrote 48 bytes to TUN device
[2025-12-21 18:51:18.772] [warning] No active session for destination IP: 96.164.82.55
[2025-12-21 18:51:26.230] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:51:26.231] [info] Wrote 48 bytes to TUN device
[2025-12-21 18:51:37.717] [warning] No active session for destination IP: 96.164.82.55
[2025-12-21 18:51:44.674] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:51:44.675] [info] Wrote 48 bytes to TUN device
[2025-12-21 18:51:56.810] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:51:56.811] [info] Read 60 bytes from TUN and publishing to topic mqtt_tunnel/commands/tunnel-client-b/A
[2025-12-21 18:51:56.812] [info] Wrote 60 bytes to TUN device
[2025-12-21 18:51:56.860] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:51:56.862] [info] Wrote 52 bytes to TUN device
[2025-12-21 18:51:56.863] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:51:56.864] [info] Read 52 bytes from TUN and publishing to topic mqtt_tunnel/commands/tunnel-client-b/A
[2025-12-21 18:51:56.864] [info] Wrote 66 bytes to TUN device
[2025-12-21 18:52:12.532] [warning] No active session for destination IP: 96.164.82.55
[2025-12-21 18:52:21.527] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:52:21.527] [info] Wrote 48 bytes to TUN device
[2025-12-21 18:53:07.317] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:53:07.318] [info] Wrote 52 bytes to TUN device
[2025-12-21 18:53:07.319] [info] Read 52 bytes from TUN and publishing to topic mqtt_tunnel/commands/tunnel-client-b/A
[2025-12-21 18:53:07.364] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:53:07.365] [info] Wrote 52 bytes to TUN device
[2025-12-21 18:53:08.279] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:53:08.280] [info] Read 40 bytes from TUN and publishing to topic mqtt_tunnel/commands/tunnel-client-b/A
[2025-12-21 18:53:08.281] [info] Wrote 60 bytes to TUN device
[2025-12-21 18:53:24.212] [warning] No active session for destination IP: 96.164.82.55
[2025-12-21 18:53:39.353] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:53:39.353] [info] Wrote 48 bytes to TUN device
````

### B1: Start des MQTT-Tunnel-Clients (mqtt-vm-b)
```bash
userb@mqtt-vm-b:~$ cd ~/MQTT_Tunnel/build
sudo ./mqtt_tunnel \
  --mode client \
  --broker 192.168.56.101 \
  --client-id tunnel-client-b
[sudo] password for userb:
[2025-12-21 18:51:12.413] [info] Creating TUN device: tun0
[2025-12-21 18:51:12.418] [info] TUN device tun0 created
[2025-12-21 18:51:12.419] [info] Connecting to command channel...
[2025-12-21 18:51:12.532] [info] Connected! Now subscribing to topic: mqtt_tunnel/commands_TX
[2025-12-21 18:51:12.534] [info] Subscribed successfully
[2025-12-21 18:51:12.535] [info] Command channel connected
[2025-12-21 18:51:12.537] [info] Sent Client Hello, waiting for Server Hello...
[2025-12-21 18:51:12.667] [info] Received Server Hello
[2025-12-21 18:51:12.668] [info] Session configured with Client ID: tunnel-client-b, IP: 10.0.0.2, ServerIP: 10.0.0.1 Inbound Topic: mqtt_tunnel/commands/tunnel-client-b/A, Outbound Topic: mqtt_tunnel/commands/tunnel-client-b/B, Session ID: 4a7a39dff2e420e2e52a6c252f30ae7562bec4ac5fec44f4f3749586275a3d38
[2025-12-21 18:51:12.758] [info] Received Server ACK, session handshake complete
[2025-12-21 18:51:12.759] [info] Configuring TUN device with IP and routes...
[2025-12-21 18:51:12.759] [info] Executing command: ip addr add 10.0.0.2/24 dev tun0
[2025-12-21 18:51:12.760] [info] Executing command: ip route add 10.0.0.1 dev tun0
[2025-12-21 18:51:12.813] [info] TUN device configured with IP: 10.0.0.2
[2025-12-21 18:51:12.821] [info] Route to server address 10.0.0.1 added
[2025-12-21 18:51:14.862] [info] Data channel connected
[2025-12-21 18:51:14.863] [info] Tunnel client started
[2025-12-21 18:51:14.864] [info] Read 48 bytes from TUN and published to topic mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:51:17.277] [info] Read 48 bytes from TUN and published to topic mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:51:26.236] [info] Read 48 bytes from TUN and published to topic mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:51:44.675] [info] Read 48 bytes from TUN and published to topic mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:51:56.791] [info] Read 60 bytes from TUN and published to topic mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:51:56.820] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/A
[2025-12-21 18:51:56.820] [info] Wrote 60 bytes to TUN device
[2025-12-21 18:51:56.866] [info] Read 52 bytes from TUN and published to topic mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:51:56.867] [info] Read 66 bytes from TUN and published to topic mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:51:56.910] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/A
[2025-12-21 18:51:56.910] [info] Wrote 52 bytes to TUN device
[2025-12-21 18:52:21.533] [info] Read 48 bytes from TUN and published to topic mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:53:07.323] [info] Read 52 bytes from TUN and published to topic mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:53:07.327] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/A
[2025-12-21 18:53:07.328] [info] Wrote 52 bytes to TUN device
[2025-12-21 18:53:07.371] [info] Read 52 bytes from TUN and published to topic mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:53:08.285] [info] Read 60 bytes from TUN and published to topic mqtt_tunnel/commands/tunnel-client-b/B
[2025-12-21 18:53:08.288] [info] Message arrived with topic: mqtt_tunnel/commands/tunnel-client-b/A
[2025-12-21 18:53:08.288] [info] Wrote 40 bytes to TUN device
[2025-12-21 18:53:39.361] [info] Read 48 bytes from TUN and published to topic mqtt_tunnel/commands/tunnel-client-b/B
```

### A2: TCP-Listener mit netcat (mqtt-vm-a)
```bash
Last login: Sun Dec 21 18:32:46 2025 from 192.168.56.1
usera@mqtt-vm-a:~$ nc -l -p 12345
TESTNACHRICHT
usera@mqtt-vm-a:~$
````

### B2: TCP-Client mit netcat (mqtt-vm-b)
```bash
userb@mqtt-vm-b:~$ printf '%s\n' 'TESTNACHRICHT' | nc 10.0.0.1 12345
```

### A3: tcpdump auf dem Host-Only-Interface – ohne MQTT (mqtt-vm-a, Interface: enp0s8, Filter: not port 1883)
```bash
usera@mqtt-vm-a:~$ sudo tcpdump -i enp0s8 -n -c 20 not port 1883
[sudo] password for usera:
tcpdump: verbose output suppressed, use -v[v]... for full protocol decode
listening on enp0s8, link-type EN10MB (Ethernet), snapshot length 262144 bytes
18:51:41.387920 IP 192.168.56.101.22 > 192.168.56.1.58422: Flags [P.], seq 867930085:867930213, ack 1185651439, win 501, length 128
18:51:41.388447 IP 192.168.56.101.22 > 192.168.56.1.58422: Flags [P.], seq 128:192, ack 1, win 501, length 64
18:51:41.388698 IP 192.168.56.1.58422 > 192.168.56.101.22: Flags [.], ack 192, win 1021, length 0
18:51:41.388854 IP 192.168.56.101.22 > 192.168.56.1.58422: Flags [P.], seq 192:272, ack 1, win 501, length 80
18:51:41.389135 IP 192.168.56.101.22 > 192.168.56.1.58422: Flags [P.], seq 272:352, ack 1, win 501, length 80
18:51:41.389262 IP 192.168.56.1.58422 > 192.168.56.101.22: Flags [.], ack 352, win 1026, length 0
18:51:41.389374 IP 192.168.56.101.22 > 192.168.56.1.58422: Flags [P.], seq 352:432, ack 1, win 501, length 80
18:51:41.389588 IP 192.168.56.101.22 > 192.168.56.1.58422: Flags [P.], seq 432:496, ack 1, win 501, length 64
18:51:41.389703 IP 192.168.56.1.58422 > 192.168.56.101.22: Flags [.], ack 496, win 1026, length 0
18:51:41.480228 IP 192.168.56.101.22 > 192.168.56.1.58422: Flags [P.], seq 496:688, ack 1, win 501, length 192
18:51:41.480415 IP 192.168.56.101.22 > 192.168.56.1.58422: Flags [P.], seq 688:752, ack 1, win 501, length 64
18:51:41.480466 IP 192.168.56.101.22 > 192.168.56.1.58422: Flags [P.], seq 752:912, ack 1, win 501, length 160
18:51:41.480529 IP 192.168.56.1.58422 > 192.168.56.101.22: Flags [.], ack 752, win 1025, length 0
18:51:41.480607 IP 192.168.56.101.22 > 192.168.56.1.58422: Flags [P.], seq 912:976, ack 1, win 501, length 64
18:51:41.480781 IP 192.168.56.1.58422 > 192.168.56.101.22: Flags [.], ack 976, win 1024, length 0
18:51:41.480898 IP 192.168.56.101.22 > 192.168.56.1.58422: Flags [P.], seq 976:1136, ack 1, win 501, length 160
18:51:41.481116 IP 192.168.56.101.22 > 192.168.56.1.58422: Flags [P.], seq 1136:1200, ack 1, win 501, length 64
18:51:41.481235 IP 192.168.56.1.58422 > 192.168.56.101.22: Flags [.], ack 1200, win 1023, length 0
18:51:41.481375 IP 192.168.56.101.22 > 192.168.56.1.58422: Flags [P.], seq 1200:1360, ack 1, win 501, length 160
18:51:41.481570 IP 192.168.56.101.22 > 192.168.56.1.58422: Flags [P.], seq 1360:1424, ack 1, win 501, length 64
20 packets captured
68 packets received by filter
0 packets dropped by kernel
```

### A4: tcpdump auf dem MQTT-Port (mqtt-vm-a, Interface: enp0s8, Filter: port 1883)
```bash
usera@mqtt-vm-a:~$ sudo tcpdump -i enp0s8 -n -c 20 port 1883
[sudo] password for usera:
tcpdump: verbose output suppressed, use -v[v]... for full protocol decode
listening on enp0s8, link-type EN10MB (Ethernet), snapshot length 262144 bytes
18:53:07.316862 IP 192.168.56.102.46974 > 192.168.56.101.1883: Flags [P.], seq 214377213:214377309, ack 145860215, win 502, options [nop,nop,TS val 644244866 ecr 350208098], length 96
18:53:07.317034 IP 192.168.56.101.1883 > 192.168.56.102.46974: Flags [P.], seq 1:5, ack 96, win 509, options [nop,nop,TS val 350253888 ecr 644244866], length 4
18:53:07.317358 IP 192.168.56.102.46974 > 192.168.56.101.1883: Flags [.], ack 5, win 502, options [nop,nop,TS val 644244867 ecr 350253888], length 0
18:53:07.321145 IP 192.168.56.101.1883 > 192.168.56.102.46974: Flags [P.], seq 5:101, ack 96, win 509, options [nop,nop,TS val 350253892 ecr 644244867], length 96
18:53:07.321543 IP 192.168.56.102.46974 > 192.168.56.101.1883: Flags [.], ack 101, win 502, options [nop,nop,TS val 644244871 ecr 350253892], length 0
18:53:07.323213 IP 192.168.56.102.46974 > 192.168.56.101.1883: Flags [P.], seq 96:100, ack 101, win 502, options [nop,nop,TS val 644244873 ecr 350253892], length 4
18:53:07.364090 IP 192.168.56.101.1883 > 192.168.56.102.46974: Flags [.], ack 100, win 509, options [nop,nop,TS val 350253935 ecr 644244873], length 0
18:53:07.364467 IP 192.168.56.102.46974 > 192.168.56.101.1883: Flags [P.], seq 100:196, ack 101, win 502, options [nop,nop,TS val 644244914 ecr 350253935], length 96
18:53:07.364495 IP 192.168.56.101.1883 > 192.168.56.102.46974: Flags [.], ack 196, win 509, options [nop,nop,TS val 350253935 ecr 644244914], length 0
18:53:07.364737 IP 192.168.56.101.1883 > 192.168.56.102.46974: Flags [P.], seq 101:105, ack 196, win 509, options [nop,nop,TS val 350253935 ecr 644244914], length 4
18:53:07.406395 IP 192.168.56.102.46974 > 192.168.56.101.1883: Flags [.], ack 105, win 502, options [nop,nop,TS val 644244956 ecr 350253935], length 0
18:53:08.279202 IP 192.168.56.102.46974 > 192.168.56.101.1883: Flags [P.], seq 196:300, ack 105, win 502, options [nop,nop,TS val 644245829 ecr 350253935], length 104
18:53:08.279413 IP 192.168.56.101.1883 > 192.168.56.102.46974: Flags [P.], seq 105:109, ack 300, win 509, options [nop,nop,TS val 350254850 ecr 644245829], length 4
18:53:08.279833 IP 192.168.56.102.46974 > 192.168.56.101.1883: Flags [.], ack 109, win 502, options [nop,nop,TS val 644245829 ecr 350254850], length 0
18:53:08.282054 IP 192.168.56.101.1883 > 192.168.56.102.46974: Flags [P.], seq 109:193, ack 300, win 509, options [nop,nop,TS val 350254853 ecr 644245829], length 84
18:53:08.282381 IP 192.168.56.102.46974 > 192.168.56.101.1883: Flags [.], ack 193, win 502, options [nop,nop,TS val 644245832 ecr 350254853], length 0
18:53:08.283808 IP 192.168.56.102.46974 > 192.168.56.101.1883: Flags [P.], seq 300:304, ack 193, win 502, options [nop,nop,TS val 644245833 ecr 350254853], length 4
18:53:08.324214 IP 192.168.56.101.1883 > 192.168.56.102.46974: Flags [.], ack 304, win 509, options [nop,nop,TS val 350254895 ecr 644245833], length 0
18:53:14.890604 IP 192.168.56.102.46970 > 192.168.56.101.1883: Flags [P.], seq 2327694560:2327694562, ack 824296485, win 501, options [nop,nop,TS val 644252440 ecr 350199996], length 2
18:53:14.890735 IP 192.168.56.101.1883 > 192.168.56.102.46970: Flags [P.], seq 1:3, ack 2, win 506, options [nop,nop,TS val 350261461 ecr 644252440], length 2
20 packets captured
21 packets received by filter
0 packets dropped by kernel
```

### A6: TCP-Client (nc) (mqtt-vm-b)
userb@mqtt-vm-b:~$ printf '%s\n' 'TESTNACHRICHT' | nc 10.0.0.1 12345
(UNKNOWN) [10.0.0.1] 12345 (?) : Connection refused
userb@mqtt-vm-b:~$
```



