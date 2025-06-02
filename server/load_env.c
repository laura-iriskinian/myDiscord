#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void load_env(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening .env");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // ignore comments or empty lines
        if (line[0] == '#' || line[0] == '\n')
            continue;

        // Remove the \n at the end
        line[strcspn(line, "\n")] = 0;

        char *equals = strchr(line, '=');
        if (!equals) continue;  
        *equals = '\0';

        const char *key = line;
        const char *value = equals + 1;

        setenv(key, value, 1); 
    }
    fclose(file);
}
