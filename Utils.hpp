//
// Created by z on 2019/12/31.
//

#ifndef MUTIL_PROCESS_RTSP_UTILS_HPP
#define MUTIL_PROCESS_RTSP_UTILS_HPP

#include <iostream>
#include <opencv2/opencv.hpp>
#include <map>
#include <tbb/tbb.h>
#include <ecv_video_engine_export.h>
using namespace std;

class Utils {
public:
    //将rtsp.json文件分成几份
    //[in] filename rtsp.json的源文件的绝对路径
    //[in] fileNum  将文件分为几份
    void devideJson(const string filename, size_t fileNum );


    //判断时间是否在早上七点到晚上八点之间, 若是返回True, 程序将继续运行，否则返回False, 程序将休眠
    bool isInCorrectTime();


    //根据rtsp当前的权重, 判断是否读取该条rtsp
    bool ifReadRtsp(const int weight);

    string rtspPath(string rtsp);

    int saveImage(const string filename, string rtsp,const cv::Mat bgr_frame, int idx);
};
#endif //MUTIL_PROCESS_RTSP_UTILS_HPP
