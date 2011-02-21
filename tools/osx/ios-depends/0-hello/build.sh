/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/gcc-4.2 \
-arch armv7 -march=armv7a -mcpu=cortex-a8 -mfpu=neon -ftree-vectorize -mfloat-abi=softfp -mthumb -mdynamic-no-pic \
-std=c99 -Wno-trigraphs -fpascal-strings -O0 -Wreturn-type -Wunused-variable -fmessage-length=0 -fvisibility=hidden \
-isysroot /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS4.1.sdk \
-I/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS4.1.sdk/usr/lib/gcc/arm-apple-darwin10/4.2.1/include \
-I/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS4.1.sdk/usr/include \
-o hello hello.c
