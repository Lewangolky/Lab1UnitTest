# Файл: tests/CMakeLists.txt

# Створюємо виконавчий файл для тестів (використовує tests.cpp)
add_executable(runTests tests.cpp)

# Пов'язуємо тести з нашою бібліотекою (RPGEngine) та gtest
target_link_libraries(runTests PRIVATE
        RPGEngine
        GTest::gtest_main  # Використовуйте сучасну ціль з ::
)

# Дозволяємо CMake автоматично виявляти тести
include(GoogleTest)
gtest_discover_tests(runTests)