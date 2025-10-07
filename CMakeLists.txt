cmake_minimum_required(VERSION 3.16)

project(M2OEP_tmaser LANGUAGES CXX)

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

add_executable(m2oep
        src/core/main.cpp
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
        src/transactions/RecurringTransaction.h
        src/transactions/TransferTransaction.cpp
        src/transactions/TransferTransaction.h
)

# Make headers available for IDEs and ensure includes work portably
target_include_directories(m2oep PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/accounts
        ${CMAKE_CURRENT_SOURCE_DIR}/src/users
        ${CMAKE_CURRENT_SOURCE_DIR}/src/io)

add_executable(m2oep_tests
        tests/test_runner.cpp
        src/accounts/Account.cpp
        src/accounts/CheckingAccount.cpp
        src/accounts/SavingsAccount.cpp
        src/accounts/CreditAccount.cpp
        src/users/User.cpp
        src/users/Budget.cpp
        src/transactions/RecurringTransaction.cpp
        src/transactions/TransferTransaction.cpp
        src/core/money.cpp
        src/core/transaction.cpp
        src/core/Ledger.cpp
        src/core/category.cpp
)

target_include_directories(m2oep_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/accounts
        ${CMAKE_CURRENT_SOURCE_DIR}/src/users
        ${CMAKE_CURRENT_SOURCE_DIR}/src/io)

# Tests
# Tests disabled for this interactive console app by default
# Uncomment to enable CTest integration
# include(CTest)
# if(BUILD_TESTING)
#   enable_testing()
#   # Add proper non-interactive tests here
# endif()
