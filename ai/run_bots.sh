#!/bin/bash

# Recorremos la lista de equipos
for equipo in "TeamAI" "TeamEnemigo1"; do

    # Para cada equipo, lanzamos 5 bots
    for i in {1..5}; do
        python3 src/main.py -p 8080 -n $equipo -ip 127.0.0.1 --ai &
    done

done

echo "¡10 bots lanzados (5 por equipo) y corriendo en segundo plano!"