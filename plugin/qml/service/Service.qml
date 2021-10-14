import QtQuick 2.12

Item{

    function provideHandlerGUI(aName, aCom){
        Pipelines().add(function(aInput){
            var scp = aInput.scope()
            var nm = scp.data("name")
            //console.log(nm + "_ide_" + aName )
            aInput.outs(aCom ? aCom.createObject(null, {name: nm + "_ide_" + aName, width: scp.data("width"), height: scp.data("height")}) : null, nm + "_handle_created").scope(true).cache("type", aName)
        }, {name: "create_" + aName + "_handler"})
    }

    Component.onCompleted: {
        var handlers = {}
        var fls = Pipelines().input("gui/service", "", null, true).asyncCall("qml_listFiles").scope().data("data")
        if (fls)
            for (var j in fls){
                var coms = Pipelines().input("gui/service/" + fls[j], "", null, true).asyncCall("qml_listFiles").scope().data("data")
                var com = null
                for (var k in coms)
                    if (coms[k].endsWith(".qml")){
                        com = Qt.createComponent("../gui/service/" + fls[j] + "/" + coms[k])
                        break
                    }

                handlers[fls[j]] = com
            }
        for (var i in handlers){
            provideHandlerGUI(i, handlers[i])
        }
    }
}
