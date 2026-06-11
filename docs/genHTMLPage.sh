#!/usr/bin/env bash

# pandoc -s -f markdown -t html libmeval.3.md -o libmeval-doc.html
cat libmeval.3.md | sed 's/^\s*%.*$//' | pandoc --metadata title="libmeval" -s -f markdown -t html -o libmeval-doc.html
