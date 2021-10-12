import QtQuick 2.12
import QtQuick.Controls 2.12
import QtWebEngine 1.8

WebEngineView {
    id: webview
    property string name
    activeFocusOnPress: true
    z: - 1

    onContextMenuRequested: { //disable context menu
        request.accepted = true
    }
    Component.onCompleted: {
        Pipelines().add(function(aInput){
            var tp = aInput.scope().data("type")
            if (tp === "data")
                webview.loadHtml(aInput.data())
            else if (tp === "url")
                webview.url = aInput.data()
            else if (tp === "script")
                webview.runJavaScript(aInput.data())
        }, {name: "updateHtmlAttr_" + name})
    }
}
