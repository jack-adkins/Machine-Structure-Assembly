inputFiles=($(find . -type f -name "*.um" -printf "%f\n"))
rm UMTESTS
./unit_test
for testFile in ${inputFiles[@]}; do
    echo $testFile >> "UMTESTS"
done