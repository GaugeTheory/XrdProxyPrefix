#!/bin/bash
if
[ "$1" == "debug" ]; then
    export XRD_LOGLEVEL=Dump
fi

###Setup the test
cat > test/XrdOpenLocal.conf << EOF
lib = $PWD/XrdOpenLocal.so
redirectlocal = test.test|/tmp/
enable = true
EOF
export XRD_PLUGIN=$PWD/XrdOpenLocal.so
export XrdRedirLocDEFAULTCONF=$PWD/test/XrdOpenLocal.conf
mkdir /tmp/xrdcptest
echo "test_DEFAULT" > /tmp/xrdcptest/testfile


##Run the test
echo -e "\e[93m xrdcp a local file \e[0m"
xrdcp /tmp/xrdcptest/testfile .
echo -e "\e[93m xrdcp a file from a non existing "test.test" remote and redirecting it to local \e[0m"
xrdcp root://test.test//xrdcptest/testfile ./testfile2
DIFF=$(diff testfile testfile2)
if  [ $? -eq 0 ] && [ "$DIFF" == "" ]; then
    echo -e "\e[92m SUCCESS \e[0m"

else

    echo -e "\e[91m FAILED \e[0m"
fi
###Cleanup the test
rm -r testfile testfile2 /tmp/xrdcptest test/XrdOpenLocal.conf
