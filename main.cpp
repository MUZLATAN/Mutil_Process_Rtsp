//
// Created by z on 2019/12/25.
//
#include <iostream>
#include "Utils.hpp"
#include "ECV.hpp"
#include <cstddef>
#include <string>
#include <thread>
#include <vector>
#include <glog/logging.h>
#include <opencv2/opencv.hpp>
extern tbb::concurrent_bounded_queue<hand_frame> frame_q;
extern void* ecv_handle;
extern closeOpts c_opts[10];
//放到队列
void hand_frame_func(int processNum, int frame_number)
{
    int frame_count = 0;
    Utils utils;
    while (true)
    {
        if (!frame_q.empty())
        {
            cout << "queue size is :" << frame_q.size() << "   ";
            hand_frame frame_info;
            frame_q.pop(frame_info);
            cout << frame_count++ << "   ";

            char* userdata = (char*)frame_info.m_userdata;
            for (size_t ind = 0; ind < processNum; ind++)
            {
                if (strcmp(userdata,c_opts[ind].m_userdata.c_str() )== 0 )
                {
                    if (frame_number <= c_opts[ind].m_number)
                    {
                        LOG(INFO)<<"ECV_Free the ctx";
                        reImp_ECV_Free(*(c_opts[ind].m_ctx));
                        LOG(INFO)<<"ECV_Free the ctx completely";
                        c_opts[ind].m_number = 0;

                    } else
                        c_opts[ind].m_number++;
                }
            }

            int frame_size = frame_info.m_frame->width  * frame_info.m_frame->height;
            shared_ptr<char> frame_yuv_data(new char[frame_size * 3 / 2], [](char *data) {
                delete[] data;
                cout<< "Show image buffer, free!"<<endl;
            });

            cv::Mat bgr_frame;

            if (frame_info.m_frame->type == ECV_IMGTYPE_I420) {
                if (!frame_info.m_frame->img_data[0].size) continue;

                memcpy(frame_yuv_data.get(), frame_info.m_frame->img_data[0].data, frame_info.m_frame->img_data[0].size);
                memcpy(frame_yuv_data.get() + frame_size, frame_info.m_frame->img_data[1].data, frame_info.m_frame->img_data[1].size);
                memcpy(frame_yuv_data.get() + frame_size + frame_size / 4, frame_info.m_frame->img_data[2].data, frame_info.m_frame->img_data[2].size);
                cv::Mat yuv_frame = cv::Mat(frame_info.m_frame->height * 3 / 2, frame_info.m_frame->width, CV_8UC1, frame_yuv_data.get());
                cvtColor(yuv_frame, bgr_frame, cv::COLOR_YUV2BGR_I420);
            }
            utils.saveImage("/home/z/workspace/Mutil_Process_Rtsp/data", string((char*)frame_info.m_userdata), bgr_frame, frame_count);
        }
    }
}
int main()
{
    Utils util;
    util.devideJson("/home/z/workspace/Mutil_Process_Rtsp/rtsp.json", 2);

    InitParam init_param;
    init_param.callback = reinterpret_cast<VideoCallback >(&(VideoCallBackFun));

    ///Init ECV
    reImp_ECV_Init(&init_param);


    int processNum = 2;
    int frame_number = 10;
    for (size_t idx = 0; idx < processNum; idx ++)
    {
        std::thread th([idx]() {
            readJson("/home/z/workspace/Mutil_Process_Rtsp/rtsp/rtsp" + to_string(idx) + ".json", idx);
        });
        th.detach();
    }
    std::thread th([processNum,frame_number](){
        hand_frame_func(processNum, frame_number);
    });
    th.detach();

    getchar();

    shared_ptr<char> uninit_sp(new char, [&](char* ch){
        hand_frame temp;
        while ( !frame_q.try_pop(temp));
        if(reImp_ECV_Uninit() != ECV_SUCCEED){
            cout<< "'Uninit', FAILED!";
        }
    });
    cout<< "after 'ECV_Uninit'";
    dlclose(ecv_handle);
    return 0;
}