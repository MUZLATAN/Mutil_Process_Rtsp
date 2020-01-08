//
// Created by z on 2019/12/31.
//

#include "Utils.hpp"
#include <ctime>
#include <sstream>
#include <folly/dynamic.h>
#include <folly/json.h>
#include <folly/FileUtil.h>
#include <folly/experimental/DynamicParser.h>
#include <boost/filesystem.hpp>
#include <glog/logging.h>
void Utils::devideJson(const string filename, size_t fileNum)
{
    ifstream vehicle_data_if(filename);
    stringstream  vehicle_data_ss;

    vehicle_data_ss << vehicle_data_if.rdbuf();
    folly::dynamic vehicle_data_d = folly::parseJson(vehicle_data_ss.str());

    //读取json里面的rtsp 并将其存入vector
    folly::DynamicParser parser(folly::DynamicParser::OnError::THROW,&vehicle_data_d);
    std::vector<std::string> vec;
    parser.required("rtsp",[&](){
        parser.arrayItems([&](std::string&& data){
            std::cout<<data<<std::endl;
            if (!data.empty())
                vec.emplace_back(std::move(data));
        });
    });
    assert(!vec.empty());

    //每一组的个数
    size_t arrayNum = vec.size() / fileNum;
    for (size_t i = 0; i < fileNum; i++)
    {
        //构造一个folly::dynamic::array用来保存生成的json
        folly::dynamic rtsp_file = folly::dynamic::array();

        if (i != (fileNum -1))
        {
            //将i*ArrNum 开始到 第ArrNum-1个rtsp 存入到文件
            //第一条 至倒数第二条rtsp的格式如下
            //    "rtsp0":{
            //    "prot":"rtsp://192.168.3.50:8554/100.264" ,
            //    "weight":1
            //    },
            for (size_t j = i * arrayNum; j <= ((i + 1) * arrayNum - 1); j++)
            {
                folly::dynamic rtsp_tmp = folly::dynamic::object
                        ("rtsp"+to_string(j),folly::dynamic::object
                                ("prot", vec[j])
                                ("weight", 1));
                rtsp_file.push_back(rtsp_tmp);
            }
        } else
        {
            for (size_t j = i * arrayNum; j < vec.size(); j++)
            {
                folly::dynamic rtsp_tmp = folly::dynamic::object
                        ("rtsp"+to_string(j),folly::dynamic::object
                                ("prot", vec[j])
                                ("weight", 1));
                rtsp_file.push_back(rtsp_tmp);
            }
        }
        auto tmpjsonres = folly::toJson(rtsp_file);

        //写文件
        int idx = filename.find_last_of('/');
        assert(idx > 0);
        string name = filename.substr(0, idx)+"/rtsp/rtsp"+to_string(i)+".json";
        folly::writeFile(tmpjsonres, name.c_str());
    }
}

int Utils::saveImage(const string filename, string rtsp, const cv::Mat bgr_frame, int idx)
{
    CV_Assert(!bgr_frame.empty());

    time_t t = time(nullptr);
    char ch[26] = {0};
    strftime(ch, sizeof(ch)-1, "%Y%m%d%H", localtime(&t));

    string save_path = filename+"/"+rtspPath(move(rtsp))+"/"+ch;
    if (boost::filesystem::create_directories(save_path))
        LOG(INFO)<<"dir create successfully"<<endl;

    cv::imwrite(save_path+"/"+to_string(idx)+".jpg", bgr_frame);

    return 0;
}

string Utils::rtspPath(string rtsp)
{
    //将 '/' 转换为 '_'
    int pos = rtsp.find('/');
    while(-1 != pos)
    {
        rtsp.replace(pos, string("/").length(),"_");
        pos = rtsp.find('/');
    }
    return rtsp;
}
