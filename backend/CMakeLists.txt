# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(PlanningPokerBackend VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Create a library from the backend source code
add_library(PlanningPokerLib
    src/WebSocketServer.cpp
    src/SessionManager.cpp
)

# Include directories
target_include_directories(PlanningPokerLib PUBLIC include)

# Find and link third-party libraries
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(SQLite3 REQUIRED)
find_package(spdlog REQUIRED)

# Link third-party libraries to the library target
target_link_libraries(PlanningPokerLib
    nlohmann_json::nlohmann_json
    ${Boost_LIBRARIES}
    SQLite::SQLite3
    spdlog::spdlog
)

# Create the main executable and link it to the library
add_executable(PlanningPokerBackend src/main.cpp)
target_link_libraries(PlanningPokerBackend PRIVATE PlanningPokerLib)

# Add tests
add_subdirectory(tests)

set(CMAKE_EXPORT_COMPILE_COMMANDS=ON)
