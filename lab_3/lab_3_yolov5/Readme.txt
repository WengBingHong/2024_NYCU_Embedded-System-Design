Reference:
'''
https://github.com/doleron/yolov5-opencv-cpp-python.git
'''
Follow the ref, and you can check your code on pc, before running on board.

# cmake opencv4.5
may lead to cmake error
need to comment the error message mentioned in cmakelist or something


# Convert model this is how you get yolov5 multiple models
python3 export.py --weights yolov5m.pt --img 640 --include onnx --opset 11
python3 -m onnxsim yolov5m.onnx sim_yolov5m.onnx
