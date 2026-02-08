if [[ $# -eq 0 ]]; then
	echo "No file given to compile"
	exit;
fi

cd build && \
cmake .. && \
make && \
cp mycc ../compiler && \
./mycc ../$1 && \
echo "Compiling assembly code..." && \
nasm output.asm -o output.o -f elf64 && \
gcc output.o -o out && \
echo "Running code..." && \
echo "<---- Code Output ---->" && \
./out || \
echo "Exit code: $?" && \
cd .. && \
cp build/out ./bin
