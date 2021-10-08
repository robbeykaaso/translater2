import QtQuick 2.12
import QtWebEngine 1.8
import QtWebChannel 1.0
import EnvJS 1.0

WebEngineView {
    property string name
    property string index: name.substring(8, name.length).split("_")[0]
    property string url0
    activeFocusOnPress: true

    EnvJS{
        id: envjs
        env: {
            "tag": name,
            "pip": "js" + index
        }
    }
    onContextMenuRequested: { //disable context menu
        request.accepted = true
    }
    webChannel: WebChannel{
        id: webview_chn
        Component.onCompleted: {
            var stm = Pipelines().asyncCall("pipelineJS" + index + "Object", 0)
            webview_chn.registerObject("Pipelinec++", stm.data())
            stm = Pipelines().asyncCall("pipelineQMLObject", 0)
            webview_chn.registerObject("Pipelineqml", stm.data())
            webview_chn.registerObject("Environment", envjs)
            url = url0
        }
    }
}
