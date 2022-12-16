CLANG_FORMAT_VERSION=clang-format-12
INSTALL_FORMATER="sudo apt-get install -f $CLANG_FORMAT_VERSION"

command -v $CLANG_FORMAT_VERSION >/dev/null 2>&1 || { echo >&2 "$CLANG_FORMAT_VERSION is not installed. Use following: $INSTALL_FORMATER"; exit 1; }

if [ "$1" = "--help" ]
then    
    echo "C++ code style check in all .cpp and .h files"
    echo "Uses $CLANG_FORMAT_VERSION"
    echo "Usage: `basename $0` [option]"
    echo " --fix    Style fix files"
    echo " --help   Information how to use"
    exit 0
fi

FILTER=$(find src -name \*.h -print -o -name \*.cpp -print)

style_fix() {
	$CLANG_FORMAT_VERSION -style=file -i $1
}

style_check() {
	$CLANG_FORMAT_VERSION -style=file $1 | diff $1 -
}

if [ "$1" = "--fix" ]
then
  for FILE_NAME in $FILTER; do style_fix $FILE_NAME; done
else
  PASSED=0
  for FILE_NAME in $FILTER; do
    style_check $FILE_NAME
    if [ $? != 0 ]
    then
      echo "in " $FILE_NAME 
      PASSED=1
    fi
  done
  if [ $PASSED = 1 ]
  then 
    exit 1
  fi
fi
