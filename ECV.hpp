//
// Created by z on 2019/12/31.
//

#ifndef MUTIL_PROCESS_RTSP_ECV_HPP
#define MUTIL_PROCESS_RTSP_ECV_HPP

#include <cstdint>
#include <memory>
#include <queue>
#include <dlfcn.h>
#include <vector>

using namespace std;
///// ECV_Init
struct Slice{
    void* data;
    uint32_t size;
};

struct Frame{
    void* private_data;
    int type;
    int64_t idx;
    int64_t ts_real;
    int64_t ts_logical;
    int width, height;
    int corrupted;
    int key_frame;
    int step[3];
    Slice img_data[3];
    Slice pkg_data;
    Slice id;
};

typedef int (*VideoCallback)(Frame*, void*);
struct InitParam {
    const char *lic;
    VideoCallback callback;
};

struct VideoContext{
};

struct VideoOption{
    const char* key;
    const char* value;
};

struct hand_frame
{
    hand_frame() = default;
    hand_frame(shared_ptr<Frame> frame, void* userdata):m_frame(frame), m_userdata(userdata){}
    shared_ptr<Frame> m_frame;
    void* m_userdata;

};
struct closeOpts
{
    VideoContext** m_ctx;
    string m_userdata;
    int m_number;
};


void readJson(const string filename, int idx);


int VideoCallBackFun(Frame* frame, void* userdata);

//// 对ECV库的释放帧数据的函数ECV_Free的封装
int reImp_ECV_Free(void*);

//// 对ECV库的初始化函数ECV_Init的封装
int reImp_ECV_Init(const InitParam* param);

//// 对ECV库的ECV_OpenVideo封装
int reImp_ECV_OpenVideo(const char*, const VideoOption*, int, void*, VideoContext**);

//// 对ECV库的ECV_Uninit封装
int reImp_ECV_Uninit();


//// 对ECV库的ECV_CheckVideoUri的封装
int reImp_ECV_CheckVideoUri(const char*);

//// 对ECV库的ECV_CheckVideoOpions
int reImp_ECV_CheckVideoOpions(int, const VideoOption*, int);

#endif //MUTIL_PROCESS_RTSP_ECV_HPP
