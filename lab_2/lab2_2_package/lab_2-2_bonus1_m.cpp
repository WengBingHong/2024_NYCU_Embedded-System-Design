#include <fcntl.h>
#include <linux/fb.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/ioctl.h>

#include <fstream>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <termio.h>
#include <unistd.h>

// Global variables to communicate and share between threads
bool save_frame_flag = false;  // Flag to indicate if frame should be saved
cv::Mat current_frame;         // Shared frame between threads
pthread_mutex_t mutex;         // Declare a mutex

// Set terminal to non-blocking raw mode
void set_nonblocking_input(bool enable) {
    struct termios t;
    tcgetattr(0, &t);  // Get current terminal attributes

    if (enable) {
        // Set terminal to raw mode (non-canonical mode, no echo)
        t.c_lflag &= ~ICANON;
        tcsetattr(0, TCSANOW, &t);

        // Set non-blocking input
        int flags = fcntl(0, F_GETFL, 0);
        fcntl(0, F_SETFL, flags | O_NONBLOCK);
    } else {
        // Restore terminal settings
        t.c_lflag |= ICANON;
        tcsetattr(0, TCSANOW, &t);

        // Remove non-blocking input
        int flags = fcntl(0, F_GETFL, 0);
        fcntl(0, F_SETFL, flags & ~O_NONBLOCK);
    }
}

// Function to capture 'c' and save the frame
void *save_frame_thread(void *arg) {
    // std::string screenshot_dir = "screenshot";
    // struct stat st;

    // if (stat(screenshot_dir.c_str(), &st) == -1) {
    //     mkdir(screenshot_dir.c_str(), 0777);  // Create the directory if it doesn't exist
    // }

    int i = 0;  // To number screenshots

    while (true) {
        char key = getchar();  // Non-blocking key press
        if (key == 'c') {
            cv::Mat save_frame; 

            pthread_mutex_lock(&mutex);  // Lock the mutex
            save_frame = current_frame;
            pthread_mutex_unlock(&mutex);  // Unlock the mutex

            if (!save_frame.empty()) {
                std::string filename = "screenshot/" + std::to_string(i++) + ".bmp";
                cv::imwrite(filename, save_frame);  // Save the frame
                std::cout << "Saved frame to: " << filename << std::endl;
            }
            
        }

        usleep(1000);  // Sleep to avoid busy-waiting
    }

    return nullptr;
}

struct framebuffer_info {
    uint32_t bits_per_pixel;  // depth of framebuffer
    uint32_t xres_virtual;    // how many pixel in a row in virtual screen
};

struct framebuffer_info get_framebuffer_info(const char *framebuffer_device_path);

int main(int argc, const char *argv[]) {
    // Initialize the mutex
    pthread_mutex_init(&mutex, nullptr);

    // Set terminal to non-blocking raw mode
    set_nonblocking_input(true);

    // variable to store the frame get from video stream
    cv::Mat frame;
    cv::Mat resized_frame;
    cv::Size2f frame_size;

    // open video stream device
    // https://docs.opencv.org/3.4.7/d8/dfe/classcv_1_1VideoCapture.html#a5d5f5dacb77bbebdcbfb341e3d4355c1
    cv::VideoCapture camera(2);

    // get info of the framebuffer
    framebuffer_info fb_info = get_framebuffer_info("/dev/fb0");

    // open the framebuffer device
    // http://www.cplusplus.com/reference/fstream/ofstream/ofstream/
    std::ofstream ofs("/dev/fb0");

    // check if video stream device is opened success or not
    // https://docs.opencv.org/3.4.7/d8/dfe/classcv_1_1VideoCapture.html#a9d2ca36789e7fcfe7a7be3b328038585
    bool check = camera.isOpened();
    if (!check) {
        std::cerr << "Could not open video device." << std::endl;
        return 1;
    }

    // set propety of the frame
    // https://docs.opencv.org/3.4.7/d8/dfe/classcv_1_1VideoCapture.html#a8c6d8c2d37505b5ca61ffd4bb54e9a7c
    // https://docs.opencv.org/3.4.7/d4/d15/group__videoio__flags__base.html#gaeb8dd9c89c10a5c63c139bf7c4f5704d
    /* The video should not be stretched. You must leave blanks on two sides.(4:3 can do the job) 800:600 or 640:480*/
    camera.set(CV_CAP_PROP_FRAME_WIDTH, 800);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT, 600);
    camera.set(CV_CAP_PROP_FPS, 30);

    // cv::VideoWriter video;
    // video.open("/run/media/sda1/screenshot/out.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, cv::Size(800, 600)); // Open the video file for writing and initialize it

    int i = 0;            // img number
    char name[16] = {0};  // img name

    // Create the save frame thread
    pthread_t save_thread;
    if (pthread_create(&save_thread, nullptr, save_frame_thread, nullptr) != 0) {
        std::cerr << "Error: Could not create save frame thread" << std::endl;
        return -1;
    }

    while (true) {
        // get video frame from stream
        // https://docs.opencv.org/3.4.7/d8/dfe/classcv_1_1VideoCapture.html#a473055e77dd7faa4d26d686226b292c1
        // https://docs.opencv.org/3.4.7/d8/dfe/classcv_1_1VideoCapture.html#a199844fb74226a28b3ce3a39d1ff6765
        camera >> frame;
        if (!camera.read(frame))
            std::cout << "No frame" << std::endl;

        // cs

        // Lock the mutex to update the shared frame
        pthread_mutex_lock(&mutex);
        current_frame = frame.clone();
        pthread_mutex_unlock(&mutex);

        // cs

        // get size of the video frame
        // https://docs.opencv.org/3.4.7/d3/d63/classcv_1_1Mat.html#a14set_nonblocking_input(false);  // Restore terminal settings6f8e8dda07d1365a575ab83d9828d1
        frame_size = frame.size();
        int frame_width = frame_size.width; ///////pin
        int frame_height = frame_size.height;///////pin
        int new_frame_width =(frame_width * fb_info.yres_virtual) / frame_height;
        cv::resize(frame ,resized_frame , cv::size(new_frame_width , fb_info.yres_virtual))
        int x_offset = (fb_info.xres_virtual - new_frame_width) / 2;
        // transfer color space from BGR to BGR565 (16-bit image) to fit the requirement of the LCD
        // https://docs.opencv.org/3.4.7/d8/d01/group__imgproc__color__conversions.html#ga397ae87e1288a81d2363b61574eb8cab
        // https://docs.opencv.org/3.4.7/d8/d01/group__imgproc__color__conversions.html#ga4e0972be5de079fed4e3a10e24ef5ef0
        cv::cvtColor(frame, frame, cv::COLOR_BGR2BGR565);

        // if (!frame.empty())
        //     video.write(frame);

        // output the video frame to framebufer row by row
        /*for (int y = 0; y < frame_size.height; y++) {
            // move to the next written position of output device framebuffer by "std::ostream::seekp()"
            // http://www.cplusplus.com/reference/ostream/ostream/seekp/
            ofs.seekp(y * fb_info.xres_virtual * fb_info.bits_per_pixel / 8);

            // write to the framebuffer by "std::ostream::write()"
            // you could use "cv::Mat::ptr()" to get the pointer of the corresponding row.
            // you also need to cacluate how many bytes required to write to the buffer
            // http://www.cplusplus.com/reference/ostream/ostream/write/
            // https://docs.opencv.org/3.4.7/d3/d63/classcv_1_1Mat.html#a13acd320291229615ef15f96ff1ff738
            ofs.write(frame.ptr<const char>(y), frame_size.width * 2);
        }*/
        for (int y = 0; y < fb_info.yres_virtual; y++) {
            // move to the next written position of output device framebuffer by "std::ostream::seekp()"
            // http://www.cplusplus.com/reference/ostream/ostream/seekp/
            ofs.seekp((y * fb_info.xres_virtual + x_offset)* fb_info.bits_per_pixel / 8);

            // write to the framebuffer by "std::ostream::write()"
            // you could use "cv::Mat::ptr()" to get the pointer of the corresponding row.
            // you also need to cacluate how many bytes required to write to the buffer
            // http://www.cplusplus.com/reference/ostream/ostream/write/
            // https://docs.opencv.org/3.4.7/d3/d63/classcv_1_1Mat.html#a13acd320291229615ef15f96ff1ff738
            ofs.write(resized_frame.ptr<const char>(y), new_frame_width * 2);
        }
    }

    // Wait for the keypress thread to finish
    pthread_join(save_thread, nullptr);

    // Destroy the mutex
    pthread_mutex_destroy(&mutex);

    // closing video stream
    // https://docs.opencv.org/3.4.7/d8/dfe/classcv_1_1VideoCapture.html#afb4ab689e553ba2c8f0fec41b9344ae6
    camera.release();

    set_nonblocking_input(false);  // Restore terminal settings

    return 0;
}

struct framebuffer_info get_framebuffer_info(const char *framebuffer_device_path) {
    struct framebuffer_info fb_info;       // Used to return the required attrs.
    struct fb_var_screeninfo screen_info;  // Used to get attributes of the device from OS kernel.

    // open deive with linux system call "open( )"
    // https://man7.org/linux/man-pages/man2/open.2.html
    int fd = open(framebuffer_device_path, O_RDWR);

    // get attributes of the framebuffer device thorugh linux system call "ioctl()"
    // the command you would need is "FBIOGET_VSCREENINFO"
    // https://man7.org/linux/man-pages/man2/ioctl.2.html
    // https://www.kernel.org/doc/Documentation/fb/api.txt
    int attr = ioctl(fd, FBIOGET_VSCREENINFO, &screen_info);

    // put the required attributes in variable "fb_info" you found with "ioctl() and return it."
    fb_info.xres_virtual = screen_info.xres;
    fb_info.bits_per_pixel = screen_info.bits_per_pixel;
    fb_info.yres_virtual = screen_info.yres;///////////////////// pin
    return fb_info;
};
