cmake -B build
cd build
cmake --build .
mt.exe -manifest ../assets/manifest.xml -outputresource:.\Debug\demo.exe
.\Debug\demo.exe
cd ..