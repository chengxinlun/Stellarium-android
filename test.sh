#!/bin/sh
/home/cheng/Documents/android-ndk-r13b/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-g++ -fstack-protector-strong -DANDROID -march=armv7-a -mfloat-abi=softfp -mfpu=vfp -fno-builtin-memmove --sysroot=/home/cheng/Documents/android-ndk-r13b/platforms/android-16/arch-arm/ -g -g -marm -O0 -std=gnu++11 -D_REENTRANT -dM -E -o moc_predefs.h /usr/local/Qt-5.8.0/mkspecs/features/data/dummy.cpp

