#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "client.h"
#include "helpers.h"
#include "requests.h"
#include "parson.h"

char* login_admin() {
    char *username = calloc(MAX, sizeof(char));
    char *password = calloc(MAX, sizeof(char));
    char *admin_string;

    // citesc numele si parola
    printf("username=");
    fgets(username, MAX, stdin);
    printf("password=");
    fgets(password, MAX, stdin);

    username[strlen(username) - 1] = '\0';
    password[strlen(password) - 1] = '\0';

    // verific sa nu aiba spatii
    if (strchr(username, ' ') != NULL || strchr(password, ' ') != NULL) {
        free(username);
        free(password);
        return NULL;
    }

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    admin_string = json_serialize_to_string_pretty(root_value);

    free(username);
    free(password);
    json_value_free(root_value);

    return admin_string;
}

char* get_credentials() {
    char *admin_username;
    char *username;
    char *password;
    char *user_string;

    admin_username = calloc(MAX, sizeof(char));
    username = calloc(MAX, sizeof(char));
    password = calloc(MAX, sizeof(char));
    user_string = calloc(MAX, sizeof(char));

    // citesc numele admin-ului, username-ul si parola
    printf("admin_username=");
    fgets(admin_username, MAX, stdin);
    printf("username=");
    fgets(username, MAX, stdin);
    printf("password=");
    fgets(password, MAX, stdin);

    // verific sa nu aiba spatii
    if (strchr(admin_username, ' ') != NULL || strchr(username, ' ') != NULL) {
        free(admin_username);
        free(username);
        free(password);
        return NULL;
    }

    admin_username[strlen(admin_username) - 1] = '\0';
    username[strlen(username) - 1] = '\0';
    password[strlen(password) - 1] = '\0';

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    // adaug datele in obiectul JSON
    json_object_set_string(root_object, "admin_username", admin_username);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    // convertesc obiectul JSON in string
    user_string = json_serialize_to_string_pretty(root_value);

    free(admin_username);
    free(username);
    free(password);
    json_value_free(root_value);

    return user_string;
}

char* get_id() {
    char *id;

    id = calloc(MAX, sizeof(char));

    // citesc id-ul
    printf("id=");
    fgets(id, MAX, stdin);

    id[strlen(id) - 1] = '\0';

    return id;
}

char* add_user() {
    char *username = calloc(MAX, sizeof(char));
    char *password = calloc(MAX, sizeof(char));
    char *user_string;

    // citesc username-ul si parola
    printf("username=");
    fgets(username, MAX, stdin);
    printf("password=");
    fgets(password, MAX, stdin);

    username[strlen(username) - 1] = '\0';
    password[strlen(password) - 1] = '\0';

    // verific sa nu aiba spatii
    if (strchr(username, ' ') != NULL || strchr(password, ' ') != NULL) {
        free(username);
        free(password);
        return NULL;
    }

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    user_string = json_serialize_to_string_pretty(root_value);

    free(username);
    free(password);
    json_value_free(root_value);

    return user_string;
}

int main(int argc, char *argv[]) {
    char *command;
    char *message;
    char *response;
    char *cookie = NULL;
    char *token = NULL;
    int sockfd;
    char *logged_username = NULL;

    while(1) {
        // citesc comanda de la tastatura
        command = calloc(MAX, sizeof(char));
        fgets(command, MAX, stdin);

        // deschid conexiunea la server
        sockfd = open_connection(HOST, PORT, PF_INET, SOCK_STREAM, 0);

        if (!strcmp(command, "exit\n")) {

            // inchid conexiunea la server
            close_connection(sockfd);
            break;

        } else if (!strcmp(command, "login_admin\n")) {

            char *admin_string;

            // iau credentialele admin-ului
            admin_string = login_admin();

            // daca admin-ul este deja logat
            if (cookie != NULL) {
                printf("ERROR: Admin deja logat!\n");
                json_free_serialized_string(admin_string);
                continue;
            }
            
            // creez POST request-ul
            message = compute_post_request(
                HOST,
                ADMIN_LOGIN,
                PAYLOAD_TYPE, 
                &admin_string, 1, 
                NULL, NULL);

            // trimit request-ul la server
            send_to_server(sockfd, message);

            // raspunsul de la server
            response = receive_from_server(sockfd);

            // verific daca credentialele se potrivesc
            if (strstr(response, "Credentials are not good!") != NULL) {
                printf("ERROR: Credentialele nu se potrivesc!\n");
                json_free_serialized_string(admin_string);
                free(response);
                continue;
            }

            // creez cookie-ul
            char *cookie_start = strstr(response, "Set-Cookie:");
            if (cookie_start != NULL) {
                cookie_start += strlen("Set-Cookie: ");
                char *cookie_end = strstr(cookie_start, ";");
                if (cookie_end != NULL) {
                    size_t cookie_length = cookie_end - cookie_start;
                    cookie = calloc(cookie_length + 1, sizeof(char));
                    strncpy(cookie, cookie_start, cookie_length);
                    printf("SUCCESS: Admin autentificat cu succes!\n");
                }
            } else {
                printf("ERROR: Nu s-a intors cookie de sesiune!\n");
            }

            token = NULL;

            json_free_serialized_string(admin_string);
            free(response);

        } else if (!strcmp(command, "add_user\n")) {

            char *user_string;

            if (cookie == NULL) {
                printf("ERROR: Lipsa rol admin!\n");
                continue;
            }

            user_string = add_user();
            if (user_string == NULL) {
                printf("ERROR: Numele nu poate contine spatii!\n");
                continue;
            }

            // creez POST request-ul
            message = compute_post_request(
                HOST,
                USERS,
                PAYLOAD_TYPE, 
                &user_string, 1, 
                cookie, NULL);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific raspunsul de la server ca sa afisez erorile daca e cazul
            if (strstr(response, "Missing admin role") != NULL) {
                printf("ERROR: Lipsa rol admin!\n");
            } else if (strstr(response, "Incomplete or incorrect information") != NULL) {
                printf("ERROR: Informatii incomplete/incorecte!\n");
            } else if (strstr(response, "User already exists") != NULL ||
                    strstr(response, "409 Conflict") != NULL) {
                printf("ERROR: Utilizatorul exista deja!\n");
            } else {
                printf("SUCCESS: Utilizator adaugat cu succes!\n");
            }

            json_free_serialized_string(user_string);
            free(response);

        } else if (!strcmp(command, "get_users\n")) {

            if (cookie == NULL) {
                printf("ERROR: Lipsa rol admin!\n");
                continue;
            }

            // creeaz GET request-ul
            message = compute_get_request(
                HOST,
                USERS,
                NULL, 
                cookie, token);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific daca user-ul are acces la librarie
            if (strstr(response, "Authorization header is missing!") != NULL) {
                printf("ERROR: Nu ai acces la librarie\n");
                continue;
            }
            
            char *result = strchr(response, '[');
            JSON_Value *root_value = json_parse_string(result);
            JSON_Array *root_array = json_value_get_array(root_value);

            printf("SUCCESS: Lista utilizatorilor\n");
            for (int i = 0; i < json_array_get_count(root_array); i++) {
                JSON_Object *root_object = json_array_get_object(root_array, i);
                const int id = json_object_get_number(root_object, "id");
                const char *password = json_object_get_string(root_object, "password");
                const char *username = json_object_get_string(root_object, "username");
                
                printf("#%d %s:%s\n", id, username, password);
            }

            json_value_free(root_value);

        } else if (!strcmp(command, "delete_user\n")) {

            char *username = calloc(MAX, sizeof(char));

            printf("username=");
            fgets(username, MAX, stdin);
            username[strlen(username) - 1] = '\0';

            if (strchr(username, ' ') != NULL) {
                printf("ERROR: Username invalid!\n");
                free(username);
                continue;
            }

            char *url = calloc(strlen(USERS) + strlen(username) + 2, sizeof(char));
            sprintf(url, "%s/%s", USERS, username);

            if (cookie == NULL) {
                printf("ERROR: Lipsa rol admin!\n");
                free(username);
                free(url);
                continue;
            }

            // creez DELETE request-ul
            message = compute_delete_request(HOST, url, cookie, NULL);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific raspunsul de la server ca sa afisez erorile daca e cazul
            if (strstr(response, "Admin privileges required") != NULL) {
                printf("ERROR: Lipsa rol admin!\n");
                continue;
            } else if (strstr(response, "Invalid username") != NULL) {
                printf("ERROR: Username invalid!\n");
                continue;
            } else {
                printf("SUCCESS: Utilizator sters!\n");
                continue;
            }

            free(username);
            free(url);
            free(response);

        } else if (!strcmp(command, "logout_admin\n")) {

            if (cookie == NULL) {
                printf("ERROR: Nu esti autentificat ca admin!\n");
                continue;
            }

            // creez GET request-ul
            message = compute_get_request(HOST, ADMIN_LOGOUT, NULL, cookie, NULL);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific raspunsul de la server ca sa afisez erorile daca e cazul
            if (strstr(response, "You are not logged in") != NULL) {
                printf("ERROR: Nu esti autentificat ca admin!\n");
            } else {
                printf("SUCCESS: Admin delogat!\n");
                free(cookie);
                cookie = NULL;
            }

            free(response);

        } else if (!strcmp(command, "login\n")) {
            char *user_string;

            user_string = get_credentials();

            if (user_string == NULL) {
                printf("ERROR: Username-ul sau admin username-ul nu pot contine spatii!\n");
                continue;
            }

            // creez POST request-ul
            message = compute_post_request(
                HOST,
                USER_LOGIN,
                PAYLOAD_TYPE,
                &user_string, 1,
                NULL, NULL);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific daca credentialele se potrivesc
            if (strstr(response, "Credentials are not good") != NULL) {
                printf("ERROR: Credentialele nu se potrivesc!\n");
                continue;
            } else if (strstr(response, "Already authenticated") != NULL) {
                printf("ERROR: Utilizatorul este deja autentificat!\n");
                continue;
            }

            // creez cookie-ul
            char *cookie_start = strstr(response, "Set-Cookie:");
            if (cookie_start != NULL) {
                cookie_start += strlen("Set-Cookie: ");
                char *cookie_end = strstr(cookie_start, ";");
                if (cookie_end != NULL) {
                    size_t cookie_length = cookie_end - cookie_start;
                    cookie = calloc(cookie_length + 1, sizeof(char));
                    strncpy(cookie, cookie_start, cookie_length);
                    printf("SUCCESS: Autentificare reusita!\n");
                }
            } else {
                printf("ERROR: Nu s-a intors cookie de sesiune, trebuie sa fii autentificat!\n");
            }

            JSON_Value *login_val = json_parse_string(user_string);
            JSON_Object *login_obj = json_value_get_object(login_val);
            const char *username = json_object_get_string(login_obj, "username");
            if (logged_username != NULL) free(logged_username);
            logged_username = calloc(strlen(username) + 1, sizeof(char));
            strcpy(logged_username, username);
            json_value_free(login_val);

            json_free_serialized_string(user_string);
            free(response);

        } else if (!strcmp(command, "get_access\n")) {

            // creez GET request-ul
            message = compute_get_request(HOST, LIBRARY_ACCESS, NULL, cookie, NULL);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific daca autentificarea a avut loc
            if (strstr(response, "You are not logged in") != NULL) {
                printf("ERROR: Utilizatorul nu este autentificat!\n");
                free(response);
                continue;
            }

            // extrag token-ul JWT din raspunsul de la server
            char *token_start = strstr(response, "\"token\":\"");
            if (token_start != NULL) {
                token_start += strlen("\"token\":\"");
                char *token_end = strchr(token_start, '\"');
                if (token_end != NULL) {
                    size_t token_len = token_end - token_start;
                    free(token);
                    token = calloc(token_len + 1, sizeof(char));
                    strncpy(token, token_start, token_len);
                    printf("SUCCESS: Token JWT primit!\n");
                } else {
                    printf("ERROR: Token invalid!\n");
                }
            } else {
                printf("ERROR: Token-ul JWT nu a fost primit!\n");
            }

            free(response);
            
        } else if (!strcmp(command, "logout\n")) {

            // verific daca utilizatorul este autentificat ca sa nu mai fac request
            if (cookie == NULL) {
                printf("Utilizatorul nu este autentificat!\n");
                continue;
            }

            // creez GET request-ul
            message = compute_get_request(HOST, USER_LOGOUT, NULL, cookie, NULL);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific daca utilizatorul este autentificat
            if (strstr(response, "You are not logged in") != NULL) {
                printf("ERROR: Utilizatorul nu este autentificat!\n");
            } else {
                printf("SUCCESS: Utilizator delogat!\n");
                free(cookie);
                cookie = NULL;
            }

            free(response);

        } else if (!strcmp(command, "add_movie\n")) {
        
            char *title;
            char *year;
            char *description;
            char *rating;
        
            // aloc memorie
            title = calloc(MAX, sizeof(char));
            year = calloc(MAX, sizeof(char));
            description = calloc(MAX, sizeof(char));
            rating = calloc(MAX, sizeof(char));

            // citesc datele filmului de la tastatura
            printf("title=");
            fgets(title, MAX, stdin);
            printf("year=");
            fgets(year, MAX, stdin);
            printf("description=");
            fgets(description, MAX, stdin);
            printf("rating=");
            fgets(rating, MAX, stdin);

            // verific daca s-au introdus complet datele
            if (strlen(title) == 1 || 
                strlen(year) == 1 || 
                strlen(description) == 1 || 
                strlen(rating) == 1) {
                printf("ERROR: Date invalide/incomplete!\n");
                continue;
            }

            title[strlen(title) - 1] = '\0';
            year[strlen(year) - 1] = '\0';
            description[strlen(description) - 1] = '\0';
            rating[strlen(rating) - 1] = '\0';

            // convertesc rating-ul in double si verific daca e valid
            double rating_value = atof(rating);
            if (rating_value < 0.0 || rating_value > 10.0) {
                printf("ERROR: Rating invalid!\n");
                free(title);
                free(year);
                free(description);
                free(rating);
                continue;
            }

            // creez obiectul JSON
            JSON_Value *json_value = json_value_init_object();;
            JSON_Object *json_object = json_value_get_object(json_value);

            // adaug datele in obiectul JSON
            json_object_set_string(json_object, "title", title);
            json_object_set_number(json_object, "year", atoi(year));
            json_object_set_string(json_object, "description", description);
            json_object_set_number(json_object, "rating", atof(rating));
            char *movie_string = json_serialize_to_string_pretty(json_value);

            // creez POST request-ul
            message = compute_post_request(
                HOST,
                MOVIES,
                PAYLOAD_TYPE, 
                &movie_string, 1, 
                cookie, token);    

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific daca utilizatorul are acces la library
            if (strstr(response, "Authorization header is missing") != NULL) {
                printf("ERROR: Nu ai acces la library!\n");
                continue;
            }

            // verific daca datele sunt complete si valide
            if (strstr(response, "Something Bad Happened") != NULL) {
                printf("ERROR: Date invalide/incomplete!\n");
                continue;
            }

            printf("SUCCESS: Film adaugat\n");

            json_free_serialized_string(movie_string);
            json_value_free(json_value);
            free(title);
            free(year);
            free(description);
            free(rating);

        } else if (!strcmp(command, "get_movies\n")) {

            // verific daca utilizatorul are acces la library ca sa nu mai fac request-ul
            if (token == NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
                continue;
            }

            // creeaz GET request-ul
            message = compute_get_request(
                HOST,
                MOVIES,
                NULL, 
                cookie, token);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific daca utilizatorul are acces la library
            if (strstr(response, "Authorization header is missing") != NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
                continue;
            }
            
            // printez rezultatul
            char *result = strchr(response, '[');
            JSON_Value *root_value = json_parse_string(result);
            JSON_Array *root_array = json_value_get_array(root_value);

            printf("SUCCESS: Lista filmelor\n");
            for (int i = 0; i < json_array_get_count(root_array); i++) {
                JSON_Object *movie_object = json_array_get_object(root_array, i);
                const int id = (int)json_object_get_number(movie_object, "id");
                const char *title = json_object_get_string(movie_object, "title");

                printf("#%d %s\n", id, title);
            }

            json_value_free(root_value);

        } else if (!strcmp(command, "get_movie\n")) {
            char *id = get_id();

            // creez adresa URL impreuna cu id-ul filmului
            char *url = calloc(strlen(MOVIES) + strlen(id) + 2, sizeof(char));
            sprintf(url, "%s/%s", MOVIES, id);

            // creez GET request-ul
            message = compute_get_request(
                HOST,
                url,
                NULL, 
                cookie, token);

            send_to_server(sockfd, message);
            
            response = receive_from_server(sockfd);

            // verific raspunsul de la server ca sa afisez erorile daca este cazul
            if (strstr(response, "Authorization header is missing") != NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
                continue;
            } else if (strstr(response, "id is not int") != NULL ||
                strstr(response, "Movie not found") != NULL) {
                printf("ERROR: ID invalid!\n");
                continue;
            }

            // afisez rezultatul in format JSON
            char *result = strchr(response, '{');
            JSON_Value *root_value = json_parse_string(result);
            JSON_Object *root_object = json_value_get_object(root_value);

            const int id_movie = json_object_get_number(root_object, "id");
            const char *title = json_object_get_string(root_object, "title");
            const int year = json_object_get_number(root_object, "year");
            const char *description = json_object_get_string(root_object, "description");
            const char *rating_str = json_object_get_string(root_object, "rating");

            // convertesc rating-ul in double
            double rating = 0.0;
            if (rating_str != NULL) {
                rating = atof(rating_str);
            }
                
            printf("id=%d\n", id_movie);
            printf("title=%s\n", title);
            printf("year=%d\n", year);
            printf("description=%s\n", description);
            printf("rating=%.1f\n", rating);
            
            json_value_free(root_value);
            free(id);

        } else if (!strcmp(command, "delete_movie\n")) {
            char *id = get_id();

            // creez adresa URL impreuna cu id-ul filmului
            char *url = calloc(strlen(MOVIES) + strlen(id) + 2, sizeof(char));
            sprintf(url, "%s/%s", MOVIES, id);

            // creez DELETE request-ul
            message = compute_delete_request(
                HOST, 
                url, 
                NULL, 
                token);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific raspunsul de la server ca sa afisez erorile daca este cazul
            if (strstr(response, "Authorization header is missing") != NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
                continue;
            } else if (strstr(response, "Invalid movie id") != NULL) {
                printf("ERROR: ID invalid!\n");
                continue;
            }

            printf("SUCCESS: Film sters cu succes\n");

            free(id);

        } else if (!strcmp(command, "update_movie\n")) {

            char *id = calloc(MAX, sizeof(char));
            char *title = calloc(MAX, sizeof(char));
            char *year = calloc(MAX, sizeof(char));
            char *description = calloc(MAX, sizeof(char));
            char *rating = calloc(MAX, sizeof(char));

            // citesc datele filmului de la tastatura
            printf("id=");
            fgets(id, MAX, stdin);
            printf("title=");
            fgets(title, MAX, stdin);
            printf("year=");
            fgets(year, MAX, stdin);
            printf("description=");
            fgets(description, MAX, stdin);
            printf("rating=");
            fgets(rating, MAX, stdin);

            id[strlen(id) - 1] = '\0';
            title[strlen(title) - 1] = '\0';
            year[strlen(year) - 1] = '\0';
            description[strlen(description) - 1] = '\0';
            rating[strlen(rating) - 1] = '\0';

            if (strlen(id) == 0 || strlen(title) == 0 || strlen(year) == 0 || 
                strlen(description) == 0 || strlen(rating) == 0) {
                printf("ERROR: Date invalide/incomplete!\n");
                free(id);
                free(title);
                free(year);
                free(description);
                free(rating);
                continue;
            }

            // creez obiectul JSON
            JSON_Value *json_value = json_value_init_object();
            JSON_Object *json_object = json_value_get_object(json_value);

            json_object_set_string(json_object, "title", title);
            json_object_set_number(json_object, "year", atoi(year));
            json_object_set_string(json_object, "description", description);
            
            // convertesc rating-ul in double
            double rating_value = atof(rating);
            json_object_set_number(json_object, "rating", rating_value);

            char *movie_string = json_serialize_to_string_pretty(json_value);

            // creez adresa URL impreuna cu id-ul filmului
            char *url = calloc(strlen(MOVIES) + strlen(id) + 2, sizeof(char));
            sprintf(url, "%s/%s", MOVIES, id);

            // creez PUT request-ul
            message = compute_put_request(
                HOST,
                url,
                PAYLOAD_TYPE,
                &movie_string,
                1,
                cookie,
                token);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);
            
            // verific raspunsul de la server
            if (strstr(response, "Authorization header is missing") != NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
                continue;
            } else if (strstr(response, "Invalid movie id") != NULL) {
                printf("ERROR: ID invalid!\n");
                continue;
            } else if (strstr(response, "Invalid payload") != NULL) {
                printf("ERROR: Date invalide/incomplete!\n");
                continue;
            } else {
                printf("SUCCESS: Film actualizat!\n");
            }

            json_free_serialized_string(movie_string);
            json_value_free(json_value);
            free(id);
            free(title);
            free(year);
            free(description);
            free(rating);
            free(url);
            free(response);

        } else if (!strcmp(command, "get_collections\n")) {

            // verific daca utilizatorul are acces la library ca sa nu mai fac request-ul
            if (token == NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
                continue;
            }

            // creez GET request-ul
            message = compute_get_request(
                HOST,
                COLLECTIONS,
                NULL,
                cookie,
                token);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific raspunsul de la server
            if (strstr(response, "Authorization header is missing") != NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
                continue;
            }

            // printez rezultatul in format JSON
            char *result = strchr(response, '[');
            JSON_Value *root_value = json_parse_string(result);
            JSON_Array *root_array = json_value_get_array(root_value);
            
            printf("SUCCESS: Lista colectiilor\n");
            for (int i = 0; i < json_array_get_count(root_array); i++) {
                JSON_Object *collection = json_array_get_object(root_array, i);
                int id = (int)json_object_get_number(collection, "id");
                const char *title = json_object_get_string(collection, "title");
                printf("#%d: %s\n", id, title);
            }

            json_value_free(root_value);
            free(response);

        } else if (!strcmp(command, "add_collection\n")) {

            // verific daca utilizatorul are acces la library ca sa nu mai fac request-ul
            if (token == NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
                continue;
            }

            char *title = calloc(MAX, sizeof(char));
            char *num_movies_str = calloc(MAX, sizeof(char));
            int *movie_ids = calloc(MAX, sizeof(int));

            // citesc datele de la tastatura
            printf("title=");
            fgets(title, MAX, stdin);
            printf("num_movies=");
            fgets(num_movies_str, MAX, stdin);

            title[strlen(title) - 1] = '\0';
            num_movies_str[strlen(num_movies_str) - 1] = '\0';

            // convertesc numarul de filme in int
            int num_movies = atoi(num_movies_str);

            // verific daca datele sunt valide
            if (strlen(title) == 0 || num_movies <= 0) {
                printf("ERROR: Date invalide/incomplete!\n");
                free(title);
                free(num_movies_str);
                free(movie_ids);
                continue;
            }

            // id-urile filmelor
            for (int i = 0; i < num_movies; i++) {
                char movie_id_str[MAX];
                printf("movie_id[%d]=", i);
                fgets(movie_id_str, MAX, stdin);
                movie_id_str[strcspn(movie_id_str, "\n")] = '\0';
                movie_ids[i] = atoi(movie_id_str);
            }

            JSON_Value *json_value = json_value_init_object();
            JSON_Object *json_object = json_value_get_object(json_value);
            json_object_set_string(json_object, "title", title);
            char *collection_string = json_serialize_to_string_pretty(json_value);

            // creez POST request-ul
            message = compute_post_request(
                HOST,
                COLLECTIONS,
                PAYLOAD_TYPE,
                &collection_string, 1,
                cookie, token);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            char *result = strchr(response, '{');
            JSON_Value *resp_value = json_parse_string(result);
            JSON_Object *resp_object = json_value_get_object(resp_value);
            int collection_id = (int)json_object_get_number(resp_object, "id");

            const char *resp_title = json_object_get_string(resp_object, "title");

            // POST request pentru fiecare film
            int error_occurred = 0;
            for (int i = 0; i < num_movies; i++) {
                char url[256];
                sprintf(url, "%s/%d/movies", COLLECTIONS, collection_id);

                JSON_Value *movie_val = json_value_init_object();
                JSON_Object *movie_obj = json_value_get_object(movie_val);
                json_object_set_number(movie_obj, "id", movie_ids[i]);
                char *movie_payload = json_serialize_to_string_pretty(movie_val);

                message = compute_post_request(
                    HOST,
                    url,
                    PAYLOAD_TYPE,
                    &movie_payload, 1,
                    cookie, token);

                send_to_server(sockfd, message);

                char *movie_response = receive_from_server(sockfd);

                // verific raspunsul de la server
                if (strstr(movie_response, "Invalid collection id or you do not own this collection") != NULL ||
                    strstr(movie_response, "Invalid movie id") != NULL) {
                    printf("ERROR: ID invalid!\n");
                    error_occurred = 1;
                    free(movie_response);
                    json_free_serialized_string(movie_payload);
                    json_value_free(movie_val);
                    break;
                }

                free(movie_response);
                json_free_serialized_string(movie_payload);
                json_value_free(movie_val);
            }

            // daca nu au fost erori printez titlul colectiei si username-ul logat in acel moment
            if (!error_occurred) {
                printf("SUCCESS: Colectie adaugata\n");
                printf("title: %s\n", resp_title);
                printf("owner: %s\n", logged_username);
            }

            json_free_serialized_string(collection_string);
            json_value_free(json_value);
            json_value_free(resp_value);
            free(title);
            free(num_movies_str);
            free(movie_ids);
            free(response);

        } else if (!strcmp(command, "get_collection\n")) {

            // verific daca utilizatorul are acces la library ca sa nu mai fac request-ul
            if (token == NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
                continue;
            }

            // id-ul colectiei
            char *id = get_id();

            // creez adresa URL impreuna cu id-ul colectiei
            char *url = calloc(strlen(COLLECTIONS) + strlen(id) + 2, sizeof(char));
            sprintf(url, "%s/%s", COLLECTIONS, id);

            // creez GET request-ul
            message = compute_get_request(
                HOST,
                url,
                NULL,
                cookie,
                token);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific raspunsul de la server
            if (strstr(response, "Authorization header is missing") != NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
            } else if (strstr(response, "Invalid collection id or you do not own this collection") != NULL) {
                printf("ERROR: ID invalid!\n");
            } else {
                // printez rezultatul in format JSON
                char *result = strchr(response, '{');
                JSON_Value *root_value = json_parse_string(result);
                JSON_Object *root_object = json_value_get_object(root_value);

                const char *title = json_object_get_string(root_object, "title");
                const char *owner = json_object_get_string(root_object, "owner");
                JSON_Array *movies = json_object_get_array(root_object, "movies");

                // printez titlul colectiei si owner-ul
                printf("SUCCESS: Detalii colectie\n");
                printf("title: %s\n", title);
                printf("owner: %s\n", owner);

                // printez filmele din colectie
                for (int i = 0; i < json_array_get_count(movies); i++) {
                    JSON_Object *movie = json_array_get_object(movies, i);
                    int movie_id = (int)json_object_get_number(movie, "id");
                    const char *movie_title = json_object_get_string(movie, "title");
                    printf("#%d: %s\n", movie_id, movie_title);
                }

                json_value_free(root_value);
            }

            free(id);
            free(url);
            free(response);

        } else if (!strcmp(command, "delete_collection\n")) {

            // verific daca utilizatorul are acces la library ca sa nu mai fac request-ul
            if (token == NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
                continue;
            }

            // id-ul colectiei
            char *id = get_id();
            
            // creez adresa URL impreuna cu id-ul colectiei
            char *url = calloc(strlen(COLLECTIONS) + strlen(id) + 2, sizeof(char));
            sprintf(url, "%s/%s", COLLECTIONS, id);

            // creez DELETE request-ul
            message = compute_delete_request(HOST, url, NULL, token);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific raspunsul de la server si afisez erorile daca e cazul
            if (strstr(response, "Authorization header is missing") != NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
            } else if (strstr(response, "You are not the owner of this collection") != NULL) {
                printf("ERROR: Nu sunteti owner!\n");
            } else if (strstr(response, "Invalid collection id or you do not own this collection") != NULL) {
                printf("ERROR: ID invalid!\n");
            } else {
                printf("SUCCESS: Colectie stearsa\n");
            }

            free(id);
            free(url);
            free(response);

        } else if (!strcmp(command, "add_movie_to_collection\n")) {

            // verific daca utilizatorul are acces la library ca sa nu mai fac request-ul
            if (token == NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
                continue;
            }

            char collection_id[MAX];
            char movie_id[MAX];

            // citesc datele de la tastatura
            printf("collection_id=");
            fgets(collection_id, MAX, stdin);
            printf("movie_id=");
            fgets(movie_id, MAX, stdin);

            collection_id[strcspn(collection_id, "\n")] = '\0';
            movie_id[strcspn(movie_id, "\n")] = '\0';

            // creez adresa URL impreuna cu id-ul colectiei pentru ruta de acces
            char url[512];
            sprintf(url, "%s/%s/movies", COLLECTIONS, collection_id);

            JSON_Value *movie_val = json_value_init_object();
            JSON_Object *movie_obj = json_value_get_object(movie_val);
            json_object_set_number(movie_obj, "id", atoi(movie_id));
            char *movie_payload = json_serialize_to_string_pretty(movie_val);

            // creez POST request-ul
            message = compute_post_request(
                HOST,
                url,
                PAYLOAD_TYPE,
                &movie_payload, 1,
                cookie, token);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific raspunsul de la server pentru a afisa erorile
            if (strstr(response, "Authorization header is missing") != NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
            } else if (strstr(response, "You are not the owner of this collection") != NULL) {
                printf("ERROR: Nu sunteti owner!\n");
            } else if (strstr(response, "Invalid payload") != NULL) {
                printf("ERROR: Date invalide/incomplete!\n");
            } else {
                printf("SUCCESS: Film adaugat in colectie\n");
            }

            json_free_serialized_string(movie_payload);
            json_value_free(movie_val);
            free(response);

        } else if (!strcmp(command, "delete_movie_from_collection\n")) {

            // verific daca utilizatorul are acces la library ca sa nu mai fac request-ul
            if (token == NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
                continue;
            }

            char collection_id[MAX];
            char movie_id[MAX];

            // citesc datele de la tastatura
            printf("collection_id=");
            fgets(collection_id, MAX, stdin);
            printf("movie_id=");
            fgets(movie_id, MAX, stdin);

            collection_id[strcspn(collection_id, "\n")] = '\0';
            movie_id[strcspn(movie_id, "\n")] = '\0';

            // creez adresa URL impreuna cu id-ul colectiei pentru ruta de acces
            char url[1024];
            snprintf(url, sizeof(url), "%s/%s/movies/%s", COLLECTIONS, collection_id, movie_id);

            message = compute_delete_request(HOST, url, NULL, token);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific raspunsul de la server pentru a afisa erorile
            if (strstr(response, "Authorization header is missing") != NULL) {
                printf("ERROR: Utilizatorul nu are acces la library!\n");
            } else if (strstr(response, "You are not the owner of this collection") != NULL) {
                printf("ERROR: Nu sunteti owner!\n");
            } else if (strstr(response, "id is not int") != NULL ||
                       strstr(response, "Collection not found") != NULL ||
                       strstr(response, "Movie not found in collection") != NULL) {
                printf("ERROR: ID invalid!\n");
            } else {
                printf("SUCCESS: Film sters din colectie!\n");
            }

            free(response);
        }

        else {
            printf("ERROR: Comanda invalida!\n");
        }

        // inchid conexiunea cu serverul
        close_connection(sockfd);

        free(command);
    }

    return 0;
}
