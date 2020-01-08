//
// Created by z on 2019/12/31.
//

#include "ECV.hpp"
#include <dlfcn.h>
#include <iostream>
#include <ecv_engine_export.h>
#include <tbb/concurrent_queue.h>
#include <memory>
#include <map>
#include <folly/dynamic.h>
#include <folly/json.h>
#include <queue>
#include <fstream>
#include <cstdio>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <glog/logging.h>
void *ecv_handle = dlopen("/home/z/Software/ECV_dev/libecv_video_engine/libecv_video_engine.so.1.0.4.1", RTLD_LAZY);

using namespace cv;
//需要重新开启一个线程去处理, 用于存放视频帧的数据
tbb::concurrent_bounded_queue<hand_frame> frame_q;
closeOpts c_opts[10];

int VideoCallBackFun(Frame *frame, void *userdata)
{
    if(frame->idx == 0)
    {
        LOG(INFO)<<"usr data is: "<<(char*)userdata<<"frame->idx"<<frame->idx;
        return 0;
    }
    if (frame->width*frame->height <= 0)
        return 0;

    if (!frame->key_frame)
    {
        if(reImp_ECV_Free(frame) == ECV_SUCCEED)
            LOG(INFO) << "not a key frame, Free frame in 'ecv_video_engine', succeed! frame_idx: " << frame->idx<<endl;
        return 0;
    }

    shared_ptr<Frame> frame_info(frame, [](Frame *ecv_frame){
        if(reImp_ECV_Free(ecv_frame) == ECV_SUCCEED)
        {
            LOG(INFO) << "Free frame in 'ecv_video_engine', succeed! frame_idx: " << ecv_frame->idx<<endl;
        }
    });
    hand_frame tmpframe(frame_info, userdata);
    frame_q.push(move(tmpframe));

    return  0;
}


void readJson(const string filename, int idx)
{
    ifstream vehicle_data_if(filename);
    stringstream  vehicle_data_ss;

    vehicle_data_ss << vehicle_data_if.rdbuf();
    folly::dynamic vehicle_data_d = folly::parseJson(vehicle_data_ss.str());

    map<string, int> m_rtsp;

    //解析json文件
    for (size_t i = 0; i < vehicle_data_d.size(); i++)
    {
        folly::dynamic per_rtsp_data = vehicle_data_d.at(i);
        auto rtsp = per_rtsp_data[*per_rtsp_data.keys().begin()];
        m_rtsp.emplace(make_pair(move(rtsp["prot"].asString()), rtsp["weight"].asInt()));
    }
    assert(!m_rtsp.empty());

    //定义选项
    VideoOption opts[2];
    opts[0].key = "decoder_type";
    opts[0].value = "auto";
    opts[1].key = "timeout";
    opts[1].value = "5000";


    ///ECV_OpenVideo
    VideoContext* ctx = nullptr;

    while (true)
    {
        for (auto it = m_rtsp.begin(); it != m_rtsp.end(); it++)
        {
            c_opts[idx].m_number = 0;
            c_opts[idx].m_userdata = (*it).first;
            c_opts[idx].m_ctx = &ctx;
            reImp_ECV_OpenVideo((*it).first.c_str(), opts, 2, (void*)(*it).first.c_str(), &ctx);
            sleep(20);
            LOG(INFO)<<"read one rtsp compeletly";
        }
    }
}

int reImp_ECV_Free(void* frame) {
    auto Free_fun =reinterpret_cast<int(*)(void*)>(dlsym(ecv_handle, "ECV_Free"));
    return (*Free_fun)(frame);
}

int reImp_ECV_Init(const InitParam *param) {
    //// ECV_Init
    int (*Init_fun)(const InitParam* );
    Init_fun = reinterpret_cast<int(*)(const InitParam*)>(dlsym(ecv_handle,"ECV_Init"));
    return (*Init_fun)(param);
}

int reImp_ECV_OpenVideo(const char* uri, const VideoOption * opts, int num, void* userdata, VideoContext **ctx) {
    /////ECV_OpenVideo
    int (*OpenVideo_fun)(const char*, const VideoOption*, int, void*, VideoContext**);
    OpenVideo_fun = reinterpret_cast<int(*)(const char*, const VideoOption*, int, void*, VideoContext**)>(dlsym(ecv_handle, "ECV_OpenVideo"));
    return (*OpenVideo_fun)(uri, opts, num, userdata, ctx);
}

int reImp_ECV_Uninit()
{
    /////ECV_Uninit
    int (*Uninit_fun)();
    Uninit_fun = reinterpret_cast<int(*)()>(dlsym(ecv_handle,"ECV_Uninit"));
    return (*Uninit_fun)();
}

int reImp_ECV_CheckVideoUri(const char *uri) {
    ///// ECV_CheckVideoUri
    int (*CheckVideoUri_fun)(const char*);
    CheckVideoUri_fun = reinterpret_cast<int(*)(const char*)>(dlsym(ecv_handle,"ECV_CheckVideoUri"));
    return (*CheckVideoUri_fun)(uri);
}

int reImp_ECV_CheckVideoOpions(int schema, const VideoOption* opts, int num)
{
    /////ECV_CheckVideoOpions
    int (*CheckVideoOpions_fun)(int, const VideoOption*, int);
    CheckVideoOpions_fun = reinterpret_cast<int(*)(int, const VideoOption*, int)>(dlsym(ecv_handle, "ECV_CheckVideoOpions"));
    return (*CheckVideoOpions_fun)(schema, opts, num);
}

