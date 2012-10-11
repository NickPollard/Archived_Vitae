adb pull /sdcard/gmon.out android_gmon.out
~/Programming/android-ndk-r8/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-gprof android/obj/local/armeabi/libvitae.so android_gmon.out > android_profile_results
