CXXFLAGS="-c -g -I. `wx-config-debug --cxxflags`"
LDFLAGS="`wx-config-debug --libs`"

CXX=g++

for file in J5MergeMain.cpp  file_checksum.cpp  libconfigcpp.cpp  merge.cpp check_flash_sections.c	grammar.c  libconfig.c	scanctx.c  scanner.c  strbuf.c; do
	${CXX} ${CXXFLAGS} $file
done

${CXX} -o J5Merge ${LDFLAGS} J5MergeMain.o	 libconfig.o	 merge.o    scanner.o file_checksum.o  libconfigcpp.o  scanctx.o  strbuf.o