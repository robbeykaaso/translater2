import QtQuick 2.12
import QtQuick.Controls 2.5

Rectangle{
    id: root
    property string name
    property int index

    property string init_edit_mode: "auto"
    property var grids: parent.parent
    property bool layout_mode: true

    property string detail: ""
    property string rt
    property var stg_config
    width: parent.width / parent.columns
    height: parent.height / parent.rows
    color: "transparent"
    border.color: (grids && grids.cur === index) ? "red" : "pink"

    MouseArea{
        enabled: !layout_mode
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        //hoverEnabled: true //will lead to wrong cursor info to other controls
        anchors.fill: parent
        propagateComposedEvents: true
        z: (grids && grids.cur === index) ? - 1 : 2
        onPressed: {
            grids.cur = index
        }
    }

    function openCurrent(aScope){
        grids.cur = index
        Pipelines().run("openWorkFile", detail, "", aScope ? aScope.cache("root", rt).cache("config", stg_config) : {root: rt, config: stg_config})
    }

    Rectangle{
        id: index_tag
        visible: layout_mode
        width: 20
        height: 20
        radius: 10
        Text {
            text: index
            font.pixelSize: 12
            anchors.verticalCenter: index_tag.verticalCenter
            anchors.horizontalCenter: index_tag.horizontalCenter
        }
    }

    Button{
        id: del
        visible: layout_mode
        width: 20
        height: 20
        text: "-"
        background: Rectangle{
            radius: 10
            opacity: parent.hovered ? 0.8 : 0.2
            color: "red"
        }
        anchors.right: root.right
        onClicked: {
            Pipelines().run("updateLayoutGrid", 0, "", {remove: true, index: index})
        }
    }
    Button{
        visible: layout_mode
        width: 20
        height: 20
        text: "+"
        background: Rectangle{
            radius: 10
            opacity: parent.hovered ? 0.8 : 0.2
            color: "red"
        }
        anchors.right: del.left
        onClicked: {
            Pipelines().run("updateLayoutGrid", 0, "", {remove: false, index: index})
        }
    }

    ComboBox{
        visible: !layout_mode
        id: md
        property var iniModel: ["auto"]
        anchors.bottom: root.bottom
        model: iniModel
        width: 80
        height: 20
        z: 10
        font.pixelSize: 12
        background: Rectangle{
            color: "green"
            opacity: parent.hovered ? 0.8 : (grids && grids.cur === index ? 0.6 : 0.2)
            border.width: grids && grids.cur === index ? 1 : 0
        }

        indicator: Rectangle { }
        delegate: ItemDelegate {
            id: itemDlgt
            width: md.width
            height: md.height
            padding: 0

            contentItem: Text {
                id: textItem
                text: modelData
                color: hovered ? "white" : "gray"
                font: md.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                leftPadding: 3
            }

            background: Rectangle {
                color: itemDlgt.hovered ? "gray" : "white"
                anchors.left: itemDlgt.left
                anchors.leftMargin: 0
            }
            //onPressed: console.log("hello")
        }
        onCurrentTextChanged: {
            if (stk_vw.items){
                if (detail !== "")
                    openCurrent()
                else if (currentText != "auto"){
                    grids.cur = index
                    stk_vw.switchView(currentText)
                    //Pipelines().run("js_getStorageInfo", ".tr_" + currentText, "openDefaultFile")
                }
            }
        }
    }

    StackView{
        id: stk_vw
        property string lasttype: ""
        property var items: ({})
        visible: !layout_mode
        function switchView(aType, aModifyModel = true){
            if (lasttype == ""){
                if (aModifyModel)
                    Pipelines().run("ideVisibleChanged", {index: index, visible: aType})
                push(items[aType])
            }else if (lasttype !== aType){
                if (items[lasttype].beforeDestroy)
                    items[lasttype].beforeDestroy()
                if (aModifyModel)
                    Pipelines().run("ideVisibleChanged", {index: index, visible: aType, invisible: lasttype})
                replace(items[aType])
            }
            lasttype = aType
        }
        Component.onCompleted: {
            Pipelines().add(function(aInput){
                var tp = aInput.scope().data("type")

                var tgt = aInput.data()
                if (tgt){
                    if (tgt.prt)
                        tgt.prt = stk_vw
                    tgt.width = Qt.binding(function(){return root.width})
                    tgt.height = Qt.binding(function(){return root.height})
                    items[tp] = tgt
                }

                md.iniModel.push(tp)
                md.model = md.iniModel
            }, {name: name + "_handle_created"})

            Pipelines().run("create_handler", "", "", {name: name, width: root.width, height: root.height})
            if (init_edit_mode && init_edit_mode != "auto"){
                md.currentIndex = md.model.indexOf(init_edit_mode)
                stk_vw.switchView(init_edit_mode, false)
            }
        }
    }

    onVisibleChanged: {
        if (!visible && grids && grids.cur === index){
            grids.cur = - 1
        }
    }
}
