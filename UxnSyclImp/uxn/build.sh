#!/bin/sh -e

format=0
console=0
install=0
debug=0
norun=0

while [ $# -gt 0 ]; do
	case $1 in
		--format)
			format=1
			shift
			;;

		--console)
			console=1
			shift
			;;

		--install)
			install=1
			shift
			;;

		--debug)
			debug=1
			shift
			;;

		--no-run)
			norun=1
			shift
			;;

		*)
			shift
	esac
done

rm -f ./bin/*

# When clang-format is present

if [ $format = 1 ];
then
	clang-format -i src/uxnasm.c
	clang-format -i src/uxncli.c
	clang-format -i src/uxnemu.c
	clang-format -i src/devices/*
fi

mkdir -p bin
# CC="${CC:-cc}"
CC="${CC:-syclcc}"
CFLAGS="${CFLAGS:--std=c++17 -Wall -Wno-unknown-pragmas}"

#CFLAGS="${CFLAGS:--std=c89 -Wall -Wno-unknown-pragmas}"
case "$(uname -s 2>/dev/null)" in
MSYS_NT*|MINGW*) # MSYS2 on Windows
	FILE_LDFLAGS="-liberty"
	if [ $console = 1 ];
	then
		UXNEMU_LDFLAGS="-static $(sdl2-config --cflags --static-libs | sed -e 's/ -mwindows//g')"
	else
		UXNEMU_LDFLAGS="-static $(sdl2-config --cflags --static-libs)"
	fi
	;;
Darwin) # macOS
	CFLAGS="${CFLAGS} -Wno-typedef-redefinition -D_C99_SOURCE"
	UXNEMU_LDFLAGS="$(brew --prefix)/lib/libSDL2.a $(sdl2-config --cflags --static-libs | sed -e 's/-lSDL2 //')"
	;;
Linux|*)
	UXNEMU_LDFLAGS="-L/usr/local/lib $(sdl2-config --cflags --libs)"
	;;
esac

if [ $debug = 1 ];
then
	echo "[debug]"
	CFLAGS="${CFLAGS} -DDEBUG -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined"
else
	CFLAGS="${CFLAGS} -DNDEBUG -O2 -g0 -s"
fi

${CC} ${CFLAGS} src/uxnasm.cpp -o bin/uxnasm
${CC} ${CFLAGS} src/uxn.cpp src/devices/system.cpp src/devices/file.cpp src/devices/datetime.cpp src/devices/mouse.cpp src/devices/controller.cpp src/devices/screen.cpp src/devices/audio.cpp src/uxnemu.cpp ${UXNEMU_LDFLAGS} ${FILE_LDFLAGS} -o bin/uxnemu
${CC} ${CFLAGS} src/uxn.cpp src/devices/system.cpp src/devices/file.cpp src/devices/datetime.cpp src/uxncli.cpp ${FILE_LDFLAGS} -o bin/uxncli

if [ $install = 1 ]
then
	cp bin/uxnemu bin/uxnasm bin/uxncli $HOME/bin/
fi

./bin/uxnasm projects/software/launcher.tal bin/launcher.rom
./bin/uxnasm projects/software/asma.tal bin/asma.rom
#./bin/uxnasm projects/software/uxnlin.tal bin/uxnlin.rom
./bin/uxnasm projects/software/fib.tal bin/fib.rom




if [ $norun = 1 ]; then exit; fi

./bin/uxnasm projects/software/piano.tal bin/piano.rom
./bin/uxnasm projects/software/uxnlin.tal bin/uxnlin.rom
#./bin/uxnemu -2x bin/launcher.rom

#./bin/uxnemu  -o bin/piano.rom
./bin/uxnemu  -2x  bin/piano.rom
# ./bin/uxncli uxnlin.rom projects/asma.tal

#./bin/uxncli uxnlin.rom
#
#cp projects/software/piano.tal bin/
#
#
#cd bin
#
#./bin/uxncli uxnlin.rom  paino.tal
# ./bin/uxncli bin/fib.rom