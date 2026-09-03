@ECHO OFF
SET CC=clang
SET CXX=clang++
SET "PHASOR_DLL_PATH=%CD%\.instrument\out\bin\phasorrt.dll"
SET "PHASOR_INCLUDE_PATH=%CD%\.instrument\out\opt\include"

ECHO Removing stale files

rm -rf profdata

ECHO Creating directories

mkdir profdata
mkdir .instrument

ECHO Building

cmake -S . -B .instrument/build -G Ninja --preset=windows-64-rel -DFORCE_PROJECT_VERSION=ON -DPGO_A=ON
cmake --build .instrument/build -j8
cmake --install .instrument/build --prefix .instrument/out

ECHO Instrument phasorc

.instrument\out\bin\phasorc.exe -o examples\hello.phsb examples\hello.phs
.instrument\out\bin\phasorc.exe -o examples\printargs.phsb examples\printargs.phs
.instrument\out\bin\phasorc.exe -o examples\rule110\rule110.phsb examples\rule110\rule110.phs
.instrument\out\bin\phasorc.exe -o examples\time.phsb examples\time.phs
.instrument\out\bin\phasorc.exe -o examples\sdl2.phsb examples\sdl2.phs
.instrument\out\bin\phasorc.exe -o examples\snake\snake.phsb examples\snake\snake.phs

ECHO Instrument phasor

.instrument\out\bin\phasor.exe examples\printargs.phs oipzdhzdrhzdhzdzdhzdokq kdjrhzsrhfzdhvcr wzyxfjtxftjxkqm ecgckycgkyxzhrzhgcqk fkbumg ocoxftjxftvnk xtvnfjxfjtzs xfjtxfjpazug xbjztxfjysn hhgzdhzdhzdrhzdrhzdhojo
.instrument\out\bin\phasor.exe examples\time.phs
.instrument\out\bin\phasor.exe examples\sdl2.phs
.instrument\out\bin\phasor.exe examples\snake\snake.phs
.instrument\out\bin\phasor.exe examples\printargs.phsb oipokq kdjvcrhzdhzdrhzdhz wdrhzdhzdrhzdhzykqm ecgcqk fkzdrhzdhzdhzdhzdhbumg ocovnk xtvnzs epazdrhzdhzdhrzdug xbjysn hhgojo
.instrument\out\bin\phasor.exe examples\time.phsb
.instrument\out\bin\phasor.exe examples\sdl2.phsb
.instrument\out\bin\phasor.exe examples\snake\snake.phsb

ECHO Instrument phasorvm 1

.instrument\out\bin\phasorvm.exe examples\hello.phsb
.instrument\out\bin\phasorvm.exe examples\printargs.phsb oipokq kdzdrhzjykfgkyjvcr wzykqm echzdrhzdrhzdrhzdrhgcqk fkbumg ochrzdhzdrhzdrhrzdovnk xtvnzs epahzdhzdrhzdhrzdzug xbjysn hhgojo
.instrument\out\bin\phasorvm.exe examples\rule110\rule110.phsb 2048 10240 false
.instrument\out\bin\phasorvm.exe examples\time.phsb
.instrument\out\bin\phasorvm.exe examples\sdl2.phsb
.instrument\out\bin\phasorvm.exe examples\snake\snake.phsb

ECHO Instrument phasordecomp

.instrument\out\bin\phasordecomp.exe examples\rule110\rule110.phsb
.instrument\out\bin\phasordecomp.exe examples\time.phsb
.instrument\out\bin\phasordecomp.exe examples\sdl2.phsb
.instrument\out\bin\phasordecomp.exe examples\snake\snake.phsb

rm -rf examples\**\*.phsb

ECHO Instrument phasorasm

.instrument\out\bin\phasorasm.exe examples\rule110\rule110.phir
.instrument\out\bin\phasorasm.exe examples\time.phir
.instrument\out\bin\phasorasm.exe examples\sdl2.phir
.instrument\out\bin\phasorasm.exe examples\snake\snake.phir

ECHO Instrument phasorvm 2

.instrument\out\bin\phasorvm.exe examples\rule110\rule110.phsb
.instrument\out\bin\phasorvm.exe examples\time.phsb
.instrument\out\bin\phasorvm.exe examples\sdl2.phsb
.instrument\out\bin\phasorvm.exe examples\snake\snake.phsb

ECHO Instrument libphasorrt

cargo run --example test --release

ECHO Copy files

mv *.profraw profdata\

ECHO Cleanup

rm -rf examples\**\*.phir examples\**\*.phsb outfile.txt