#include <fcntl.h>
#include <linux/fb.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termio.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>

struct framebuffer_info {
    uint32_t bits_per_pixel;  // framebuffer depth
    uint32_t xres_virtual;    // how many pixel in a row in virtual screen
};

struct framebuffer_info get_framebuffer_info(const char *framebuffer_device_path);

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

int main(int argc, const char *argv[]) {
    // Set terminal to non-blocking raw mode
    set_nonblocking_input(true);

    cv::Mat image;
    cv::Size2f image_size;

    framebuffer_info fb_info = get_framebuffer_info("/dev/fb0");
    std::ofstream ofs("/dev/fb0");

    // read image file (sample.bmp) from opencv libs.
    // https://docs.opencv.org/3.4.7/d4/da8/group__imgcodecs.html#ga288b8b3da0892bd651fce07b3bbd3a56
    image = cv::imread("picture.bmp", cv::IMREAD_COLOR);

    // get image size of the image.
    // https://docs.opencv.org/3.4.7/d3/d63/classcv_1_1Mat.html#a146f8e8dda07d1365a575ab83d9828d1
    image_size = image.size();

    // transfer color space from BGR to BGR565 (16-bit image) to fit the requirement of the LCD
    // https://docs.opencv.org/3.4.7/d8/d01/group__imgproc__color__conversions.html#ga397ae87e1288a81d2363b61574eb8cab
    // https://docs.opencv.org/3.4.7/d8/d01/group__imgproc__color__conversions.html#ga4e0972be5de079fed4e3a10e24ef5ef0
    cv::cvtColor(image, image, cv::COLOR_BGR2BGR565);

    int x_offset = 0;                         // Starting horizontal offset
    int scroll_speed = 500;                     // Speed of scroll (pixels per frame)
    int screen_width = fb_info.xres_virtual;  // Framebuffer width
    bool scroll_right = true;                 // Initial scroll direction

    while (true) {
        // Check for user input to control direction
        char key = getchar();
        if (key == 'j') {
            scroll_right = false;  // Set direction to left
        } else if (key == 'l') {
            scroll_right = true;  // Set direction to right
        }

        // Update x_offset based on direction
        if (scroll_right) {
            x_offset = (x_offset + scroll_speed) % screen_width;
        } else {
            x_offset = (x_offset - scroll_speed + screen_width) % screen_width;
        }

        // Display the image on framebuffer with circular scrolling
        for (int y = 0; y < image.rows; y++) {
            int display_offset = (x_offset) % screen_width;

            ofs.seekp(y * fb_info.xres_virtual * fb_info.bits_per_pixel / 8);

            if (display_offset + image.cols <= screen_width) {
                ofs.write(image.ptr<const char>(y), image.cols * 2);  // No wrap needed
            } else {
                int part1 = screen_width - display_offset;  // Width of first part
                int part2 = image.cols - part1;             // Width of wrapped part

                ofs.write(image.ptr<const char>(y), part1 * 2);
                ofs.seekp(y * fb_info.xres_virtual * fb_info.bits_per_pixel / 8);
                ofs.write(image.ptr<const char>(y) + part1 * 2, part2 * 2);
            }
        }

        usleep(30000);  // Delay for smooth scrolling (30 ms for ~30 FPS)
    }

    return 0;
}

struct framebuffer_info get_framebuffer_info(const char *framebuffer_device_path) {
    struct framebuffer_info fb_info;       // Used to return the required attrs.
    struct fb_var_screeninfo screen_info;  // Used to get attributes of the device from OS kernel.

    // open device with linux system call "open()"
    // https://man7.org/linux/man-pages/man2/open.2.html
    int buffer_fd = open(framebuffer_device_path, O_RDWR);
    ioctl(buffer_fd, FBIOGET_VSCREENINFO, &screen_info);

    // get attributes of the framebuffer device thorugh linux system call "ioctl()".
    // the command you would need is "FBIOGET_VSCREENINFO"
    // https://man7.org/linux/man-pages/man2/ioctl.2.html
    // https://www.kernel.org/doc/Documentation/fb/api.txt

    // put the required attributes in variable "fb_info" you found with "ioctl() and return it."
    fb_info.xres_virtual = screen_info.xres_virtual;      // it is = 8
    fb_info.bits_per_pixel = screen_info.bits_per_pixel;  // it is = 16

    return fb_info;
};
