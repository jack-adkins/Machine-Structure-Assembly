# /****************************************************************************
#             run_tests.sh
#  *
#  * Assignment: um
#  * Authors: Jack Adkins, Seth Gellman
#  * Date: 11/17/24
#  *
#  * Summary:
#  * This shell file handles all of our testing framework, where we automate
#  * several different diff tests at the same time to ensure our um works
#  * properly.
# ****************************************************************************/

# make all of the unit tests
make
./unit_test

inputFiles=($(find . -type f -name "*.um" -printf "%f\n"))
for testFile in ${inputFiles[@]}; do
    testName=$(echo $testFile | sed -E 's/(.*).um/\1/')

    echo "Running test: $testName..."
    testOut=$testName".out"
    expectedOutput=$testName".1"
    expectedInput=$testName".0"

    if [ -f $expectedInput ]; then
        ./um $testFile < $expectedInput > $testOut
        # um $testFile < $expectedInput > $testOut
    else
        ./um $testFile > $testOut
        # um $testFile > $testOut
    fi

    if [ -f $expectedOutput ]; then
        diff $testOut $expectedOutput
    fi
done

echo "Testing completed."