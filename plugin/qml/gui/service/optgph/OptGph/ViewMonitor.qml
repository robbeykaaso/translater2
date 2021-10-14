import QtQuick 2.12

Item{
   /* target: parent.combo
    onClicked: {
        console.log("hi")
    }*/

    function updateViews(aInput){
        var tps = Pipelines().input("").asyncCall("getIdeTypes").data()
        var mdl = [], val = []
        for (var i in tps){
            if (tps[i] === "image"){
                val.push("reagrid" + i + "_ide_image")
                mdl.push(i)
            }
        }
        parent.val = val
        parent.combo.model = mdl
    }

    Component.onCompleted: {
        parent.combo.popup.visibleChanged.connect(function(aInput){
            if (parent.combo.popup.visible)
                updateViews()
        })
        updateViews()
    }
}
