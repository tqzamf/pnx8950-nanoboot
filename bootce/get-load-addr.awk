$3 == "__image_start" { printf "0x%08x", (or(strtonum("0x" substr($1,9,8)), 0x80000000)) }
