# Generate lua API documentation
add_custom_target(docs
        COMMAND ldoc ${CMAKE_SOURCE_DIR}/luarecovery --dir ${CMAKE_BINARY_DIR}/docs
        COMMENT
        "Generating API documentation"
        )