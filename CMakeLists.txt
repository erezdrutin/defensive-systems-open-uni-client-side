cmake_minimum_required(VERSION 3.21)
project(defensive_maman_15)

set(CMAKE_CXX_STANDARD 17)
set(CRYPTO++_INCLUDE_DIR "/usr/local/Cellar/cryptopp/8.9.0/include")
set(CRYPTO++_LIBRARY_DIR "/usr/local/Cellar/cryptopp/8.9.0/lib")
set(CRYPTO++_LIBRARY_NAME "cryptopp")

include_directories(${CRYPTO++_INCLUDE_DIR})
link_directories(${CRYPTO++_LIBRARY_DIR})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mrdrnd")

add_executable(defensive_maman_15 main.cpp RSAWrapper.cpp RSAWrapper.h Base64Wrapper.cpp Base64Wrapper.h AESWrapper.cpp AESWrapper.h CryptoHandler.cpp CryptoHandler.h checksum.cpp FileHandler.cpp FileHandler.h ProtocolHandler.cpp ProtocolHandler.h Logger.cpp Logger.h)
target_link_libraries(defensive_maman_15 ${CRYPTO++_LIBRARY_NAME})
