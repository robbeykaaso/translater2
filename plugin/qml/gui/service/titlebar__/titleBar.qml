import QtQuick 2.12
import QtQuick.Controls 2.12

Item{
    MouseArea{
        property int coor_x
        property int coor_y
        anchors.fill: parent
        onPressed: function(aInput){
            coor_x = aInput["x"]
            coor_y = aInput["y"]
        }
        onPositionChanged: function(aInput){
            main_window.x += aInput["x"] - coor_x
            main_window.y += aInput["y"] - coor_y
        }
    }
    Text{
        height: parent.height
        width: height
        anchors.right: parent.right
        verticalAlignment: Text.AlignVCenter
        anchors.verticalCenter: parent.verticalCenter
        text: "X"
        MouseArea{
            anchors.fill: parent
            onClicked: {
                main_window.visible = false
            }
        }
    }
}
