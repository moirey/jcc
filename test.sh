#!/bin/bash
try() {
    expected="$1"
    input="$2"

    ./jcc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

                if [ "$actual" = "$expected" ]; then
                      echo "$input => $actual"
                        else
                              echo "$input => $expected expected, but got $actual"
                                  exit 1
                                    fi
                                  }

                                  try 21 '5+20-4'
                                  try 42 42

                                  echo OK
