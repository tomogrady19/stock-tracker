#include <stdio.h>
#include "stockc/http.h"

int main(void)
{
    printf("Starting stockc backend...\n");
    return start_http_server(8080);
}
