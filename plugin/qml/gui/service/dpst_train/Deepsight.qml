import QtQuick 2.12
import QtQuick.Controls 1.4
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4
import "../../../qml/gui/Pipe"
import "../../../qml/gui/Basic"

Row{
    id: root
    property var prt: true
    property string name

    property string user_id
    property string detail
    property string rt
    property var stg_config

    property string server_detail
    property string server_rt
    property var server_config

    TabView{
        id: tbvw
        width: parent.width
        height: parent.height
        style: TabViewStyle {
            frameOverlap: 1
            tab: Rectangle {
                color: styleData.selected ? "gray" :"white"
                border.color:  "gray"
                implicitWidth: Math.max(text.width + 4, 80)
                implicitHeight: 30
                radius: 2
                Text {
                    id: text
                    anchors.centerIn: parent
                    text: styleData.title
                    color: styleData.selected ? "white" : "black"
                    font.pixelSize: 16
                }
            }
            frame: Rectangle {
                color: "white"
            }
        }
        Tab{
            title: "anno"
            active: true
            Row{
                anchors.fill: parent
                Rectangle{
                    width: 120
                    height: parent.height
                    color: "white"
                    border.width: 1
                    List{
                        title: ["annos"]
                        name: "imagelist_" + root.name
                        anchors.fill: parent
                        Component.onCompleted: {
                            Pipelines().add(function(aInput){
                                var sels = aInput.data()
                                for (var i in sels)
                                    sels[i] = entries.get(sels[i]).entry[0]
                                aInput.setData(sels).out()
                            }, {name: name + "_listViewIDSelected",
                                after: name + "_listViewSelected"})
                            Pipelines().find(name + "_listViewSelected").nextF(function(aInput){
                                aInput.scope().cache("path", detail)
                                              .cache("root", rt)
                                              .cache("config", stg_config)
                                aInput.out()
                            }, "manual", {name: "c++_" + name + "_listViewSelected",
                                          type: "Partial",
                                          external: "c++"})
                        }
                    }
                }
                Column{
                    width: parent.width - 120
                    height: parent.height
                    Row{
                        width: parent.width
                        height: 30
                        Button{
                            text: Pipelines().tr("generate")
                            width: 60
                            height: parent.height
                            onClicked: Pipelines().run("generateAnnos", detail, "", {config: stg_config, root: rt, name: name})
                        }
                        Button{
                            text: Pipelines().tr("view")
                            width: 60
                            height: parent.height
                            onClicked: Pipelines().run("setViewMap", {})
                        }
                    }
                    Rectangle{
                        width: parent.width
                        height: parent.height - 30
                        border.width: 1
                        Column{
                            anchors.fill: parent
                            Combo{
                                property bool inited: false
                                leftPadding: 15
                                width: 180
                                caption.text: Pipelines().tr("stage:")
                                combo.model: ["none", "train", "test", "validation"]
                                background.border.width: 1
                                background.border.color: "black"
                                combo.onCurrentIndexChanged: {
                                    if (inited){
                                        var sels = Pipelines().input([], "setAnnoStage").asyncCall("imagelist_" + root.name + "_listViewSelected").data()
                                        Pipelines().run("setAnnoStage", combo.model[combo.currentIndex], "", {annos: sels, config: stg_config, root: rt, path: detail})
                                    }else
                                        inited = true
                                }

                                Component.onCompleted: {
                                    Pipelines().find("c++_imagelist_" + root.name + "_listViewSelected_AnnoAbstract").nextF(function(aInput){
                                        var idx = Math.max(0, combo.model.indexOf(aInput.data()))
                                        if (combo.currentIndex !== idx){
                                            inited = false
                                            combo.currentIndex = idx
                                        }
                                    }, "manual", "c++_imagelist_" + root.name + "_listViewSelected_AnnoAbstract_Show")
                                }
                            }
                            GroupBox{
                                title: Pipelines().tr("roi:")
                                x: 5
                                width: 300
                                height: 80
                                font.pixelSize: 12
                                Column{
                                    anchors.fill: parent
                                    Button{
                                        property bool roi_mode: false
                                        text: Pipelines().tr("edit")
                                        width: 50
                                        height: 20
                                        background: Rectangle{
                                            id: roi_edit
                                            color: "transparent"
                                            border.color: "black"
                                            border.width: 1
                                        }
                                        onClicked: {
                                            if (!roi_mode){
                                                roi_edit.color = "green"
                                                Pipelines().run("setDpstAnnoMode", "", "", {mode: "roi", path: detail, root: rt, config: stg_config})
                                            }else{
                                                roi_edit.color = "transparent"
                                                Pipelines().run("setDpstAnnoMode", "", "", {mode: "anno", path: detail, root: rt, config: stg_config})
                                            }
                                            roi_mode = !roi_mode
                                        }
                                        Component.onCompleted: {
                                            Pipelines().find("joblist_" + root.name + "_listViewSelected").nextF(function(aInput){
                                                if (!roi_mode)
                                                    aInput.outs("", "setDpstAnnoMode")
                                                          .scope(true)
                                                          .cache("mode", "")
                                                          .cache("path", detail)
                                                          .cache("root", rt)
                                                          .cache("config", stg_config || {})
                                            }, "manual", {name: name + "_refreshResultAnno"})
                                        }
                                    }
                                    Check{
                                        property bool permit_save: true
                                        caption.text: Pipelines().tr("local") + ":"
                                        check.checked: true
                                        check.onCheckedChanged: {
                                            if (permit_save){
                                                var chk = Pipelines().input(check.checked, "", {root: rt, path: detail, config: stg_config}, true).asyncCall("setLocalROI").data()
                                                if (check.checked !== chk){
                                                    permit_save = false
                                                    check.checked = chk
                                                }else
                                                    Pipelines().run("setDpstAnnoMode", "", "", {mode: "", path: detail, root: rt, config: stg_config})
                                            }
                                            else
                                                permit_save = true
                                        }
                                        function init(aChecked){
                                            if (aChecked === undefined)
                                                aChecked = false
                                            permit_save = aChecked === check.checked
                                            check.checked = aChecked
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        Tab{
            title: "job"
            active: true
            Row{
                anchors.fill: parent
                Rectangle{
                    width: 120
                    height: parent.height
                    color: "white"
                    border.width: 1
                    List{
                        title: ["jobs"]
                        name: "joblist_" + root.name
                        anchors.fill: parent
                        Component.onCompleted: {
                            Pipelines().add(function(aInput){
                                var sels = aInput.data()
                                for (var i in sels)
                                    sels[i] = entries.get(sels[i]).entry[0]
                                aInput.setData(sels).out()
                            }, {name: name + "_listViewIDSelected",
                                after: name + "_listViewSelected"})
                            Pipelines().add(function(aInput){
                                aInput.out()
                            }, {name: "c++_" + name + "_listViewSelected",
                                aftered: name + "_listViewSelected",
                                type: "Partial",
                                external: "c++"})
                        }
                    }
                }
                Column{
                    width: parent.width - 120
                    height: parent.height
                    Row{
                        width: parent.width
                        height: 30
                        Button{
                            text: Pipelines().tr("train")
                            width: 60
                            height: parent.height
                            onClicked: {
                                /*if (!sv_stat.linked){
                                    Pipelines().run("popMessage", {title: "warning", text: "please link a server at first"})
                                    return
                                }*/
                                var prm = {}
                                var dt = Pipelines().input( {
                                                            title: "parameter mode",
                                                            content:  {
                                                                mode: {
                                                                    type: "combo",
                                                                    model: ["file"]
                                                                }
                                                            }
                                                        }, "", {}, true).asyncCall("_setParam").data()
                                if (dt["mode"] === "file"){
                                    var dir = Pipelines().input({folder: false, filter: ["Parameter files (*.json)"]}, "", {}, true).asyncCall("_selectFile").data()
                                    if (dir.length){
                                        var stm = Pipelines().input(false, "", {path: dir[0], config: {}}, true).asyncCall("qml_readJsonObject")
                                        if (stm.data())
                                            prm = stm.scope().data("data")
                                    }
                                }else
                                    return
                                Pipelines().run("startTrain", {root: rt, path: detail, config: stg_config, parameter: prm}, "", {remote: true})
                            }
                        }
                        /*Button{
                            text: Pipelines().tr("interface")
                            width: 60
                            height: parent.height
                        }*/
                        /*Button{
                            property string server_detail
                            text: Pipelines().tr("server")
                            width: 60
                            height: parent.height
                            background: Rectangle{
                                id: sv_stat
                                property bool linked: false
                                color: linked ? "green" : "red"
                            }
                            onClicked:{
                                if (server_detail == "")
                                    server_detail = "127.0.0.1:8081:12345"
                                var dtl = server_detail.split(":")
                                var dt = Pipelines().input( {
                                                            title: "set server",
                                                            content:  {
                                                                ip: {
                                                                    value: server_detail === "" ? "" : dtl[0]
                                                                },
                                                                port: {
                                                                    value: server_detail === "" ? "" : dtl[1]
                                                                }
                                                            }
                                                        }, "", {}, true).asyncCall("_setParam").data()
                                Pipelines().run("tryLinkServer", {ip: dt["ip"], port: dt["port"], id: "1234"})
                            }
                            Component.onCompleted: {
                                Pipelines().find("serverStated").nextF(function(aInput){
                                    server_detail = aInput.data()["detail"] || ""
                                    sv_stat.linked = server_detail !== ""
                                }, "", {name: name + "_serverStated"})
                                Pipelines().run("serverStated", {value: "from gui"})
                            }
                            Component.onDestruction: {
                                Pipelines().find("serverStated").removeNext(name + "_serverStated", true, false)
                            }
                        }*/
                        Button{
                            property string server_detail
                            text: Pipelines().tr("server")
                            width: 60
                            height: parent.height
                            background: Rectangle{
                                id: sv_stat
                                property bool linked: false
                                color: linked ? "green" : "red"
                            }
                            onClicked:{
                                if (server_detail == "")
                                    server_detail = "127.0.0.1:8081:12345"
                                var dtl = server_detail.split(":")
                                var dt = Pipelines().input( {
                                                            title: "set server",
                                                            content:  {
                                                                ip: {
                                                                    value: server_detail === "" ? "" : dtl[0]
                                                                },
                                                                port: {
                                                                    value: server_detail === "" ? "" : dtl[1]
                                                                }
                                                            }
                                                        }, "", {}, true).asyncCall("_setParam").data()
                                Pipelines().run("tryLinkServer", {ip: dt["ip"], port: dt["port"], id: "1234"})
                            }
                            Component.onCompleted: {
                                Pipelines().find("serverStated").nextF(function(aInput){
                                    server_detail = aInput.data()["detail"] || ""
                                    //console.log(server_detail)
                                    sv_stat.linked = server_detail !== ""
                                }, "", {name: name + "_serverStated"})
                                Pipelines().run("serverStated", {value: "from gui"})
                            }
                            Component.onDestruction: {
                                Pipelines().find("serverStated").removeNext(name + "_serverStated", true, false)
                            }
                        }
                        Button{
                            text: Pipelines().tr("result")
                            width: 60
                            height: parent.height
                            onClicked: Pipelines().run("openResult", {})
                        }
                        /*Button{
                            text: Pipelines().tr("test")
                            width: 60
                            height: parent.height
                            onClicked: {
                                console.log(Pipelines().input({}, "", {remote: true}, true).asyncCall("testServer").data()["hello"])
                                //Pipelines().run("testServer", {}, "", {remote: true})
                            }
                        }*/
                    }
                    Rectangle{
                        width: parent.width
                        height: parent.height - 30
                        border.width: 1
                        color: "black"
                        ListView{
                            property string cur_job: ""
                            property var logs
                            leftMargin: 5
                            anchors.fill: parent
                            clip: true
                            model: ListModel{
                               /* ListElement{
                                    cap: "..."
                                }
                                ListElement{
                                    cap: "..."
                                }
                                ListElement{
                                    cap: "..."
                                }*/
                            }
                            delegate: Label{
                                text: cap
                                width: parent.width
                                color: "white"
                                wrapMode: Text.WordWrap
                            }

                            ScrollBar.vertical: ScrollBar{

                            }
                            function setCurJob(aCurrent){
                                if (aCurrent !== cur_job){
                                    cur_job = aCurrent
                                    model.clear()
                                    var lgs = logs[aCurrent]
                                    for (var i in lgs)
                                        model.append({cap: lgs[i]})
                                }
                            }

                            Component.onCompleted: {
                                Pipelines().find("joblist_" + root.name + "_updateListView").nextF(function(aInput){
                                    var dt = aInput.data()
                                    if (dt["selects"]){
                                        setCurJob(dt["data"][dt["selects"][0]]["entry"][0])
                                    }
                                }, {name: "after_joblist_" + root.name + "_updateListView"})
                                Pipelines().find("joblist_" + root.name + "_listViewSelected").nextF(function(aInput){
                                    setCurJob(aInput.data().length ? aInput.data()[0] : "")
                                    aInput.scope().cache("path", detail)
                                                  .cache("root", rt)
                                                  .cache("config", stg_config)
                                    aInput.out()
                                }, "manual", {name: name + "_refreshLogList"})

                                logs = {}
                                Pipelines().find("logTrain").nextF(function(aInput){
                                    var job = aInput.scope().data("job")
                                    if (!logs[job])
                                        logs[job] = []
                                    var msg = aInput.data()
                                    //console.log(job + ":" + msg)
                                    logs[job].push(msg)
                                    //console.log(job + ";" + cur_job)
                                    if (job === cur_job){
                                        model.append({cap: msg})
                                        if (msg === "complete"){
                                           // console.log("hi")
                                            aInput.outs("", "setDpstAnnoMode")
                                                  .scope(true)
                                                  .cache("mode", "")
                                                  .cache("path", detail)
                                                  .cache("root", rt)
                                                  .cache("config", stg_config || {})
                                        }
                                    }
                                }, "", {name: name + "_logTrain"})
                            }
                            Component.onDestruction: {
                                Pipelines().find("logTrain").removeNext(name + "_logTrain", true, false)
                            }
                        }
                    }
                }

            }
        }
    }
    Component.onCompleted: {
        /*auto imgs = aModel.value("jobs").toObject();
        for (auto i : imgs.keys())
            dt.push_back(rea2::Json("entry", rea2::JArray(i)));
        auto ret = rea2::Json("title", rea2::JArray("id"), "data", dt);
        if (dt.size())
            ret.insert("selects", QJsonArray({aSelLast ? dt.size() - 1 : 0}));*/

        Pipelines().find("startTrain").nextF(function(aInput){
            var dt = aInput.data()
            if (prt.lasttype === "dpst_train" && dt["path"] === detail && dt["root"] === rt && JSON.stringify(dt["config"]) === JSON.stringify(stg_config))
                if (dt["err"])
                    aInput.outs({title: "warning", text: dt["msg"]}, "popMessage")
                else
                    aInput.outs("", "updateJoblist_" + name)
        }, "", {name: "afterStartTrain_" + name})
        Pipelines().find("qml_clientOnline").nextF(function(aInput){
            var scp = aInput.scope()
            if (aInput.data() !== ""){
                user_id = aInput.data()
                server_config = scp.data("config")
                server_detail = scp.data("path")
                server_rt = scp.data("root")
            }
            if (prt.lasttype === "dpst_train"){
                var stm = Pipelines().input(false, "", {path: server_detail, config: server_config}, true).asyncCall("qml_" + server_rt + "readJsonObject")
                if (stm.data()){
                    var jobs = stm.scope().data("data")
                    var sv = JSON.stringify(server_config)
                    var lst = []
                    for (var i in jobs){
                        var jb = jobs[i]
                        if (jb["user"] === user_id && jb["path"] === detail && jb["root"] === server_rt && JSON.stringify(jb["config"]) === sv)
                            lst.push({entry: [i]})
                    }
                    var ret = {title: ["id"], data: lst}
                    if (lst.length)
                        ret["selects"] = [lst.length - 1]
                    aInput.outs(ret, "joblist_" + root.name + "_updateListView", "manual")
                }
            }
        }, "", {name: "updateJoblist_" + name})
        Pipelines().add(function(aInput){
            var scp = aInput.scope()
            detail = scp.data("path")
            stg_config = scp.data("config")
            //console.log(tbvw.getTab(0).children[0].children[1].children[0].children[1].children[0].children[1])
            tbvw.getTab(0).children[0].children[1].children[1].children[0].children[1].contentChildren[0].children[1].init(aInput.data()["local_roi"])
            rt = scp.data("root")
        }, {name: "updateDpstAttr_" + name})
      //  console.log(name)
    }
}
