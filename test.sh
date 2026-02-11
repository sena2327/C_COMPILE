#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  x86_64-linux-gnu-gcc -static -o tmp tmp.s
  qemu-x86_64 ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
  fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5; "
assert 20 "10 + f + 10;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 1 " 1 < 2;"
assert 0 "1 >= 30;"
assert 1 "1 == 1;"
assert 1 "1 != 2;"
assert 1 "a = 1;"
assert 1 "abs = 1;"
assert 1 "abs = 1;return abs;"
assert 1 "foo = 1;hoo = 2;return foo;"
assert 1 "return 1;"
assert 0 "if(1 == 1) 0;else 1;"
assert 1 "if(1 == 2) 0;else 1;"

echo OK