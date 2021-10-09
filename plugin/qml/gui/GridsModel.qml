import QtQuick 2.12

Item{
    property int index //max index
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
    property var ide_type: ({})
    property var ide_status: ({})
    property bool layout_mode: true

    function nomalizedModel(){
        var act_mdl = []
        var act_ide_type = {}
        var act_ide_status = {}
        for (var i in mdl){
            var grd = JSON.parse(JSON.stringify(mdl[i]))
            var idx = i.toString()
            grd["i"] = idx
            act_ide_type[idx] = ide_type[mdl[i].i]
            act_ide_status[idx] = ide_status[mdl[i].i]
            act_mdl.push(grd)
        }
        return {layout: act_mdl, layout_mode: layout_mode, ide_type: act_ide_type, ide_status: act_ide_status}
    }

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
            if (pths.length)
                Pipelines().input(false, "", {path: pths[0], data: nomalizedModel()}, true).asyncCall("qml_writeJsonObject")
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

        Pipelines().add(function(aInput){
            var dt = aInput.data()
            ide_type[dt["index"].toString()] = dt["visible"]
            aInput.outs("", "saveGridModel")
        }, {name: "ideVisibleChanged"})

        Pipelines().add(function(aInput){
            var dt = aInput.data()
            var idx = dt.tag.split("_")[0].split("reagrid")[1]
            delete dt["tag"]
            ide_status[idx] = dt
            aInput.outs("", "saveGridModel")
        }, {name: "storageOpened"})

        Pipelines().find("layoutChanged").nextF(function(aInput){
            mdl = aInput.data()
            aInput.out()
        }).nextF(function(aInput){
            aInput.setData(nomalizedModel()).out()
        }, "", {name: "saveGridModel", external: "c++"})

        Pipelines().add(function(aInput){
            if (aInput.data() === ""){
                layout_mode = !layout_mode
                Pipelines().run("saveGridModel", "")
            }else
                layout_mode = aInput.data()
            aInput.setData(layout_mode).out()
        }, {name: "enableLayout"})
    }
}
