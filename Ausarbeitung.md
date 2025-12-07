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
  - [5.3 Wireshark-Analyse](#53-wireshark-analyse)
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

## 3.2 Architekturübersicht

Bedingt durch die Entwicklung einer eigenen Anwendung zum Tunneln von Daten via MQTT bietet sich der Einsatz einer TUN-Device an. Ein TUN-Gerät ist eine spezifische Art von virtuellem Netzwerkgerät im Linux-Kernel, das zur Implementierung von Netzwerk-Tunneln verwendet wird, insbesondere für VPN-Dienste (Virtual Private Network).
Es arbeitet auf der Netzwerkschicht (Schicht 3) und verarbeitet IP-Pakete. [19].

Bei einem TUN-Device handelt es sich um eine virtuelle Netzwerkschnittstelle und kann vereinfacht als Punkt-zu-Punkt- oder Ethernet-Gerät betrachtet werden. Die Besonderheit liegt darin, dass anstelle einer Datenübertragung per physischen Medium, die Datenverbindung virtuell über eine eigene Anwendung aufgebaut wird. [19]

Der Kernel behandelt die Datenpakete der TUN-Device genauso, als würden diese von einem echten physischen Gerät abstammen. Die TUN-Device übernimmt dabei sämtliche Netzwerkrelevanten Aufgaben wie z.B. auch die Segmentierung des ankommenden Datenstroms. Aus diesem Grund kann an dieser Stelle auf weitere Details zu diesem Thema verzichtet werden.


## 3.3 Kommunikationsmodell

Für die bidirektionale Kommunikation zwischen Tunnel-Client und Tunnel-Server werden bewusst **zwei getrennte MQTT-Topics** verwendet:

- `tunnel/client_to_server`
- `tunnel/server_to_client`

Diese Entscheidung wurde aus folgenden Gründen getroffen:

- sie verhindert zuverlässig, dass ein Client seine eigenen Nachrichten wieder empfängt  
- viele verbreitete MQTT-Broker unterstützen das MQTT-5-Flag `no_local` nicht oder verarbeiten es inkonsistent  
- dadurch ist die Lösung auch mit **MQTT 3.1.1** vollständig kompatibel  
- das Verhalten bleibt deterministisch und unabhängig vom Broker  
- die Implementierung wird robuster gegenüber unterschiedlichen Broker-Implementierungen (Mosquitto, EMQX, HiveMQ, VerneMQ usw.)

Durch diese klare Trennung der Datenrichtungen wird ein stabiler und vorhersagbarer Tunnelbetrieb gewährleistet.

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
18. http://www.bbs-1.de/bbs1/umat/netze/netz8.html
19. https://www.kernel.org/doc/html/latest/networking/tuntap.html

