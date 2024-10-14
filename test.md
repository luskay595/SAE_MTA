
# Configuring Keycloak with Nginx and SSL

This document describes the configuration of a Keycloak server with Nginx, SSL and MariaDB as database. It also includes a configuration for starting the Keycloak service on the system.

## Prerequisites

- Keycloak must be installed on the server.
- A valid SSL certificate is required to secure communications with Keycloak.
- MariaDB is used as the database for Keycloak.

## Nginx configuration

### HTTP to HTTPS redirection

The first configuration block in the Nginx file redirects all HTTP traffic (port 80) to HTTPS (port 443) to ensure that all connections to Keycloak are secure.```nginx
server {
    listen 80;  # Listening on port 80
    server_name keycloak.example.org;

    # Redirects all HTTP requests to HTTPS
    return 301 https://$host$request_uri;
}
```- **listen 80**: The Nginx server listens on HTTP port 80.
- **server_name**: The domain name used to access Keycloak.
- **return 301**: Permanent redirection to the HTTPS equivalent of the requested URL.

### HTTPS configuration with SSL

The second configuration block handles HTTPS connections via port 443 and uses an SSL certificate to secure communications.```nginx
server {
    listen 443 ssl;  # Listens on port 443 for SSL connections
    server_name keycloak.example.org;  # Replace with your domain name

    ssl_certificate /etc/keycloak/certs/keycloak.pem;
    ssl_certificate_key /etc/keycloak/certs/keycloak-key.pem;
    access_log /var/log/nginx/keycloak-access.log;
    error_log /var/log/nginx/keycloak-error.log;
    
    location / {
        proxy_pass https://localhost:8443; # Redirects to port 8443
        proxy_set_header Host $host;         # Defines Host header
        proxy_set_header X-Real-IP $remote_addr;  # Sets the real IP address
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;  # Adds the original IP
        proxy_set_header X-Forwarded-Proto https;  # Indicates original protocol
    }
}
```- **listen 443 ssl**: Nginx listens on port 443 for SSL-secured connections.
- **ssl_certificate**: The path to the SSL certificate file used to encrypt connections.
- **ssl_certificate_key**: The path to the private key associated with the SSL certificate.
- **proxy_pass**: Redirects requests to Keycloak, which operates on port 8443 (Keycloak configuration).
- proxy_set_header**: These directives pass certain information from the original client to Keycloak, such as IP address and protocol (HTTP/HTTPS).

### Log files

- **keycloak-access** and **keycloak-error.log**: These files log Nginx server accesses and errors for Keycloak, useful for activity monitoring and troubleshooting.

---

## Keycloak configuration

Keycloak configuration is modified to use HTTPS port 8443 and MariaDB as database.

### Basic settings```bash
https-port=8443
hostname=keycloak.example.org
https-certificate-file=/etc/keycloak/certs/keycloak.pem
https-certificate-key-file=/etc/keycloak/certs/keycloak-key.pem
```- **https-port**: The port on which Keycloak will listen for HTTPS connections (internal port 8443).
- **hostname**: The hostname under which Keycloak can be accessed (must match the one configured in Nginx).
- **https-certificate-file** and **https-certificate-key-file**: Paths to certificate and key files for securing HTTPS connections.

### Database configuration

Keycloak is configured to use an external MariaDB database with the following parameters:```bash
db=mariadb
db-url=jdbc:mariadb://192.168.253.143:3306/keycloak_db
db-username=keycloak_user
db-password=admin123
```- **db**: Database type (MariaDB in this case).
- **db-url**: JDBC connection URL to the MariaDB database (includes server IP address and database name).
- **db-username** and **db-password**: Identifiers used to connect to the database.

---

## Service Keycloak

To manage Keycloak as a service under Linux, the following configuration file is used for automatic server startup.```ini
[Unit]
Description=Keycloak Service
After=network.target

[Service]
User=keycloak
Group=keycloak
ExecStart=/opt/keycloak/bin/kc.sh start
Restart=on-failure

[Install]
WantedBy=multi-user.target
```- Description**: Service description.
- After**: Indicates that Keycloak starts after the network is active.
- **User** and **Group**: The user and group under which the Keycloak service is run (must have the correct permissions).
- ExecStart**: Command to start Keycloak.
- Restart**: Automatically restarts Keycloak in the event of a failure.
- WantedBy**: Tells systemd that the service must be enabled for multi-user mode.

---

## Remarks

- Security**: Ensure that SSL certificates and database information are protected and accessible only to authorized users.
- **Logs** : Log files (Nginx and Keycloak) are essential for identifying any configuration or performance problems.
- Nginx proxy**: Nginx is used here as a reverse proxy to redirect user requests to Keycloak, while providing an additional layer of security and auditing.
