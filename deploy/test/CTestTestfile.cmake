# CMake generated Testfile for 
# Source directory: /home/cesar/Documents/Studia/PW/zal2/test
# Build directory: /home/cesar/Documents/Studia/PW/zal2/deploy/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_defer "test_defer")
set_tests_properties(test_defer PROPERTIES  TIMEOUT "1")
add_test(test_await "test_await")
set_tests_properties(test_await PROPERTIES  TIMEOUT "1")
add_test(test_macierzy "macierz.sh" "1")
add_test(test_silni "silnia.sh" "1")
