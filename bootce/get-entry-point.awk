$3 == "_start" { printf "0x%08x", (or(and(strtonum("0x"$1), 0xffffffff), 0xa0000000)) }
