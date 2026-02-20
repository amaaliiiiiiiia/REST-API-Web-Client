# REST-API-Web-Client

A high-performance HTTP client developed in C, designed to interact with a REST API for managing users, movies, and collections. The project focuses on low-level network communication, manual memory management, and secure session handling via JWT and Cookies.

## Project Structure
- `client.c / client.h`: Core application logic, command parsing, and flow control.
- `helpers.c / helpers.h`: Implementation of HTTP request builders for all supported methods.
- `parson.c / parson.h`: Network abstraction layer for socket-based communication.
- `buffer.c / buffer.h`: Dynamic data structures for managing server responses.
- `requests.c / requests.h`: Third-party JSON library for C.

## Technical Features
- Full HTTP Method Support: Implemented and handled GET, POST, PUT, and DELETE requests to interact with API endpoints.
- Session & Security: Managed authentication state using Session Cookies and JWT (JSON Web Tokens) for protected routes.
- Socket Programming: Established and managed TCP connections using low-level socket operations (open_connection, send_to_server, close_connection).
- JSON Processing: Integrated and utilized the Parson library for robust JSON serialization and parsing of request bodies and server responses.
- Buffer Management: Implemented custom buffer logic for efficient data handling and parsing of server-sent messages.

## Functionalities:
The client supports a complex command-line interface, including:
- Authentication: `login`, `login_admin`, `logout`.
- User Management: `add_user`, `get_users`, `delete_user`.
- Library & Collection Management: `get_movies`, `add_movie`, `update_movie`, `delete_movie`, and collection-specific operations. 
