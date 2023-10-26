idf_component_register(
    SRCS "main.cpp"
         "encoder.c"
         "neowriter.cpp"
    INCLUDE_DIRS "." "../include"
)

component_compile_options(-std=c++20)
