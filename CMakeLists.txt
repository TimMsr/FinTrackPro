cmake_minimum_required(VERSION 3.16)

project(FinTrackPro LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warnings
if(MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(Threads REQUIRED)

# Core domain logic shared by the terminal app, the API server and the tests
add_library(fintrack_core STATIC
        src/core/money.cpp
        src/core/transaction.cpp
        src/core/Ledger.cpp
        src/core/category.cpp
        src/utility.cpp
        src/accounts/Account.cpp
        src/accounts/CheckingAccount.cpp
        src/accounts/SavingsAccount.cpp
        src/accounts/CreditAccount.cpp
        src/users/User.cpp
        src/users/Budget.cpp
        src/io/CsvHandler.cpp
        src/io/Validation.cpp
        src/transactions/RecurringTransaction.cpp
        src/transactions/TransferTransaction.cpp
)

target_include_directories(fintrack_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/accounts
        ${CMAKE_CURRENT_SOURCE_DIR}/src/users
        ${CMAKE_CURRENT_SOURCE_DIR}/src/io
        ${CMAKE_CURRENT_SOURCE_DIR}/src/transactions)

# Interactive terminal client
add_executable(fintrack src/core/main.cpp)
target_link_libraries(fintrack PRIVATE fintrack_core)

# JSON API server used by the web dashboard
add_executable(fintrack_server src/server/main.cpp src/server/Store.cpp)
target_include_directories(fintrack_server SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
target_link_libraries(fintrack_server PRIVATE fintrack_core Threads::Threads)

# Unit tests
add_executable(fintrack_tests tests/test_runner.cpp)
target_link_libraries(fintrack_tests PRIVATE fintrack_core)

enable_testing()
add_test(NAME unit COMMAND fintrack_tests)
