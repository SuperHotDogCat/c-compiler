#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"

assert 21 "5+ 20 - 4;"

assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'

assert 10 '-10+20;'
assert 10 '- -10;'
assert 10 '- - +10;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'
assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'
assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'
assert 14 'a = 3;b = 5 * 6 - 8;a + b / 2;'
assert 14 'a = 3;b = 5 * 6 - 8;return a + b / 2;'
# 制御構文
assert 0 'if (1 == 0) return 1;'
assert 1 'if (1 == 1) return 1;'
assert 1 'a=2; while (a) a = a - 1; if (a) return 1;'
assert 0 'a=1; while (a) a = a - 1; if (a) return a;'
assert 100 'a=1; while (a < 100) a = a + 1; return a;'
# assert 255 'a=1; while (a < 255) a = a + 1; return a;' なんかデフォルトだと255超えるとオーバーフローするらしいです。
echo OK
