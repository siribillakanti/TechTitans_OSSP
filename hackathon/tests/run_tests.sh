#!/bin/bash
BIN=./pagesim
pass=0; fail=0

run_case() {
  local name="$1" frames="$2" refs="$3" exp="$4" got
  got=$($BIN -q -f "$frames" -r "$refs" | sed 's/\x1b\[[0-9;]*m//g' \
        | awk '/^FIFO /{a=$2} /^LRU /{b=$2} /^OPTIMAL /{c=$2} END{print a, b, c}')
  if [ "$got" == "$exp" ]; then
    printf "PASS  %-26s expected=%-9s got=%s\n" "$name" "$exp" "$got"; pass=$((pass+1))
  else
    printf "FAIL  %-26s expected=%-9s got=%s\n" "$name" "$exp" "$got"; fail=$((fail+1))
  fi
}

run_case "T1 textbook (3 frames)"  3 "7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1" "15 12 9"
run_case "T2 Belady string, 3 fr"  3 "1 2 3 4 1 2 5 1 2 3 4 5" "9 10 7"
run_case "T3 Belady string, 4 fr"  4 "1 2 3 4 1 2 5 1 2 3 4 5" "10 8 6"
run_case "T4 all same page"        2 "1 1 1 1 1" "1 1 1"
run_case "T5 frames >= distinct"   3 "1 2 3 1 2 3 1 2 3" "3 3 3"
run_case "T6 single frame"         1 "1 2 3 4" "4 4 4"
run_case "T7 single frame repeats" 1 "1 1 2 2 3 3" "3 3 3"
run_case "T8 comma separated"      3 "1,2,3,4,1,2,5,1,2,3,4,5" "9 10 7"

ok=1
for i in 1 2 3; do
  r=$($BIN -q -n 50 -m 9 -f 4 | sed 's/\x1b\[[0-9;]*m//g' \
      | awk '/^FIFO /{a=$2} /^LRU /{b=$2} /^OPTIMAL /{c=$2} END{print (c<=a && c<=b) ? "ok":"bad"}')
  [ "$r" == "ok" ] || ok=0
  sleep 1
done
[ $ok -eq 1 ] && { echo "PASS  T9 OPT <= FIFO,LRU (random)"; pass=$((pass+1)); } || { echo "FAIL  T9"; fail=$((fail+1)); }

$BIN -f 0  >/dev/null 2>&1 && { echo "FAIL  T10 reject 0 frames"; fail=$((fail+1)); } || { echo "PASS  T10 reject 0 frames"; pass=$((pass+1)); }
$BIN -f 99 >/dev/null 2>&1 && { echo "FAIL  T11 reject 99 frames"; fail=$((fail+1)); } || { echo "PASS  T11 reject 99 frames"; pass=$((pass+1)); }
$BIN -r abc >/dev/null 2>&1 && { echo "FAIL  T12 reject bad refs"; fail=$((fail+1)); } || { echo "PASS  T12 reject bad refs"; pass=$((pass+1)); }

echo "----"; echo "Passed: $pass  Failed: $fail"
[ $fail -eq 0 ]
