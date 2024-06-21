# Postfix with Greylisting and Memcached

This project sets up a Postfix mail server with greylisting functionality using Memcached. The greylisting logic is implemented in a custom C program that integrates with Postfix.

## Prerequisites

- Docker
- Docker Compose

## Installation

1. **Clone the repository**:
   ```sh
   git clone <repository_url>
   cd <repository_directory>
   ```

2. **Prepare the Docker environment**:
   ```sh
   docker-compose up --build
   ```

## Docker Setup

The `Dockerfile` and `docker-compose.yml` files configure a Postfix server and a Memcached instance. Below are the key components:

### Dockerfile

The `Dockerfile` installs necessary packages, sets up Memcached, compiles the greylisting C program, and adds user accounts.

```dockerfile
FROM phplist/postfix

# Install basic packages
RUN apt-get update \
    && apt-get install -y python3 python3-pip mailutils \
    && apt-get clean

# Install python-memcached
RUN pip3 install python-memcached --index-url=https://pypi.org/simple/

# Install gcc and libmemcached-dev
RUN apt-get update \
    && apt-get install -y gcc libmemcached-dev \
    && apt-get clean

# Copy and compile greylist.c
COPY greylist.c /usr/local/bin/greylist.c

RUN gcc -o /usr/local/bin/greylist /usr/local/bin/greylist.c -lmemcached \
    && chmod +x /usr/local/bin/greylist

# Add users
RUN for user in alexandre robin lucas axel massi logan; do \
        useradd -m $user && \
        echo "User $user added."; \
    done
```

### docker-compose.yml

The `docker-compose.yml` file defines two services: `postfix` and `memcached`.

```yaml
version: '3'

services:
  postfix:
    build: .
    hostname: mail.example.com  # Hostname matching DNS configuration
    volumes:
      - ./postfix-main.cf:/etc/postfix/main.cf
      - ./postfix-master.cf:/etc/postfix/master.cf
    environment:
      MAIL_DOMAIN: example.com  # Mail domain matching DNS configuration
    network_mode: host
    dns:
      - 100.64.85.13  # Local DNS server IP address

  memcached:
    image: memcached:alpine
    ports:
      - "11211:11211"
```

## Greylist Program

The greylisting logic is implemented in `greylist.c`. This program checks if an incoming email should be temporarily rejected (greylisted) and logs the necessary information.

### greylist.c

```c
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
        log_error("Cannot open log file");
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
        log_error("Error connecting to Memcached");
        memcached_free(memc);
        exit(1);
    }

    value = memcached_get(memc, key, key_length, &value_length, &flags, &rc);
    if (rc == MEMCACHED_SUCCESS) {
        time_t timestamp = (time_t) strtol(value, NULL, 10);
        if (now - timestamp >= GREYLIST_DELAY) {
            printf("Email can be accepted.\n");
            memcached_delete(memc, key, key_length, (time_t) 0);
        } else {
            printf("Email is still greylisted.\n");
        }
        free(value);
    } else if (rc == MEMCACHED_NOTFOUND) {
        printf("Adding email to greylist.\n");
        char timestamp_str[20];
        snprintf(timestamp_str, sizeof(timestamp_str), "%ld", now);
        rc = memcached_set(memc, key, key_length, timestamp_str, strlen(timestamp_str), (time_t) (GREYLIST_DELAY * 2), (uint32_t) 0);
        if (rc != MEMCACHED_SUCCESS) {
            log_error("Error adding to greylist");
        }
    } else {
        log_error("Error retrieving from greylist");
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
        } else if (strncmp(line, "recipient=", 10) == 0) {
            strncpy(recipient, line + 10, sizeof(recipient));
            recipient[strcspn(recipient, "\n")] = 0;
        }
    }

    log_info(ip, recipient);
    check_greylist(ip, recipient);

    return 0;
}
```

## Postfix Configuration

### main.cf

The `postfix-main.cf` file configures the main Postfix settings.

```sh
# Postfix server hostname
myhostname = mail.example.com
# Local domain
mydomain = example.com
# Local networks
mynetworks = 127.0.0.0/8 [::ffff:127.0.0.0]/104 [::1]/128 100.64.85.0/24
# Local mailbox settings
mailbox_command =
mailbox_size_limit = 0
recipient_delimiter = +
inet_interfaces = all
inet_protocols = all
disable_dns_lookups = yes
# Greylisting policy service
smtpd_recipient_restrictions =
    permit_mynetworks
    reject_unauth_destination
    check_policy_service unix:private/greylist
```

### master.cf

The `postfix-master.cf` file configures Postfix services, including the greylisting service.

```sh
smtp      inet  n       -       n       -       -       smtpd
pickup    fifo  n       -       -       60      1       pickup
cleanup   unix  n       -       -       -       0       cleanup
qmgr      fifo  n       -       n       300     1       qmgr
rewrite   unix  -       -       -       -       -       trivial-rewrite
bounce    unix  -       -       -       -       0       bounce
defer     unix  -       -       -       -       0       bounce
trace     unix  -       -       -       -       0       bounce
verify    unix  -       -       -       -       1       verify
flush     unix  n       -       -       1000?   0       flush
proxymap  unix  -       -       n       -       -       proxymap
smtp      unix  -       -       -       -       -       smtp
relay     unix  -       -       -       -       -       smtp
showq     unix  n       -       -       -       -       showq
error     unix  -       -       -       -       -       error
retry     unix  -       -       -       -       -       error
discard   unix  -       -       -       -       -       discard
local     unix  -       n       n       -       -       local
virtual   unix  -       n       n       -       -       virtual
lmtp      unix  -       -       -       -       -       lmtp
anvil     unix  -       -       -       -       1       anvil
scache    unix  -       -       -       -       1       scache

# Greylisting service
greylist  unix  -       n       n       -       0       spawn
  user=nobody argv=/usr/local/bin/greylist
```
