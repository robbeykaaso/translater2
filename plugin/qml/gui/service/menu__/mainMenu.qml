import QtQuick 2.12
import QtQuick.Controls 2.5

MenuBar{
    Menu{
        title: Pipelines().tr("File")
        Action{
            text: Pipelines().tr("saveModel")
            shortcut: "Ctrl+S"
            onTriggered: {
                Pipelines().run("js_saveWorkFile", "")
            }
        }
        Action{
            text: Pipelines().tr("closeModel")
            shortcut: "Ctrl+C"
            onTriggered: {
                Pipelines().run("js_openWorkFile", "")
            }
        }
    }
    Menu{
        title: Pipelines().tr("View")
        /*MenuItem{
            text: Pipelines().tr("test")
            onTriggered: Pipelines().run("testUpdateLayout", {})
        }*/
        MenuItem{
            text: Pipelines().tr("loadView")
            onTriggered: Pipelines().run("loadView", {})
        }
        MenuItem{
            text: Pipelines().tr("saveView")
            onTriggered: Pipelines().run("saveView", 0)
        }
        MenuItem{
            text: Pipelines().tr("layout")
            onTriggered: Pipelines().run("enableLayout", "")
        }
        MenuItem{
            text: Pipelines().tr("model")
            onTriggered: Pipelines().run("enableModel", "")
        }
    }
    Menu{
        title: Pipelines().tr("Help")
    }
}
