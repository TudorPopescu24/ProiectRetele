#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

/* codul de eroare returnat de anumite apeluri */
extern int errno;
using namespace std;
/* portul de conectare la server*/
int port;

string read_client()
{
    string msg;
    cin >> msg;
    return msg;
}

void getInfoDepartures(string msg, int sd)
{
    char answerFromServer[1000];
    if (write(sd, msg.c_str(), 100) <= 0)
    {
        perror("[client]Eroare la write() spre server.\n");
        return;
    }
    //citirea raspunsului dat de server (apel blocant pina cind serverul raspunde)
    if (read(sd, answerFromServer, 1000) < 0)
    {
        perror("[client]Eroare la read() de la server.\n");
        return;
    }
    auto jsonAnswer = json::parse(answerFromServer);
    if (jsonAnswer.empty())
    {
        cout << endl;
        cout << "Nu exista plecari in urmatoarea ora." << endl;
        cout << endl;
    }
    else
    {
        cout << endl;
        cout << "Informatii despre plecarile din urmatoarea ora:\n";
        for (json::iterator i = jsonAnswer.begin(); i != jsonAnswer.end(); ++i)
        {
            json j = *i;
            cout << endl;
            cout << "Id tren: " << j["id"] << endl;
            cout << "Data plecare: " << j["data_plecare"] << endl;
            cout << "Data sosire: " << j["data_sosire"] << endl;
            cout << "Statie plecare: " << j["statie_plecare"] << endl;
            cout << "Statie sosire: " << j["statie_sosire"] << endl;
            string intarziere = j["intarziere"].dump();
            intarziere.erase(0, 1);
            intarziere.erase(2, 1);
            string devreme = j["devreme"].dump();
            devreme.erase(0, 1);
            devreme.erase(2, 1);
            int minutesLate = stoi(intarziere);
            int minutesEarly = stoi(devreme);
            if ((minutesLate - minutesEarly) == 0)
            {
                cout << "Trenul va pleca conform planului.\n";
            }
            else if ((minutesLate - minutesEarly) > 0)
            {
                cout << "Trenul va pleca cu o intarziere de " << minutesLate - minutesEarly << " de minute.\n";
            }
            else
            {
                cout << "Trenul va pleca mai devreme cu  " << minutesEarly - minutesLate << " de minute.\n";
            }
        }
        cout << endl;
    }
}

void getInfoArrivals(string msg, int sd)
{
    char answerFromServer[1000];
    if (write(sd, msg.c_str(), 100) <= 0)
    {
        perror("[client]Eroare la write() spre server.\n");
        return;
    }
    //citirea raspunsului dat de server (apel blocant pina cind serverul raspunde)
    if (read(sd, answerFromServer, 1000) < 0)
    {
        perror("[client]Eroare la read() de la server.\n");
        return;
    }
    auto jsonAnswer = json::parse(answerFromServer);
    if (jsonAnswer.empty())
    {
        cout << endl;
        cout << "Nu exista sosiri in urmatoarea ora." << endl;
        cout << endl;
    }
    else
    {
        cout << endl;
        cout << "Informatii despre sosirile din urmatoarea ora:\n";
        for (json::iterator i = jsonAnswer.begin(); i != jsonAnswer.end(); ++i)
        {
            json j = *i;
            cout << endl;
            cout << "Id tren: " << j["id"] << endl;
            cout << "Data plecare: " << j["data_plecare"] << endl;
            cout << "Data sosire: " << j["data_sosire"] << endl;
            cout << "Statie plecare: " << j["statie_plecare"] << endl;
            cout << "Statie sosire: " << j["statie_sosire"] << endl;
            string intarziere = j["intarziere"].dump();
            intarziere.erase(0, 1);
            intarziere.erase(2, 1);
            string devreme = j["devreme"].dump();
            devreme.erase(0, 1);
            devreme.erase(2, 1);
            int minutesLate = stoi(intarziere);
            int minutesEarly = stoi(devreme);
            if ((minutesLate - minutesEarly) == 0)
            {
                cout << "Trenul va ajunge conform planului.\n";
            }
            else if ((minutesLate - minutesEarly) > 0)
            {
                cout << "Trenul va ajunge cu o intarziere de " << minutesLate - minutesEarly << " de minute.\n";
            }
            else
            {
                cout << "Trenul va ajunge mai devreme cu " << minutesEarly - minutesLate << " de minute.\n";
            }
        }
        cout << endl;
    }
}

void sendInfo(int sd)
{
    char answerFromServer[1000];
    string id, msg;
    cout << "Introduceti id-ul trenului despre care doriti sa transmiteti informatii: ";
    cin >> id;
    msg = "ID:" + id;
    cout << msg << endl;
    if (write(sd, msg.c_str(), 100) <= 0)
    {
        perror("[client]Eroare la write() spre server.\n");
        return;
    }
    //citirea raspunsului dat de server (apel blocant pina cind serverul raspunde)
    if (read(sd, answerFromServer, 1000) < 0)
    {
        perror("[client]Eroare la read() de la server.\n");
        return;
    }
    if (strstr(answerFromServer, "false"))
    {
        cout << "Nu exista niciun tren cu id-ul " << id << ".\n"
             << endl;
        ;
        return;
    }
    else if (strstr(answerFromServer, "true"))
    {
        string command;
        cout << "Am gasit trenul cu id-ul " << id << ":\n";
        auto jsonAnswer = json::parse(answerFromServer + 4);
        cout << endl;
        cout << "Id tren: " << jsonAnswer["id"] << endl;
        cout << "Data plecare: " << jsonAnswer["data_plecare"] << endl;
        cout << "Data sosire: " << jsonAnswer["data_sosire"] << endl;
        cout << "Statie plecare: " << jsonAnswer["statie_plecare"] << endl;
        cout << "Statie sosire: " << jsonAnswer["statie_sosire"] << endl;
        cout << endl;

        cout << "In functie de tipul de informatie pe care doriti sa il trimiteti, puteti folosi urmatoarele comenzi:\n";
        cout << "'sendLateDeparture:[mins]' - trimiteti o intarziere a plecarii trenului cu [mins] minute.\n";
        cout << "'sendLateArrival:[mins]' - trimiteti o intarziere a sosirii trenului cu [mins] minute.\n";
        cout << "'sendEarlyDeparture:[mins]' - trimiteti o plecare mai devreme a trenului cu [mins] minute.\n";
        cout << "'sendEarlyArrival:[mins]' - trimiteti o sosire mai devreme a trenului cu [mins] minute.\n";
        cout << endl;
        cout << "Introduceti o comanda pentru a transmite informatiile: ";
        cin >> command;
        if (command.find("sendLateDeparture") != string::npos ||
            command.find("sendLateArrival") != string::npos ||
            command.find("sendEarlyDeparture") != string::npos ||
            command.find("sendEarlyArrival") != string::npos)
        {
            if (write(sd, command.c_str(), 100) <= 0)
            {
                perror("[client]Eroare la write() spre server.\n");
                return;
            }
            cout << "Informatiile au fost actualizate cu succes.\n";
        }
        else
            cout << "Comanda gresita. Incercati din nou.\n";
        cout << endl;
    }
}

void getInfoToday(string msg, int sd)
{
    char answerFromServer[1000];
    if (write(sd, msg.c_str(), 100) <= 0)
    {
        perror("[client]Eroare la write() spre server.\n");
        return;
    }
    //citirea raspunsului dat de server (apel blocant pina cind serverul raspunde)
    if (read(sd, answerFromServer, 1000) < 0)
    {
        perror("[client]Eroare la read() de la server.\n");
        return;
    }
    auto jsonAnswer = json::parse(answerFromServer);
    if (jsonAnswer.empty())
    {
        cout << endl;
        cout << "Nu exista trenuri care sa plece sau sa soseasca astazi." << endl;
        cout << endl;
    }
    else
    {
        cout << endl;
        cout << "Informatii despre mersul trenurilor din ziua aceasta:\n";
        for (json::iterator i = jsonAnswer.begin(); i != jsonAnswer.end(); ++i)
        {
            json j = *i;
            cout << endl;
            cout << "Id tren: " << j["id"] << endl;
            cout << "Data plecare: " << j["data_plecare"] << endl;
            cout << "Data sosire: " << j["data_sosire"] << endl;
            cout << "Statie plecare: " << j["statie_plecare"] << endl;
            cout << "Statie sosire: " << j["statie_sosire"] << endl;
        }
        cout << endl;
    }
}

void treat(string msg, int sd)
{
    if (msg == "help")
    {
        cout << endl;
        cout << "Informatii comenzi: " << endl;
        cout << "'getInfoToday' - Afiseaza informatii despre mersul trenurilor de astazi." << endl;
        cout << "'getInfoDepartures' - Afiseaza informatii despre plecarile trenurilor din urmatoarea ora." << endl;
        cout << "'getInfoArrivals' - Afiseaza informatii despre sosirile trenurilor din urmatoarea ora." << endl;
        cout << "'sendInfo' - Deschide meniul pentru transmiterea de informatii despre trenuri." << endl;
        cout << endl;
    }
    else if (msg == "getInfoToday")
    {
        getInfoToday(msg, sd);
    }
    else if (msg == "getInfoDepartures")
    {
        getInfoDepartures(msg, sd);
    }
    else if (msg == "getInfoArrivals")
    {
        getInfoArrivals(msg, sd);
    }
    else if (msg == "sendInfo")
    {
        sendInfo(sd);
    }
    else
    {
        cout << endl;
        cout << "Ati introdus o comanda gresita." << endl;
        cout << endl;
    }
}

int main(int argc, char *argv[])
{
    int sd;                    // descriptorul de socket
    struct sockaddr_in server; // structura folosita pentru conectare
        // mesajul trimis
    int nr = 0;
    char buf[10];

    /* exista toate argumentele in linia de comanda? */
    if (argc != 3)
    {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    /* stabilim portul */
    port = atoi(argv[2]);

    /* cream socketul */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr(argv[1]);
    /* portul de conectare */
    server.sin_port = htons(port);

    /* ne conectam la server */
    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return errno;
    }
    cout << "MERSUL TRENURILOR - Proiect realizat de Popescu Tudor-George" << endl;
    cout << "Bine ati venit! Puteti scrie comanda 'help' pentru a vedea toate comenzile posibile." << endl;
    flush(cout);
    /* citirea mesajului */
    while (1)
    {
        cout << "Introduceti o comanda: ";
        flush(cout);
        string mesaj = read_client();
        if (mesaj == "exit")
        {
            cout << endl;
            cout << "Ati iesit din aplicatie.\n";
            break;
        }
        else
            treat(mesaj, sd);
    }

    /* inchidem conexiunea, am terminat */
    close(sd);
}
