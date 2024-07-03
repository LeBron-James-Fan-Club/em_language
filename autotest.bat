echo Compiling files
make
echo Running tests
echo Test 1
./bin/a.exe  ./tests/test1.txt ./tests/test1_out.s
./tests/mipsy.exe ./tests/test1_out.s > ./tests/test1_out.txt
echo Test 2
./bin/a.exe  ./tests/test2.txt ./tests/test2_out.s
./tests/mipsy.exe ./tests/test2_out.s > ./tests/test2_out.txt
echo Test 3
./bin/a.exe  ./tests/test3.txt ./tests/test3_out.s
./tests/mipsy.exe ./tests/test3_out.s > ./tests/test3_out.txt
echo Test 4
./bin/a.exe  ./tests/test4.txt ./tests/test4_out.s
./tests/mipsy.exe ./tests/test4_out.s > ./tests/test4_out.txt
echo Test 5
./bin/a.exe  ./tests/test5.txt ./tests/test5_out.s
./tests/mipsy.exe ./tests/test5_out.s > ./tests/test5_out.txt
echo Test 6
./bin/a.exe  ./tests/test6.txt ./tests/test6_out.s
./tests/mipsy.exe ./tests/test6_out.s > ./tests/test6_out.txt
echo Test 7
./bin/a.exe  ./tests/test7.txt ./tests/test7_out.s
./tests/mipsy.exe ./tests/test7_out.s > ./tests/test7_out.txt
echo Test 8
./bin/a.exe  ./tests/test8.txt ./tests/test8_out.s
./tests/mipsy.exe ./tests/test8_out.s > ./tests/test8_out.txt
