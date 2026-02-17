
all: clean test_all driver

driver:
	-echo "Building sample driver examples"
	g++ --std=c++20 src/single_threaded_driver.cpp src/dummy_product_db.cpp -Iinclude -o driver_1_single_threaded
	g++ --std=c++20 src/single_threaded_driver_v2.cpp src/dummy_product_db.cpp -Iinclude -o driver_2_single_threaded
	g++ --std=c++20 src/multi_threaded_driver.cpp src/dummy_product_db.cpp -Iinclude -o driver_3_multi_threaded
	-echo "Running sample driver example"
	./driver_1_single_threaded
	./driver_2_single_threaded
	./driver_3_multi_threaded

format:
	-echo "Formatting code in place"
	clang-format -i src/*.cpp src/*.hpp tests/*.cpp include/*.hpp

test_all: test test_asan test_tsan

test:
	-echo "Building tests"
	g++ -std=c++20 tests/cache_tests.cpp -Iinclude -o run_tests -lgtest -lgtest_main -lpthread 
	-echo "Running tests"
	./run_tests

test_tsan:
	-echo "Building threaded tests"
	g++ -std=c++20 -fsanitize=thread tests/cache_thread_tests.cpp -Iinclude -o run_tsan_tests -lgtest -lgtest_main -lpthread 
	-echo "Running thread tests"
	./run_tsan_tests

test_asan:
	-echo "Building threaded tests"
	g++ -std=c++20 -fsanitize=address,undefined -g tests/cache_thread_tests.cpp -Iinclude -o run_asan_tests -lgtest -lgtest_main -lpthread 
	-echo "Running thread tests"
	./run_asan_tests

clean:
	rm -fr run_tests run_asan_tests run_tsan_tests driver_1_single_threaded driver_2_single_threaded driver_3_multi_threaded

