import QtQuick 2.12

Item{
    property int index
    property var mdl:[
 /*       {
            i: "0",
            x: 0,
            y: 0,
            w: 1,
            h: 2,
            dely: 0,
        },
        {
            i: "1",
            x: 1,
            y: 0,
            w: 3,
            h: 2,
            dely: 0
        },
        {
            i: "2",
            x: 4,
            y: 0,
            w: 1,
            h: 2,
            dely: 0
        }*/
    ]
    Component.onCompleted: {
        /*index = mdl.length
        Pipelines().add(function(aInput){
            aInput.outs(mdl, "updateLayout")
        }, {name: "testUpdateLayout"})*/

        Pipelines().add(function(aInput){
            var lyot = aInput.data()
            do {
                if (Object.keys(lyot).length){
                    mdl = lyot
                }else{
                    var pths = Pipelines().input({filter: ["Json files (*.json)"], folder: false}, "", null, true).asyncCall("_selectFile").data()
                    if (!pths.length)
                        break
                    var dt = Pipelines().input(false, "", {path: pths[0]}, true).asyncCall("qml_readJsonObject")
                    if (!dt.data())
                        break
                    mdl = dt.scope().data("data")["layout"] || {}
                }
                index = mdl.length
                aInput.outs(mdl, "updateLayout")
            }while(0)
        }, {name: "loadView"})

        Pipelines().add(function(aInput){
            var pths = Pipelines().input({save: true, filter: ["Json files (*.json)"], folder: false}, "", null, true).asyncCall("_selectFile").data()
            if (pths.length){
                var act_mdl = []
                for (var i in mdl){
                    var grd = JSON.parse(JSON.stringify(mdl[i]))
                    grd["i"] = i.toString()
                    act_mdl.push(grd)
                }
                Pipelines().input(false, "", {path: pths[0], data: {layout: act_mdl}}, true).asyncCall("qml_writeJsonObject")
            }
        }, {name: "saveView"})

        Pipelines().add(function(aInput){
            var scp = aInput.scope()
            var idx = scp.data("index")
            if (scp.data("remove")){
                if (mdl.length > 1){
                    mdl = mdl.filter(function(i){
                        return parseInt(i.i) !== idx
                    })
                    aInput.outs(mdl, "updateLayout")
                }
            }else{
                var tar = mdl.findIndex(function(i){
                    return parseInt(i.i) === idx
                })
                var nw = JSON.parse(JSON.stringify(mdl[tar]))
                nw.i = index.toString()
                index++
                mdl.push(nw)
                aInput.outs(mdl, "updateLayout")
            }
        }, {name: "updateLayoutGrid"})

        Pipelines().find("layoutChanged").nextF(function(aInput){
            mdl = aInput.data()
            aInput.out()
        }, "", {name: "c++_layoutChanged", external: "c++"})
    }
}
