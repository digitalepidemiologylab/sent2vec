cmake_minimum_required(VERSION 3.8)
project(sent2vec)

set(CMAKE_CXX_STANDARD 11)


link_directories("/usr/local/include/cpp_redis/")
link_directories("/usr/local/include/tacopie")


set(SOURCE_FILES
        src/args.cc
        src/args.h
        src/dictionary.cc
        src/dictionary.h
        src/fasttext.cc
        src/fasttext.h
        src/main.cc
        src/matrix.cc
        src/matrix.h
        src/model.cc
        src/model.h
        src/productquantizer.cc
        src/productquantizer.h
        src/qmatrix.cc
        src/qmatrix.h
        src/real.h
        src/utils.cc
        src/utils.h
        src/vector.cc
        src/vector.h
        lib/json.hpp)


add_executable(sent2vec ${SOURCE_FILES})

target_link_libraries(sent2vec cpp_redis tacopie)

