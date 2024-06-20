FROM phplist/postfix

# Mettre à jour le système et installer mailutils
RUN apt-get update && apt-get install -y mailutils

# Ajouter les utilisateurs lors de la construction de l'image
RUN for user in alexandre robin lucas axel massi logan; do \
        useradd -m $user && \
        echo "Utilisateur $user ajouté."; \
    done


