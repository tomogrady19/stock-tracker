#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stockc/http.h"

int main(void)
{
    printf("Starting stockc backend...\n");

    const char *port_env = getenv("PORT");
    int port = port_env ? atoi(port_env) : 8080; // Default to 8080 if PORT is not set

    printf("Listening on port %d\n", port);

    return start_http_server(port);
}