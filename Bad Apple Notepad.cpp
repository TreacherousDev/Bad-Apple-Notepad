#include <windows.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <vector>

const int WIDTH = 250;
const int HEIGHT = 100;

// Pixel brightness to character
// You can add more shades by making more comparisons 
// and assigning characters to each one
wchar_t pixelToChar(int brightness) {
    if (brightness < 64) return L'.';
    if (brightness < 128) return L'-';
    if (brightness < 192) return L'+';
    return L'@';
}

// Generate Notepad content from a video frame
std::wstring generateNotepadContent(const cv::Mat& frame) {
    std::wstring content;
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            // Check if not out of bounds of frame
            if (x < frame.cols && y < frame.rows) {
                cv::Vec3b pixel = frame.at<cv::Vec3b>(y, x);
                // Convert pixel to grayscale
                // This makes it easy to get just the brightness
                int brightness = (int)(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);
                content += pixelToChar(brightness);
            }
            else {
                content += L'.'; // Fill empty space
            }
        }
        content += L'\n'; // Newline after each row
    }
    return content;
}

int main() {
    // Load video file
    cv::VideoCapture cap("bad_apple.avi");
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file.\n";
        return -1;
    }

    // Find Notepad window
    HWND notepadWindow = FindWindow(TEXT("Notepad"), nullptr);
    if (!notepadWindow) {
        std::cerr << "No Notepad window found.\n";
        return 1;
    }

    // Call the edit notepad Windows API
    HWND editControl = FindWindowEx(notepadWindow, nullptr, TEXT("Edit"), nullptr);
    if (!editControl) {
        std::cerr << "Failed to find the edit control in Notepad.\n";
        return 1;
    }

    // Process each frame
    std::cout << "Processing video and sending frames to Notepad...\n";
    while (true) {
        cv::Mat frame;
        cap >> frame; // Capture the next frame

        if (frame.empty()) {
            std::cerr << "End of video reached.\n";
            break;
        }

        // Resize frame to match the grid
        cv::Mat resizedFrame;
        cv::resize(frame, resizedFrame, cv::Size(WIDTH, HEIGHT));

        // Resize and generate content to Notepad
        std::wstring content = generateNotepadContent(resizedFrame);
        SendMessageW(editControl, WM_SETTEXT, 0, (LPARAM)content.c_str());

        // Delay to match the frame rate 16 ms = approx 60 fps
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}
