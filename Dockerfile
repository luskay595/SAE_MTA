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
        echo "Utilisateur $user ajouté."; \
    done

