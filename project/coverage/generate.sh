export CFLAGS="-ftest-coverage -fprofile-arcs"
export CXXFLAGS="-ftest-coverage -fprofile-arcs"
export LDFLAGS="-coverage"
make clean -C ../mpin_sdk_unit_tests
make clean -C ../mfa_sdk_unit_tests
make -C ../mpin_sdk_unit_tests
make -C ../mfa_sdk_unit_tests
lcov --zerocounters --directory ../..
../mpin_sdk_unit_tests/dist/mpin_sdk_unit_tests
../mfa_sdk_unit_tests/dist/mfa_sdk_unit_tests
mkdir -p output
lcov --capture --initial --directory ../.. -output-file output/coverage.info
lcov --no-checksum --capture --directory ../.. --output-file output/coverage.info
lcov --remove output/coverage.info "/usr/include/*" "tests/*" "ext/*" "src/json/*" "src/utf8/*" -output-file output/coverage.info
genhtml -o output/coverage -t "SDK Core Test Coverage" output/coverage.info
