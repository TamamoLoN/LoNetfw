#!/bin/bash
###
 # @Author: TamamoLoN 1016052306@qq.com
 # @Date: 2024-07-15 13:12:38
 # @LastEditors: TamamoLoN 1016052306@qq.com
 # @LastEditTime: 2024-07-19 17:30:56
 # @FilePath: /RTSP_Server/build.sh
 # @Description: 
 # 
 # Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
### 
# rm -rf build
# mkdir build
dir_name="build"
bin_name="bin"
cpu_core=$(nproc)

# 检查目录是否存在，如果不存在则创建
if [ -d "$dir_name" ]; then
    echo "目录 $dir_name 存在。"
else
    echo "目录 $dir_name 不存在，正在创建..."
    mkdir "$dir_name"
    echo "目录 $dir_name 创建成功。"
fi

if [ -d "$bin_name" ]; then
    echo "目录 $bin_name 存在。"
else
    echo "目录 $bin_name 不存在，正在创建..."
    mkdir "$bin_name"
    echo "目录 $bin_name 创建成功。"
fi

git submodule sync --recursive
git submodule update --init --recursive

cd $dir_name
echo "CPU核心数为"$cpu_core", 开始编译..."
cmake ..
make -j$cpu_core
make install

if [ $? -eq 0 ]; then
    echo "编译成功"
else
    echo "编译失败"
    exit -1
fi
# clear