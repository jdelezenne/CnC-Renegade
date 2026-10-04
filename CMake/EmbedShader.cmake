file(READ "${INPUT}" shader HEX)
string(REGEX REPLACE "(..)" "0x\\1," shader "${shader}")
file(WRITE "${OUTPUT}" "#pragma once\ninline constexpr unsigned char ${SYMBOL}[] = {${shader}};\n")
