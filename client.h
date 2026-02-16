#ifndef CLIENT_H
#define CLIENT_H

#define HOST "63.32.125.183"
#define PORT 8081
#define PAYLOAD_TYPE "application/json"
#define ADMIN_LOGIN "/api/v1/tema/admin/login"
#define ADMIN_LOGOUT "/api/v1/tema/admin/logout"
#define USERS "/api/v1/tema/admin/users"
#define USER_LOGIN "/api/v1/tema/user/login"
#define USER_LOGOUT "/api/v1/tema/user/logout"
#define LIBRARY_ACCESS "/api/v1/tema/library/access"
#define MOVIES "/api/v1/tema/library/movies"
#define COLLECTIONS "/api/v1/tema/library/collections"
#define MAX 256

// Functie pentru autentificarea admin-ului
char* login_admin();

// Functie pentru autentificarea unui utilizator
char* get_credentials();

// Functie pentru citirea unui ID
char* get_id();

// Functie pentru adaugarea unui utilizator
char* add_user();

#endif
