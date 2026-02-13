#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stockc/http.h"

int main(void)
{
    printf("Starting stockc backend...\n");

    const char *port_env = getenv("PORT");
    int port = 8080;  // default for local development

    if (port_env && strlen(port_env) > 0) {
        port = atoi(port_env);
    }

    printf("Listening on port %d\n", port);

    return start_http_server(port);
}