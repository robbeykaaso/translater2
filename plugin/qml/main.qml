import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 1.4
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import QtQuick.Controls.Universal 2.3
import "qml/gui/Basic"
import "qml/gui/Pipe"
import QSGBoard 1.0

ApplicationWindow {
    width: 800
    height: 600
    visible: true
    property var mdl
    Universal.theme: Universal.Dark

    menuBar: MenuBar{
        id: mainmenu
        Menu{
            title: Pipelines().tr("file")
            MenuItem{
                text: Pipelines().tr("open folder")
                onClicked: {
                    var dir = Pipelines().input({folder: true}, "", {}, true).asyncCall("_selectFile").data()
                    if (!dir.length)
                        return
                    var pths = Pipelines().input(dir[0], "", {}, true).asyncCall("qml_listFiles").scope().data("data")
                    mdl = {
                        type: "folder",
                        name: dir[0],
                        id: dir[0],
                        children: []
                    }
                    for (var i in pths)
                        if (pths[i] !== "." && pths[i] !== ".."){
                            var tp = "folder"
                            if (pths[i].indexOf(".") >= 0)
                                if (pths[i].indexOf(".png") >= 0 || pths[i].indexOf(".bmp") >= 0 || pths[i].indexOf(".jpg") >= 0)
                                    tp = "image"
                                else if (pths[i].indexOf(".dxf") >= 0 || pths[i].indexOf(".plt") >= 0)
                                    tp = "shape"
                                else
                                    tp = "text"
                            mdl.children.push({
                                                  name: pths[i],
                                                  type: tp,
                                                  id: dir[0] + "/" + pths[i],
                                                  children: []
                                              })
                        }
                    treeview.buildDefaultTree([mdl], dir[0])
                }
            }
        }
    }

    contentData:
        Column{
            anchors.fill: parent
            Row{
                width: parent.width
                height: parent.height - 30
                TreeView0{
                    id: treeview
                    width: valid() ? 150 : 0
                    height: parent.height
                    imagePath: {
                        "image": '../../resource/image.png',
                        "text": '../../resource/text.png',
                        "shape": '../../resource/shape.png'
                    }
                    selectWay: 'background'
                    selectColor: 'blue'
                    openWay: 'all'
                    onSetCurrentSelect: function (select) {
                        console.log("treeview0 selected: " + select)
                    }
                    Component.onCompleted: {
                       /* var tmp = [{
                                       "type": "folder",
                                       "name": "folder0",
                                       "id": "folder0",
                                       "children": [{
                                               "type": "page",
                                               "name": "page0",
                                               "id": "page0",
                                               "children": [{
                                                       "id": "id0",
                                                       "type": "image",
                                                       "name": "image0",
                                                       "position": [],
                                                       "comment": "",
                                                       "source": "" //url
                                                   }, {
                                                       "id": "id1",
                                                       "type": "text",
                                                       "name": "text0",
                                                       "position": [],
                                                       "comment": "",
                                                       "relative_position": [],
                                                       "content": "",
                                                       "size": 16,
                                                       "color": "green",
                                                       "bold": ""
                                                   }, {
                                                       "id": "id2",
                                                       "type": "shape",
                                                       "name": "text0",
                                                       "position": [],
                                                       "comment": "",
                                                       "relative_position": [],
                                                       "direction": {
                                                           "color": "green",
                                                           "border": {
                                                               "type": "line",
                                                               "color": "red"
                                                           },
                                                           "radius": 30
                                                       }
                                                   }]
                                           }, {
                                               "type": "folder",
                                               "name": "folder0",
                                               "id": "folder1",
                                               "children": [{
                                                       "id": "id3",
                                                       "type": "text",
                                                       "name": "text0_0",
                                                       "range": [0, 0, 50, 50],
                                                       "content": ""
                                                   }]
                                           }]
                                   }]
                        buildDefaultTree(tmp, 'folder1')
                        */
                    }
                }
                TextArea{
                    width: parent.width - 150
                    height: parent.height
                    text: "hello world"
                }
            }
            Status{
                width: parent.width
                height: 30
            }
        }
    File{

    }
}
