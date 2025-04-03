cmake_minimum_required(VERSION 3.5)

project(06.allocator)

# Указываем, что требуется библиотека Boost
find_package(Boost 1.86.0 REQUIRED)
#Подключаем директивы для использования Boost
include_directories(${Boost_INCLUDE_DIR})

option(WITH_BOOST_TEST "Whether to build Boost test" ON)

# add_executable(02_logging_allocator logging_allocator.cpp)
# set_target_properties(02_logging_allocator PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

# add_executable(01_std_03_allocator simple_allocator.cpp)
# set_target_properties(01_std_03_allocator PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

# add_executable(00_new_overload new_overload.cpp)
# set_target_properties(00_new_overload PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

# add_executable(03_allocator_traits allocator_traits.cpp)
# set_target_properties(03_allocator_traits PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

# add_executable(04_polymorphism polymorphism.cpp)
# set_target_properties(04_polymorphism PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

# add_executable(05_polymorphic_allocator polymorphic_allocator.cpp)
# set_target_properties(05_polymorphic_allocator PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

add_executable(my_boost_pool_alloc my_boost_pool_alloc.cpp)
set_target_properties(my_boost_pool_alloc PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

