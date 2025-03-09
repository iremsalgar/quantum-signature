cmake_minimum_required(VERSION 3.10)
project(quantum-signature)

# OpenSSL'i bul
find_package(OpenSSL REQUIRED)

# LIBOQS BAŞLIK VE KÜTÜPHANE YOLLARI
include_directories(/home/isalgar/my_project_repo/liboqs/build/install/include)
link_directories(/home/isalgar/my_project_repo/liboqs/build/install/lib)

# EXECUTABLE
add_executable(quantum-signature src/main.c)

# KÜTÜPHANE BAĞLANTILARI (LIBOQS + OPENSSL)
target_link_libraries(quantum-signature PRIVATE oqs OpenSSL::SSL OpenSSL::Crypto)
