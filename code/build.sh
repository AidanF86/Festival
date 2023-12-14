pushd ../build/

INCLUDE="-I../code/include/raylib"
LIBS="-L. libraylib.a"
SHARED_OBJECTS="./libraylib.so.450"

# -O3 the big guns, if we ever need them
g++ -fno-gnu-unique -rdynamic -shared -fPIC -o festival.so $INCLUDE ../code/festival.cpp -g
g++ -fno-gnu-unique -o festival.x86_64 $INCLUDE $LIBS $SHARED_OBJECTS ../code/festival_linux.cpp -g

popd