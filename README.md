# c-compiler

# Docker Environment
```
docker build --platform linux/amd64 -t compiler
docker run -it --rm -v $(pwd):/home/user/c-compiler/ --cap-add=SYS_PTRACE --security-opt="seccomp=unconfined" compiler
```

# Reference
[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)
