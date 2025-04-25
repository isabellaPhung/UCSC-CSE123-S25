// Simple test script to pull JSON and push it to database
#include <esp_log.h>

static char *buffer;

int DB_task(void *pvParameters)
{
    buffer = (char*)pvParameters;
    
}