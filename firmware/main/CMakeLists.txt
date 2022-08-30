file(GLOB_RECURSE SOURCES *.c)
file(GLOB_RECURSE HEADERS *.h)
set(COMPONENT_REQUIRES lwip mbedtls)
string(REGEX REPLACE "\/[a-zA-Z0-9_]*\.h" "" HEADER_DIRECTORIES "${HEADERS}")
idf_component_register(SRCS ${SOURCES}
INCLUDE_DIRS ${HEADER_DIRECTORIES}
EMBED_FILES server/error.html server/favicon.ico server/test.css server/test.js server/root.html)