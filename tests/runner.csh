#!/bin/csh

# Get the script path and set variables
set script_path = `realpath $0 | xargs dirname`
set toyccomp = "../ToyCComp"

# Navigate to the script directory
cd $script_path

# Initialize variables
set specific_test = ""
set rerun_failed = 0
set failed_tests_file = "failed_tests.log"

# Parse command-line arguments
while ($#argv > 0)
    switch ($argv[1])
        case -t:
            if ($#argv < 2) then
                echo "Error: -t requires a test name"
                exit 1
            endif
            set specific_test = $argv[2]
            shift
            shift
            breaksw
        case -lf:
            set rerun_failed = 1
            shift
            breaksw
        default:
            echo "Usage: $0 [-t test_name] [-lf]"
            exit 1
    endsw
end

# If -lf is used, read failed tests from the log file
if ($rerun_failed) then
    if (! -e $failed_tests_file) then
        echo "No failed tests log found"
        exit 1
    endif
    set test_files = `cat $failed_tests_file`
else if ("$specific_test" != "") then
    # If -t is used, set the specific test
    set test_files = "$specific_test"
else
    # Otherwise, find all test files
    set test_files = (`find $PWD -type f -name 'test*c'`)
endif

# Initialize counters and lists
set passed_count = 0
set failed_count = 0
set failed_tests = ""

# Clear the failed tests log
echo -n > $failed_tests_file

# Run tests
foreach file ($test_files)
    set test_name = `echo $file | cut -d '/' -f6-`

    # Run ToyCComp and capture logs
    $toyccomp $file > log 2> err
    if ($status != 0) then
        echo "--> ToyCComp failed for $test_name"
        cat err
        @ failed_count++
        echo $file >> $failed_tests_file
        set failed_tests = "$failed_tests\t- $test_name\n"
        continue
    endif

    # Assemble and compile
    nasm -f elf64 out.s -o out.o
    if ($status != 0) then
        echo "--> NASM assembly failed for $test_name"
        @ failed_count++
        echo $file >> $failed_tests_file
        set failed_tests = "$failed_tests\t- $test_name\n"
        continue
    endif

    gcc -no-pie -o out ../lib/print.c out.o
    if ($status != 0) then
        echo "--> GCC compilation failed for $test_name"
        @ failed_count++
        echo $file >> $failed_tests_file
        set failed_tests = "$failed_tests    - $test_name\n"
        continue
    endif

    # Run the executable and capture results
    ./out > res
    if ($status != 0) then
        echo "--> Test execution failed for $test_name"
        @ failed_count++
        echo $file >> $failed_tests_file
        set failed_tests = "$failed_tests    - $test_name\n"
        continue
    endif

    # Compare results
    diff res ref/$test_name.ref > /dev/null
    if ($status != 0) then
        echo "--> Test $test_name failed"
        @ failed_count++
        echo $file >> $failed_tests_file
        set failed_tests = "$failed_tests    - $test_name\n"
    else
        echo "--> Test $test_name passed"
        @ passed_count++
    endif

    # Clean up intermediate files
    rm -f out.s out out.o err
end

# Clean up intermediate files
rm -f out.s out out.o err log res

# Print summary
echo ""
echo "Summary:" | tee test_summary.log
echo "    Passed: $passed_count" | tee -a test_summary.log
echo "    Failed: $failed_count" | tee -a test_summary.log

if ($failed_count > 0) then
    echo "" | tee -a test_summary.log
    echo "Failed tests:" | tee -a test_summary.log
    printf "$failed_tests" | tee -a test_summary.log
endif

# Exit with the appropriate code
exit $failed_count
