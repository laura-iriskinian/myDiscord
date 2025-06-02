#!/bin/bash

# Script pour libérer le port 8080 utilisé par MyDiscord

echo "Recherche des processus utilisant le port 8080..."

# Trouver tous les processus utilisant le port 8080
PORT_PROC=$(lsof -i:8080 -t)

if [ -z "$PORT_PROC" ]; then
    echo "Aucun processus n'utilise le port 8080."
    exit 0
fi

echo "Processus trouvés: $PORT_PROC"

# Tenter d'arrêter proprement les processus
for PID in $PORT_PROC; do
    echo "Tentative d'arrêt du processus $PID..."
    kill $PID
done

# Attendre un peu pour voir si les processus se terminent
sleep 2

# Vérifier à nouveau s'il reste des processus
REMAINING=$(lsof -i:8080 -t)
if [ -n "$REMAINING" ]; then
    echo "Certains processus ne répondent pas. Arrêt forcé..."
    for PID in $REMAINING; do
        echo "Arrêt forcé du processus $PID"
        kill -9 $PID
    done
fi

echo "Le port 8080 devrait maintenant être libre."

# Supprimer le fichier PID s'il existe
if [ -f "/tmp/mydiscord_server.pid" ]; then
    rm -f "/tmp/mydiscord_server.pid"
    echo "Fichier PID supprimé."
fi

echo "Opération terminée."