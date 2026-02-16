# REST-API-Web-Client
# Tema 4. Client web. Comunicatie cu REST API

## Structura proiect:
- client.c
- client.h
- helpers.c
- helpers.h
- parson.c
- parson.h
- buffer.c
- buffer.h
- requests.c
- requests.h

## Resurse:
Fisierele helpers.c, helpers.h, buffer.c, bufferh, requests.c, requests.h sunt luate din cadrul laboratorului 9 (Protocolul HTTP), iar parson.c, parson.h din link-ul primit in enuntul temei (https://github.com/kgabis/parson).
Pe langa ce se afla in codul din laboratorul 9, in fisierul request.c, care continea functii pentru request-uri de tip GET si POST, am adaugat functii pentru request-urile de tip DELETE si PUT, pe care le-am folosit pentru comenzile:
- "delete_user",
- "delete_movie",
- "delete_collection",
- "delete_movie_from_collection",

respectiv pentru comanda:
- "update_movie"

Pe cele pentru cererile GET le-am folosit la comenzile:
- "get_users",
- "logout_admin",
- "get_access",
- "logout",
- "get_movies",
- "get_movie",
- "get_collections",
- "get_collection",

iar cele pentru cererile de tip POST au fost folosite la:
- "login_admin",
- "add_user",
- "login",
- "add_movie",
- "add_collection",
- "add_movie_to_collection".

## client.c / client.h
In acest fisier se afla implementarea propriu-zisa a aplicatiei web. In client.h sunt definite rutele de acces catre care se trimit request-urile si cateva functii de baza:
- "login_admin" care preia de la tastatura username-ul si parola pentru autentificarea admin-ului, verifica daca contine spatii si returneaza un string in format JSON ce contine datele citite anterior.
- "get_credentials" care face aproape acelasi ca "login_admin", dar pentru autentificarea unui utilizator oarecare, iar in plus citeste de la tastatura si numele admin-ului.
- "get_id" care citeste de la tastatura un id si este folosita pentru functiile in care ruta de acces este compusa si dintr-un "movieId" sau "collectionId"
- "add_user" care se comporta ca "login_admin" dar este folosita pentru adaugarea unui utilizator

## main:

#### Alocarea memoriei. Conexiunea la server
Aloc memorie pentru comanda ce urmeaza sa fie citita de la tastatura, deschid conexiunea cu serverul, folosindu-ma de functia "open_connection" din fisierul "helpers.c", preluat din laborator, iar cat timp comanda nu este "exit", moment in care conexiunea cu serverul va fi oprita ("close_connection" din "helpers.c"), se citesc de la tastastura diferite comenzi.

#### Gestionarea fluxului de comenzi
In functie de fiecare, folosesc functiile implementate mai sus, verific cookie-ul sau token-ul, afisez o eroare daca este cazul, iar daca nu continui. Creez in functie de fiecare comanda cate un request de tipul GET, POST, DELETE sau PUT, trimit request-ul catre server folosind functia "sent_to_server" (implementata in "helpers.c"), retin raspunsul de la server intr-o variabila "response", careia mai apoi ii verific continutul pentru a afisa un mesaj de eroare sau de succes, dupa caz. Am adaugat comentarii in cod la fiecare gestionare a comenzilor.

#### Format JSON
Pentru comenzile in care se citesc sau se afiseaza date in format JSON folosesc functiile din fisierul "parson.c" si creez obiecte de tip JSON. Serializez obiectul JSON in string, iar apoi il dau ca parametru functiei ce trimite request-ul, pe post de "body_data".

## requests.c / requests.h
- Functii pentru cererile HTTP (GET, POST, DELETE, PUT)

## helpers.c / helpers.h
- Functii pentru deschiderea si inchiderea conexiunii cu serverul
- Functii pentru trimiterea si primirea mesajelor de la server

## buffer.c / buffer.h
- Structura de date si functii pentru un bufferce este folosit pentru gestionarea datelor de la server
