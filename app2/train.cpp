#include "rea.h"
#include "util.h"
#include <QFileInfo>

static auto fake_ret = rea::Json("angle", 160,
                          "caption", "ellipse",
                          "center", rea::JArray(0, 0),
                          "radius", rea::JArray(100, 100),
                          "type", "ellipse",
                          "color", "green",
                          "text", rea::Json(
                                     "visible", true,
                                     "size", rea::JArray(50, 30),
                                     "location", "bottom"),
                          "tag", "result",
                          "width", 5);

/*static rea::regPip<int> reg_training([](rea::stream<int>* aInput){
    auto scp = aInput->scope();
    auto rt = scp->data<QString>("root");
    auto pth = scp->data<QString>("path");
    auto cfg = scp->data<QJsonObject>("config");
    auto job = scp->data<QString>("job");
    auto tsk = readStorage(rt, pth, cfg, "JsonObject")->scope()->data<QJsonObject>("data");
    auto all_imgs = tsk.value("images").toObject();
    //pull annos
    std::vector<QString> imgs_pths;
    for (auto i : all_imgs){
        auto imgs = i.toObject();
        auto imgs_cfg = imgs.value("config").toObject();
        auto imgs_root = imgs.value("root").toString();
        auto lst = imgs.value("data").toArray();
        for (auto j : lst)
            imgs_pths.push_back(j.toString());
    }
    //push result annos
    auto rt0 = scp->data<QString>("root0");
    auto pth0 = scp->data<QString>("path0");
    pth0 = QFileInfo(pth0).path() + "/";
    auto cfg0 = scp->data<QJsonObject>("config0");
    for (auto i : imgs_pths)
        writeJsonObject(rt0, pth0 + "/" + job + "/" + i + ".json", cfg0, rea::Json("objects", rea::Json("result_0", fake_ret)));
    rea::pipeline::instance()->call("doTraining", 1000, scp, false);
}, rea::Json("around", "doTraining"));
*/
