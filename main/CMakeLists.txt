idf_component_register(
    SRCS "main.cpp"
         "encoder.c"
    INCLUDE_DIRS "."
)

component_compile_options(-std=c++20)
