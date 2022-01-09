/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
	Intoarce corect identificatorul din program al thread-ului.
  
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <mysql/mysql.h>
#include <iostream>
#include <nlohmann/json.hpp>

/* portul folosit */
#define PORT 2908
using namespace std;
using json = nlohmann::json;

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData
{
    int idThread; //id-ul thread-ului tinut in evidenta de acest program
    int cl;       //descriptorul intors de accept
} thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
void getInfoToday(MYSQL *con, struct thData tdL);
void getDepartures(MYSQL *con, struct thData tdL);
void getArrivals(MYSQL *con, struct thData tdL);

struct connection_details
{
    const char *server, *user, *password, *database;
};

MYSQL *mysql_connection_setup()
{
    struct connection_details mysql_details;
    mysql_details.server = "localhost";          // where the mysql database is
    mysql_details.user = "tudor";                // user
    mysql_details.password = "password";         // the password for the database
    mysql_details.database = "MersulTrenurilor"; // the databse
    MYSQL *connection = mysql_init(NULL);        // mysql instance

    //connect database
    if (!mysql_real_connect(connection, mysql_details.server, mysql_details.user, mysql_details.password, mysql_details.database, 0, NULL, 0))
    {
        cout << "Connection Error: " << mysql_error(connection) << endl;
        exit(1);
    }

    return connection;
}

// mysql_res = mysql result
MYSQL_RES *mysql_perform_query(MYSQL *connection, const char *sql_query)
{
    //send query to db
    if (mysql_query(connection, sql_query))
    {
        cout << "MySQL Query Error: " << mysql_error(connection) << endl;
        exit(1);
    }

    return mysql_use_result(connection);
}

int main()
{
    struct sockaddr_in server; // structura folosita de server
    struct sockaddr_in from;
    int nr; //mesajul primit de trimis la client
    int sd; //descriptorul de socket
    int pid;
    pthread_t th[100]; //Identificatorii thread-urilor care se vor crea
    int i = 0;

    /* crearea unui socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Eroare la socket().\n");
        return errno;
    }
    /* utilizarea optiunii SO_REUSEADDR */
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    /* pregatirea structurilor de date */
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons(PORT);

    /* atasam socketul */
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen(sd, 2) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return errno;
    }
    /* servim in mod concurent clientii...folosind thread-uri */
    while (1)
    {
        int client;
        thData *td; //parametru functia executata de thread
        socklen_t length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        // client= malloc(sizeof(int));
        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        /* s-a realizat conexiunea, se astepta mesajul */

        // int idThread; //id-ul threadului
        // int cl; //descriptorul intors de accept

        td = (struct thData *)malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;

        pthread_create(&th[i], NULL, &treat, td);

    } //while
};
static void *treat(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);

    printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush(stdout);

    pthread_detach(pthread_self());
    while (1)
    {
        raspunde((struct thData *)arg);
    }
    /* am terminat cu acest client, inchidem conexiunea */
    close((intptr_t)arg);
    return (NULL);
};

void raspunde(void *arg)
{
    int i = 0;
    char msg[100];
    struct thData tdL;
    MYSQL *con; // the connection
    // connect to the mysql database
    con = mysql_connection_setup();

    tdL = *((struct thData *)arg);
    if (read(tdL.cl, msg, 100) <= 0)
    {
        printf("[Thread %d]\n", tdL.idThread);
        perror("Eroare la read() de la client.\n");
    }

    string mesaj(msg);

    if (mesaj == "getInfoToday")
    {
        getInfoToday(con, tdL);
    }
    else if (mesaj == "getInfoDepartures")
    {
        getDepartures(con, tdL);
    }
    else if (mesaj == "getInfoArrivals")
    {
        getArrivals(con, tdL);
    }

    // close database connection
    mysql_close(con);
}

void getInfoToday(MYSQL *con, struct thData tdL)
{
    MYSQL_RES *res; // the results
    MYSQL_ROW row;  // the results rows (array)
    json answer = json::array();
    res = mysql_perform_query(con, "select * from InfoTren where (Date(data_plecare) = CURDATE()) OR (Date(data_sosire) = CURDATE());");
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        json j;
        j["id"] = row[0];
        j["statie_plecare"] = row[1];
        j["statie_sosire"] = row[2];
        j["data_plecare"] = row[3];
        j["data_sosire"] = row[4];
        answer.push_back(j);
    }
    string s = answer.dump();
    /* returnam mesajul clientului */
    if (write(tdL.cl, s.c_str(), 1000) <= 0)
    {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
    else
        printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
    // clean up the database result
    mysql_free_result(res);
}

void getDepartures(MYSQL *con, struct thData tdL)
{
    MYSQL_RES *res; // the results
    MYSQL_ROW row;  // the results rows (array)
    json answer = json::array();
    res = mysql_perform_query(con, "select * from InfoTren where TIMESTAMPDIFF(minute, TIME(data_plecare), TIME(NOW())) <= 60 AND date(data_plecare) = CURRENT_DATE;");
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        json j;
        j["id"] = row[0];
        j["statie_plecare"] = row[1];
        j["statie_sosire"] = row[2];
        j["data_plecare"] = row[3];
        j["data_sosire"] = row[4];
        answer.push_back(j);
    }
    string s = answer.dump();
    /* returnam mesajul clientului */
    if (write(tdL.cl, s.c_str(), 1000) <= 0)
    {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
    else
        printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
    // clean up the database result
    mysql_free_result(res);
}

void getArrivals(MYSQL *con, struct thData tdL)
{
    MYSQL_RES *res; // the results
    MYSQL_ROW row;  // the results rows (array)
    json answer = json::array();
    res = mysql_perform_query(con, "select * from InfoTren where ABS(TIMESTAMPDIFF(minute, TIME(data_sosire), TIME(NOW()))) <= 60 AND date(data_sosire) = CURRENT_DATE;");
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        json j;
        j["id"] = row[0];
        j["statie_plecare"] = row[1];
        j["statie_sosire"] = row[2];
        j["data_plecare"] = row[3];
        j["data_sosire"] = row[4];
        answer.push_back(j);
    }
    string s = answer.dump();
    /* returnam mesajul clientului */
    if (write(tdL.cl, s.c_str(), 1000) <= 0)
    {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
    else
        printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
    // clean up the database result
    mysql_free_result(res);
}