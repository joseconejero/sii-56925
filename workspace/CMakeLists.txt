CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT (TENIS)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
find_package(Threads REQUIRED)
SET(COMMON_SRCS
    src/MundoCliente.cpp
    src/Esfera.cpp
    src/Plano.cpp
    src/Raqueta.cpp
    src/Vector2D.cpp
)
SET(COMMON_SRCS_1
    src/MundoServidor.cpp
    src/Esfera.cpp
    src/Plano.cpp
    src/Raqueta.cpp
    src/Vector2D.cpp
)

add_executable(servidor src/servidor.cpp ${COMMON_SRCS_1})
target_link_libraries(servidor glut GL GLU Threads::Threads)

ADD_EXECUTABLE(cliente src/cliente.cpp ${COMMON_SRCS})
ADD_EXECUTABLE(bot src/bot.cpp)
TARGET_LINK_LIBRARIES(cliente glut GL GLU)
