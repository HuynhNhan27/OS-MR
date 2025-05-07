#!/bin/bash
case $1 in
    "run")
        for FILE in input/*; do
            if [ -f "$FILE" ]; then
                FILE_NAME=$(basename "$FILE")
                echo "Running test with input file: $FILE_NAME"
                ./os "$FILE_NAME"
                echo "----------------------------------------"
            fi
        done
        ;;
    "make")
        echo "Cleaning up..."
        make clean
        echo "Makeing all..."
        make all
        ;;
    *)
        FILE_PATH="input/$1"
        if [ ! -f "$FILE_PATH" ]; then
            echo "File not found!"
            exit 1
        else
            ./os "$1"
        fi
        ;;
esac