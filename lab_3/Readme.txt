# When we need high precision use strong model, slow but precise


# In camera detection we should use small model like yolo fastestv2
# lab_3_fast_camera_detection.cpp compilation don't forget to add libopencv_world.so.3.4 libgomp.so.1
arm-linux-gnueabihf-g++ lab_3_fast_camera_detection.cpp -o lab_3_fast_camera_detection src/yolo-fastestv2.cpp -I src/include -I include/ncnn lib/libncnn.a -I /opt/EmbedSky/gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf/include/ -I /usr/local/arm-opencv/install/include/ -L /usr/local/arm-opencv/install/lib/ -Wl,-rpath-link=/opt/EmbedSky/gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/libc/lib/ -Wl,-rpath-link=/opt/EmbedSky/gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf/qt5.5/rootfs_imx6q_V3_qt5.5_env/lib/ -Wl,-rpath-link=/opt/EmbedSky/gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf/qt5.5/rootfs_imx6q_V3_qt5.5_env/qt5.5_env/lib/ -Wl,-rpath-link=/opt/EmbedSky/gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf/qt5.5/rootfs_imx6q_V3_qt5.5_env/usr/lib/ -lpthread -lopencv_world -fopenmp -std=c++11

# excecute
LD_LIBRARY_PATH=. ./your_code
