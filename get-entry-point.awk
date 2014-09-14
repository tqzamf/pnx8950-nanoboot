$3 == "_start" { printf "0x%08x", (strtonum("0x"$1) + 0x20000000) }
