# helper script to compile, run and test
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j && ctest --test-dir build --output-on-failure && ./build/byoredis "$@"
