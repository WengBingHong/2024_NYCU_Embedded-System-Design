#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

std::vector<std::string> load_class_list() {
    std::vector<std::string> class_list;
    std::ifstream ifs("config_files/coco_classes.txt");
    std::string line;
    while (getline(ifs, line)) {
        class_list.push_back(line);
        std::cout << "get " << line << std::endl;
    }
    std::cout << "finish reading class list" << std::endl;
    return class_list;
}

struct framebuffer_info {
    uint32_t bits_per_pixel;  // framebuffer depth
    uint32_t xres_virtual;    // how many pixel in a row in virtual screen
};

struct framebuffer_info get_framebuffer_info(const char *framebuffer_device_path);

const std::string YOLO_VERSION = "v4";

void load_net(cv::dnn::Net &net, bool is_cuda) {
    auto result = cv::dnn::readNetFromDarknet("config_files/yolo" + YOLO_VERSION + ".cfg", "config_files/yolo" + YOLO_VERSION + ".weights");
    if (is_cuda) {
        std::cout << "Attempty to use CUDA\n";
        result.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        result.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);
    } else {
        std::cout << "Running on CPU\n";
        result.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        result.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }
    net = result;
}

const std::vector<cv::Scalar> colors = {cv::Scalar(255, 255, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 0)};

int main(int argc, char **argv) {
    std::vector<std::string> class_list = load_class_list();

    // Load image
    std::string image_path = "example/demo.png";  // Path to the image
    cv::Mat image = cv::imread(image_path);
    cv::Size2f image_size;
    if (image.empty()) {
        std::cerr << "Error: Unable to load image " << image_path << std::endl;
        return -1;
    }
    framebuffer_info fb_info = get_framebuffer_info("/dev/fb0");
    std::ofstream ofs("/dev/fb0");

    // bool is_cuda = argc > 1 && strcmp(argv[1], "cuda") == 0;
    bool is_cuda = 0;

    cv::dnn::Net net;
    load_net(net, is_cuda);

    auto model = cv::dnn::DetectionModel(net);
    model.setInputParams(1. / 255, cv::Size(416, 416), cv::Scalar(), true);

    // Perform detection
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    model.detect(image, classIds, confidences, boxes, 0.2, 0.4);

    // Draw detections on the image
    for (size_t i = 0; i < classIds.size(); ++i) {
        const auto &box = boxes[i];
        int classId = classIds[i];
        const auto color = colors[classId % colors.size()];
        cv::rectangle(image, box, color, 3);

        cv::rectangle(image, cv::Point(box.x, box.y - 20), cv::Point(box.x + box.width, box.y), color, cv::FILLED);
        cv::putText(image, class_list[classId], cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
    }

    cv::imwrite("example/output.png", image);

    image_size = image.size();
    cv::cvtColor(image, image, cv::COLOR_BGR2BGR565);

    // output to framebufer row by row
    for (int y = 0; y < image_size.height; y++) {
        ofs.seekp(y * fb_info.xres_virtual * fb_info.bits_per_pixel / 8);
        ofs.write(image.ptr<const char>(y), image_size.width * fb_info.bits_per_pixel / 8);
    }

    return 0;
}

struct framebuffer_info get_framebuffer_info(const char *framebuffer_device_path) {
    struct framebuffer_info fb_info;       // Used to return the required attrs.
    struct fb_var_screeninfo screen_info;  // Used to get attributes of the device from OS kernel.

    int buffer_fd = open(framebuffer_device_path, O_RDWR);
    ioctl(buffer_fd, FBIOGET_VSCREENINFO, &screen_info);

    // put the required attributes in variable "fb_info" you found with "ioctl() and return it."
    fb_info.xres_virtual = screen_info.xres_virtual;      // it is = 8
    fb_info.bits_per_pixel = screen_info.bits_per_pixel;  // it is = 16

    return fb_info;
};
