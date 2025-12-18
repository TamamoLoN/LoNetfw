# LoNetfw
本框架基于sylar网络框架学习重写，支持linux/windows\
sylar: https://github.com/sylar-yin/sylar \
linux: epoll dl posix\
windows: wepoll minhook pthreads-w32\

## 编译
### linux(仅在ubuntu18.04、ubuntu20.04上测试编译)
``` shell
bash build.sh
```
### windows(仅在VS2022上测试编译)
``` shell
git submodule sync --recursive
git submodule update --init --recursive
mkdir build
cd build
cmake ..
# 生成.sln后使用VS打开编译
```

## TODO - 完善各个模块的架构以及流程说明