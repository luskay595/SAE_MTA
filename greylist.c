#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libmemcached/memcached.h>
#include <time.h>

#define MEMCACHED_SERVER "localhost"
#define MEMCACHED_PORT 11211
#define GREYLIST_DELAY 300 // 5 minutes

void log_error(const char *message) {
    fprintf(stderr, "%s\n", message);
}

void log_info(const char *ip, const char *recipient) {
    FILE *log_file = fopen("/var/log/greylist.log", "a");
    if (log_file == NULL) {
        log_error("Impossible d'ouvrir le fichier de log");
        return;
    }

    fprintf(log_file, "IP: %s, Recipient: %s\n", ip, recipient);
    fclose(log_file);
}

void check_greylist(const char *ip, const char *recipient) {
    memcached_server_st *servers = NULL;
    memcached_st *memc;
    memcached_return rc;
    char key[256];
    size_t key_length;
    char *value;
    size_t value_length;
    uint32_t flags;
    time_t now = time(NULL);

    snprintf(key, sizeof(key), "%s|%s", ip, recipient);
    key_length = strlen(key);

    memc = memcached_create(NULL);
    servers = memcached_server_list_append(servers, MEMCACHED_SERVER, MEMCACHED_PORT, &rc);
    rc = memcached_server_push(memc, servers);
    if (rc != MEMCACHED_SUCCESS) {
        log_error("Erreur de connexion à Memcached");
        memcached_free(memc);
        exit(1);
    }

    value = memcached_get(memc, key, key_length, &value_length, &flags, &rc);
    if (rc == MEMCACHED_SUCCESS) {
        time_t timestamp = (time_t) strtol(value, NULL, 10);
        if (now - timestamp >= GREYLIST_DELAY) {
            printf("L'email peut être accepté.\n");
            memcached_delete(memc, key, key_length, (time_t) 0);
        } else {
            printf("L'email est encore en période de greylisting.\n");
        }
        free(value);
    } else if (rc == MEMCACHED_NOTFOUND) {
        printf("Ajout de l'email à la greylist.\n");
        char timestamp_str[20];
        snprintf(timestamp_str, sizeof(timestamp_str), "%ld", now);
        rc = memcached_set(memc, key, key_length, timestamp_str, strlen(timestamp_str), (time_t) (GREYLIST_DELAY * 2), (uint32_t) 0);
        if (rc != MEMCACHED_SUCCESS) {
            log_error("Erreur lors de l'ajout à la greylist");
        }
    } else {
        log_error("Erreur lors de la récupération de la greylist");
    }

    memcached_free(memc);
}

int main() {
    char line[1024];
    char ip[256];
    char recipient[256];

    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (strncmp(line, "client_address=", 15) == 0) {
            strncpy(ip, line + 15, sizeof(ip));
            ip[strcspn(ip, "\n")] = 0; 
        }
        else if (strncmp(line, "recipient=", 10) == 0) {
            strncpy(recipient, line + 10, sizeof(recipient));
            recipient[strcspn(recipient, "\n")] = 0;
        }
    }
    log_info(ip, recipient);
    check_greylist(ip, recipient);

    return 0;
}

