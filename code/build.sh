pushd ../build/

INCLUDE="-I../code/include/raylib -I../code/include/libchardet"
LIBS="-L. libraylib.a libchardet.a"
SHARED_OBJECTS="./libraylib.so.450 ./libchardet.so.1.0.0"

# -O3 the big guns, if we ever need them
g++ -fstack-protector-all -fno-gnu-unique -rdynamic -shared -fPIC -o festival.so $INCLUDE  ../code/festival.cpp -g
g++ -fstack-protector-all -fno-gnu-unique -o festival.x86_64 $INCLUDE $LIBS $SHARED_OBJECTS ../code/festival_linux.cpp

#../code/include/tree-sitter-cpp/src/parser.c
#clang -rdynamic -shared -fPIC -o festival.so $INCLUDE  ../code/festival.cpp -g
#clang -o festival.x86_64 $INCLUDE $LIBS $SHARED_OBJECTS

popd
