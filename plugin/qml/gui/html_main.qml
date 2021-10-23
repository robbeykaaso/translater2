import QtQuick 2.12
import QtQuick.Controls 2.5

ApplicationWindow {
    id: main_window
    width: 800
    height: 600

    GridsModel{
        id: grids_model
    }

    contentData:
        Column{
            anchors.fill: parent
            ReaGrid{
                id: container
                width: parent.width
                height: parent.height - 30

                Action{
                    shortcut: "Shift+A"
                    onTriggered: {
                        Pipelines().run("enableLayout", "")
                    }
                }
                Action{
                    shortcut: "Shift+B"
                    onTriggered: {
                        Pipelines().run("enableModel", "")
                    }
                }
                com: GridItem{
                        fixed: grids_model.fixed_ide.indexOf(index) >= 0
                        hidden_service: grids_model.hidden_service
                        layout_mode: grids_model.layout_mode
                        model_mode: grids_model.model_mode
                        init_edit_mode: grids_model.ide_type[index.toString()] || "auto"
                        anchors.fill: parent
                    }
                Component.onCompleted: {
                    Pipelines().add(function(aInput){
                        aInput.scope().cache("index", cur_edit)
                        aInput.out()
                    }, {name: "js_openWorkFile"}).nextF(function(aInput){
                        var itm = getGridItem(aInput.scope().data("index"))
                        if (itm && !itm.mode.endsWith("__")){
                            if (aInput.data() === ""){
                                aInput.scope().cache("root", itm.rt).cache("config", itm.stg_config)
                                if (itm.mode === "auto")
                                    aInput.out(itm.detail.split(".").pop().toLowerCase())
                                else
                                    aInput.out(itm.mode)
                            }else{
                                itm.detail = aInput.data()
                                itm.rt = aInput.scope().data("root")
                                itm.stg_config = aInput.scope().data("config")

                                if (itm.mode === "auto")
                                    aInput.out(aInput.data().split(".").pop().toLowerCase())
                                else
                                    aInput.out(itm.mode)
                            }
                        }
                    }, "", {name: "openWorkFile", type: "Partial", external: "c++"})

                    Pipelines().add(function(aInput){
                        var itm = getGridItem(cur_edit)
                        if (itm && !itm.mode.endsWith("__") && itm.detail !== ""){
                            aInput.setData(itm.detail).scope()
                            .cache("root", itm.rt)
                            .cache("config", itm.stg_config)
                            .cache("name", itm.name)
                            if (itm.mode === "auto"){
                                var dt = itm.detail
                                dt = dt.substring(dt.lastIndexOf(".") + 1, dt.length)
                                aInput.out(dt)
                            }else
                                aInput.out(itm.mode)
                        }
                    }, {name: "js_saveWorkFile",
                        external: "c++"})

                    Pipelines().add(function(aInput){
                        main_window.visible = aInput.data()
                        if (aInput.scope().data("noframe"))
                            main_window.flags = Qt.Window | Qt.CustomizeWindowHint
                        aInput.out()
                    }, {name: "openNwlanWindow"})
                }
            }
        }
    CommonSv{

    }
    CustomSv{

    }

}
