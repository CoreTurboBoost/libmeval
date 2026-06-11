#!/usr/bin/env bash

# pandoc --standalone --from markdown --to man libmeval.3.md -o libmeval.3
pandoc -s -f markdown -t man libmeval.3.md -o libmeval.3
