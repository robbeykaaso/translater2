import QtQuick 2.12
import "../qml/gui/Pipe"
Item{
    File{

    }
    Progress{

    }
    PWindow{

    }
    MsgDialog{

    }
    function setParam(aType){
        Pipelines().add(function(aInput){
            var tg = aInput.tag()
            Pipelines().find("_paramSet").nextF(function(aInput){
                aInput.out(tg)
            }, "manual", {name: aType + "_paramSet",
                type: "Partial",
                external: aType,
                replace: true})
            aInput.out("manual")
        }, {name: aType + "_setParam",
            aftered: "_setParam",
            type: "Delegate",
            delegate: aType + "_paramSet",
            external: aType})
    }

    Component.onCompleted: {
        Pipelines().add(function(aInput){
            aInput.out()
        }, {name: "js_updateProgress",
            befored: "updateProgress",
            external: "js"})
        Pipelines().add(function(aInput){
            aInput.out()
        }, {name: "c++_updateProgress",
            befored: "updateProgress",
            external: "c++"})

        Pipelines().add(function(aInput){
            var tg = aInput.tag()
            Pipelines().find("_fileSelected").nextF(function(aInput){
                aInput.out(tg)
            }, "manual", {name: "js_fileSelected",
                    type: "Partial",
                    external: "js",
                    replace: true})

            aInput.out("manual")
        }, {name: "js_selectFile",
            aftered: "_selectFile",
            type: "Delegate",
            delegate: "js_fileSelected",
            external: "js"})

        setParam("js")
        setParam("c++")

        Pipelines().add(function(aInput){
            var tg = aInput.tag()
            Pipelines().find("messagePoped").nextF(function(aInput){
                aInput.out(tg)
            }, "manual", {name: "c++_messagePoped",
                type: "Partial",
                external: "c++",
                replace: true})
            aInput.out("manual")
        }, {name: "c++_popMessage",
            aftered: "popMessage",
            type: "Delegate",
            delegate: "c++_messagePoped",
            external: "c++"})
    }
}
