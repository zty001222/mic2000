This is the first try of using git clone and git branch

docker run -it --rm maxxing/compiler-dev bash
docker run -it --rm -v /home/max/compiler:/root/compiler maxxing/compiler-dev bash
build/compiler -koopa hello.c -o hello.koopa