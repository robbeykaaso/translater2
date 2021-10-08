import "../.."
import QtQuick.Controls 2.12

Web{
    Button{
        text: Pipelines().tr("open")
        anchors.right: parent.right
        z: 1
        onClicked: {

        }
    }

    url0: "file:/html/fstree.html"
}
