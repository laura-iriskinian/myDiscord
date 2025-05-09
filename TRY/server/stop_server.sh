#!/bin/bash

# Script pour arrêter le serveur MyDiscord

PID_FILE="/tmp/mydiscord_server.pid"

if [ -f "$PID_FILE" ]; then
    PID=$(cat "$PID_FILE")
    if ps -p $PID > /dev/null; then
        echo "Arrêt du serveur MyDiscord (PID: $PID)"
        kill $PID
        # Attendre que le processus se termine
        for i in {1..5}; do
            if ! ps -p $PID > /dev/null; then
                echo "Serveur arrêté."
                rm -f "$PID_FILE"
                exit 0
            fi
            sleep 1
        done
        # Si le serveur ne s'arrête pas après 5 secondes, forcer l'arrêt
        echo "Le serveur ne répond pas, arrêt forcé."
        kill -9 $PID
        rm -f "$PID_FILE"
    else
        echo "Le fichier PID existe mais le processus n'est pas en cours d'exécution."
        rm -f "$PID_FILE"
    fi
else
    echo "Serveur MyDiscord non trouvé (pas de fichier PID)."
    # Chercher quand même les processus correspondants
    PIDS=$(ps aux | grep 'server' | grep -v grep | grep -v stop_server | awk '{print $2}')
    if [ -n "$PIDS" ]; then
        echo "Processus serveur trouvés. Tentative d'arrêt..."
        for PID in $PIDS; do
            echo "Arrêt du processus $PID"
            kill $PID
        done
    fi
fi

# Vérifier s'il y a des processus bloquant le port 8080
PORT_PROC=$(lsof -i:8080 -t)
if [ -n "$PORT_PROC" ]; then
    echo "Processus utilisant le port 8080 trouvés. Tentative d'arrêt..."
    for PID in $PORT_PROC; do
        echo "Arrêt du processus $PID utilisant le port 8080"
        kill $PID
    done
fi

echo "Opération terminée."