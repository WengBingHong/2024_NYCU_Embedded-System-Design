Reference:
https://github.com/doleron/yolov5-opencv-cpp-python.git
Follow the ref, and you can check your code on pc, before running on board.

# cmake opencv4.5
may lead to cmake error
need to comment the error message mentioned in opencv-4.5.4/modules/world/CMakeLists.txt

#if(";${OPENCV_MODULES_BUILD};" MATCHES ";opencv_viz;" AND #OPENCV_MODULE_opencv_viz_IS_PART_OF_WORLD AND VTK_VERSION VERSION_GREATER_EQUAL "8.90.0")
#  vtk_module_autoinit(TARGETS opencv_world MODULES ${VTK_LIBRARIES})
#endif()

# Convert model this is how you get yolov5 multiple models
python3 export.py --weights yolov5m.pt --img 640 --include onnx --opset 11
python3 -m onnxsim yolov5m.onnx sim_yolov5m.onnx
