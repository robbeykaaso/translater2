import "../.."
import QtQuick.Controls 2.12
import QtQuick 2.12

Web{
    Button{
        text: Pipelines().tr("open")
        anchors.right: parent.right
        z: 1
        onClicked:
            mn.open()
        Menu{
            id: mn
            MenuItem{
                text: Pipelines().tr("folder")
                onClicked: {
                    Pipelines().run("openFolder_" + name, "")
                }
            }
            MenuItem {
                text: Pipelines().tr("aws")
                onClicked: {
                    Pipelines().run("openAWS_" + name, "")
                }
            }
        }

        Component.onCompleted: {
            Pipelines().find("copyToClipboard").nextF(function(aInput){
                aInput.outs(aInput.data(), "copyToClipboard_" + name)
            })
        }
    }

    url0: "file:/html/fstree.html"
}
