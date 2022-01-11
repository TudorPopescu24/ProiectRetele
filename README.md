Mersul Trenurilor 1
Mersul Trenurilor
Proiect realizat de
Popescu Tudor-George, grupa A1, anul 2
1. Introducere
Proiectul "Mersul Trenurilor" reprezinta o aplicatie client/server care transmite si
actualizeaza in timp real informatiile despre mersul trenurilor. Utilizatorii vor avea acces
la 2 actiuni:
1. Sa ceara informatiile despre mersul trenurilor prin urmatoarele moduri:
In functie de mersul trenurilor din ziua respectiva.
In functie de plecarile din urmatoarea ora (conform cu planul, in intarziere cu x
minute).
In functie de sosirile din urmatoarea ora (conform cu planul, in intarziere cu x
minute, cu x minute mai devreme).
2. Sa trimita informatii catre server despre posibile intarzieri sau daca trenul
ajunge/pleaca mai devreme.
Toata logica aplicatiei va fi realizata in partea de server, astfel incat clientul doar va
cere informatii de la server despre plecari/sosiri si va putea transmite catre server
informatiile despre posibile intarzieri, plecari mai devreme sau estimari ale sosirilor.

2. Tehnologii utilizate
In realizarea proiectului, am ales sa utilizez un server TCP concurent. TCP
(Transmission Control Protocol) este un protocol de transport orientat conexiune, fara
pierdere de informatii, in care acestea sunt primite in ordinea in care au fost transmise
de catre server. Am ales acest protocol datorita preciziei si integralitatii datelor
transportate. Intrucat este vorba despre sosirile si plecarile trenurilor, informatiile trebuie
sa fie ca mai exacte si sa nu se piarda, o abordare UDP nefiind convenabila in acest
context.
Mersul Trenurilor 2
Pentru asigurarea concurentei, am folosit thread-uri, intrucat reprezinta o metoda
rapida si eficienta de a diviza procesele pentru clienti. Deoarece thread-urile sunt
independente unul fata de celalalt, clientul poate face request-uri fara a fi afectat de
actiunile altui client.

3. Arhitectura aplicatiei
Comunicarea cu baza de date se face prin intermediul librariei MySQL. MySQL
reprezinta un sistem de gestiune a bazelor de date relationale, bazat pe limbajul de
programare SQL. Am ales sa creez o astfel de baza de date, deoarece pot gestiona si
modifica foarte usor detaliile despre mersul trenurilor (cod unic, data plecare, data
sosire, loc plecare, destinatie, intarzieri, etc.). Toate informatiile despre mersul trenurilor
sunt stocate intr-o tabela InfoTren, care contine urmatoarele coloane: id, plecare, sosire,
data_plecare, data_sosire, intarziere_plecare, intarziere_sosire, mai_devreme_plecare,
mai_devreme_sosire.
Tabela InfoTren din baza de date SQL
Pentru transmiterea informatii despre trenuri de la server catre client am folosit JSONuri,
cu ajutorul librariei “nlohmann/json”, intrucat marimea datelor transmise este
considerabila si trebuie organizata cat mai bine. Astfel, pentru fiecare tren, am stocat
informatiile intr-un obiect de tip JSON (id, data plecare, statie plecare, etc.), in final
inserand toate obiectele intr-un JSON array pe care l-am serializat si transmis catre
client. In client, deserializez array-ul JSON primit de la server, il parcurg, si afisez
informatiile.
Logica aplicatiei este urmatoarea:
1. Clientul face un request catre server (acesta poate sa ceara sau sa trimita
informatii).
Mersul Trenurilor 3
2. Server-ul proceseaza request-ul de la client si comunica cu baza de date pentru a
prelua sau modifica informatiile necesare, in functie de request-ul clientului.
3. Dupa ce interogheaza baza de date, server-ul va transmite catre client informatiile
cerute.
4. Daca doreste, clientul are posibilitatea de a mai face alte cereri catre server, dupa
ce prima a fost procesata.
Diagrama - Arhitectura aplicatiei
Mersul Trenurilor 4

4. Detalii de implementare
Pentru comunicarea dintre client si server, am folosit un socket, intrucat utilizarea
acestuia este mai avantajoasa, dat fiind faptul ca acesta reprezinta un canal de
comunicare bidirectional, pe cand pipe-urile si fifo-urile sunt unidirectionale.
Pentru a asigura concurenta aplicatiei, in implementarea serverului am folosit threaduri,
astfel incat, pentru fiecare client care se conecteaza, este creat un nou thread care
se ocupa de cererile acestuia.
Clientul poate sa faca un numar nelimitat de cereri catre server, iar procesul client se va
inchide doar atunci cand utilizatorul va folosi comanda quit.
In structura codului de server, am implementat functii speciale pentru interogarea bazei
de date SQL, cum ar fi:
mysql_connection_setup - creeaza conexiunea catre baza de date in care sunt
stocate informatiile despre mersul trenurilor
mysql_perform_query - creeaza interogarea necesara pentru
preluarea/modificarea informatiilor din baza de date, in functie de request-ul
clientului
Pentru serializarea si deserializarea JSON-urilor am folosit functiile json.dump() si
json::parse() din libraria nlohmann/json.
Construirea si Serializarea array-ului JSON
Mersul Trenurilor 5
Totodata, in server, am creat functii care trateaza cererile diferite pe care le poate face
clientul:
getInfoToday - preia informatiile din baza de date despre mersul trenurilor din ziua
respectiva
getDepartures - ofera informatii despre plecarile din urmatoarea ora (conform cu
planul, in intarziere cu x minute)
getArrivals - ofera informatii despre sosirile din urmatoarea ora (conform cu planul,
in intarziere cu x minute, cu x minute mai devreme)
checkIfTrainExists - verifica daca id-ul trenului transmis de client exista in baza de
date
sendLateDeparture - actualizeaza baza de date cu informatii despre intarziere
plecare
sendLateArrival - actualizeaza baza de date cu informatii despre intarziere sosire
sendEarlyDeparture - actualizeaza baza de date cu informatii despre plecare mai
devreme
sendEarlyArrival - actualizeaza baza de date cu informatii despre sosire mai
devreme
Functia getDepartures
Mersul Trenurilor 6
Conectarea la baza de date SQL
Diagrama - Arhitectura aplicatiei cu detalii de implementare
Mersul Trenurilor 7

5. Concluzii si imbunatatiri
Sa fie implementata o interfata grafica pentru comenzile pe care le poate executa
clientul, pentru a face aplicatia cat mai atractiva, intuitiva si usor de utilizat.
Sa existe roluri de user si admin:
Un user poate cere informatii despre mersul trenurilor si sa transmita posibile
intarzieri, plecari mai devreme, estimari sosire.
Un admin poate adauga mersul trenurilor pentru ziua urmatoare, poate
restrictiona accesul utilizatorilor care transmit informatii false, etc.
Sa existe si alte modalitati de a cere informatii pentru un client (e.g. sa ceara
informatii despre trenuri in functie de ruta, destinatie, plecare, ziua urmatoare, etc.).

6. Bibliografie
1. Model de server TCP concurent implementat cu thread-uri
https://profs.info.uaic.ro/~computernetworks/cursullaboratorul.php
2. Database Connection in C++ with MySQL
https://www.youtube.com/watch?v=cSZvq7Kv6_0&ab_channel=Steve'steacher
3. MySQL Database Connection + Performing Queries
https://github.com/WeebNetsu/database-connection-cpp
4. Librarie JSON
https://github.com/nlohmann/json
