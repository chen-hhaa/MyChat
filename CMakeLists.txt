cmake_minimum_required(VERSION 2.6)

project(CHATSERVER)

include_directories(include)

link_directories(
    ${PROJECT_SOURCE_DIR}/lib
	/usr/lib64/mysql/
)

find_library(MYSQL_LIB libmysqlclient.so /usr/lib64/mysql/)

add_compile_options(--std=c++11)

add_executable(ChatServer server.cpp src/UserManager.cpp src/ConfigReader.cpp)

TARGET_LINK_LIBRARIES(ChatServer mysqlclient)