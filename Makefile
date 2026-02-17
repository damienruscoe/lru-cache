
all: clean test driver

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

test:
	-echo "Building tests"
	g++ -std=c++20 tests/cache_tests.cpp -Iinclude -o run_tests -lgtest -lgtest_main -lpthread 
	-echo "Running tests"
	./run_tests

clean:
	rm -fr run_tests driver_1_single_threaded driver_2_single_threaded driver_3_multi_threaded

