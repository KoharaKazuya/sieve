#!/bin/sh

while read line; do
  cond="$(echo $1 | sed -e "s/{}/$line/g")"
  if echo "$cond" | sh; then
    echo $line
  fi
done
