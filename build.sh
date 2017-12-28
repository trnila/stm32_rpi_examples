#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

build_makefile() {
	for dir in $(find -name "Makefile" -not -path "*/cmake-build-debug/*"); do
		echo $dir
		make -j4 -C $(dirname "$dir")
	done
}

clean() {
	find . -name "build" -exec rm -r "{}" \;
	find . -name "CMakeFiles" -type d -exec rm -r "{}" \;
	find . -name "CMakeCache.txt" -delete
	for dir in $(find -name "Makefile"); do
		echo $dir
		make -C $(dirname "$dir") clean || true
	done
}

build() {
	build_makefile

	for dir in $(find -name "CMakeLists.txt"); do
		cd $(dirname "$dir")
		if [ ! -d build ]; then
			mkdir build
		fi

		cd build
		pwd
		cmake ..
		cd "$DIR"
	done

	build_makefile
}

# TODO: c++ headers missing on raspian for arm-none-eabi
export CXXFLAGS="-I/usr/include/newlib/c++/5.4.1/arm-none-eabi -I/usr/include/newlib/c++/5.4.1/"

if [ -z "$1" ]; then
	build
elif [ "$1" == "clean" ]; then
	clean
fi
