import "regenerator-runtime/runtime"
//https://stackoverflow.com/questions/53558916/babel-7-referenceerror-regeneratorruntime-is-not-defined
global.$ = require('jquery')
global.rea = require('reajs')
const copy_to_clipboard = require('copy-to-clipboard')
const rea = require('reajs')

require('jstree')
require('jstree/dist/themes/default/style.css')
//require('bootstrap/dist/css/bootstrap.min.css')
//https://webcache.googleusercontent.com/search?q=cache:9JnMpTlbPRsJ:https://github.com/vakata/jstree/issues/1275+&cd=14&hl=zh-CN&ct=clnk&gl=cz
let tr = $("#jstree")

let mdl
function clearCache () {
  mdl = {
    "loaded": {},
    "id": "",
    "config": {}
  }
}
let src_mode = "test_sys"
let src = {
  test_sys: async function (aPathList) {
    const org = {
      folder0: {
        file0: "text.json",
        folder0: {
          file0: "xxx.bmp",
          file1: "fff.txt",
          folder0: {
            file0: "ggg.txt"
          }
        }
      },
      folder1: {

      },
      folder2: {

      },
      file0: "adsfasf.xml"
    }
    let root = org
    if (aPathList)
      for (let i in aPathList)
        root = root[aPathList[i]]
    else
      clearCache()

    let ret = []
    for (let i in root)
      if (i.startsWith("folder"))
        ret.push(i)
      else if (i.startsWith("file"))
        ret.push(root[i])
    return ret
  },
  file_sys: async function (aPathList) {
    let dir0 = ""
    if (!aPathList) {
      let dir = await rea.pipelines().input({ folder: true }, "", null, true).asyncCall("js_selectFile")
      if (!dir.data().length)
        return null
      dir0 = dir.data()[0]
      mdl.id = dir0
    } else {
      for (let i in aPathList) {
        if (dir0 != "")
          dir0 += "/"
        dir0 += aPathList[i]
      }
    }
    let ret = (await rea.pipelines().input(dir0, "", null, true).asyncCall("js_listFiles")).scope().data("data")
    return ret
  },
  aws_sys: async function (aPathList) {
    let dir0 = ""
    for (let i in aPathList) {
      if (dir0 != "")
        dir0 += "/"
      dir0 += aPathList[i]
    }
    return (await rea.pipelines().input(dir0, "", new rea.scopeCache({ config: mdl.config }), true).asyncCall("js_aws0listFiles")).scope().data("data")
  }
}

const icons = {
  "png": "img",
  "bmp": "img",
  "jpg": "img",
  "jpeg": "img",
  "xml": "txt",
  "txt": "txt",
  "json": "txt",
  "tr_qsg": "txt",
  "tr_text": "txt",
  "obj": "cube",
  "stl": "cube",
  "html": "html",
  "tr_html": "html",
  "dpst_train": "dpst",
  "tr_dpst_train": "dpst",
  "dpst_anno": "dpst",
  "tr_dpst_anno": "dpst",
  "mp4": "md",
  "mkv": "md",
  "avi": "md"
}

function prepareEntry (aID, aText) {
  let entry = {
    id: aID,
    text: aText
  }
  let ext = aText.split(".").pop().toLowerCase()
  if (icons[ext]) {
    entry["icon"] = "../assets/" + icons[ext] + ".png"
  }/*else{
    entry["state"] = {
      opened: false
    }
  }*/
  return entry
}

function createChildNode (aTarget) {
  let tgts = aTarget.split("/")
  let ent = prepareEntry(aTarget, tgts.pop())
  let prt = tgts.join("/")
  tr.jstree().create_node(prt == mdl.id ? "#" : [tgts.join("/")], ent)
}

async function copyFileOrDir (aFileOrDir, aOriRoot, aRoot, aTarget, aOriConfig) {
  let fls = (await rea.pipelines().input(aFileOrDir, "", new rea.scopeCache({ config: aOriConfig }), true).asyncCall("js_" + aOriRoot + "listFiles")).scope().data("data")
  let clds = []
  for (let i in fls)
    if (fls[i] != "." && fls[i] != "..")
      clds.push(fls[i])
  if (!clds.length) {
    //console.log(aFileOrDir + " ==> " + aTarget)
    let stm = (await rea.pipelines().input(false, "", new rea.scopeCache({ path: aFileOrDir, config: aOriConfig }), true).asyncCall("js_" + aOriRoot + "readByteArray"))
    if (stm.data()) {
      stm.setData(false).scope().cache("path", aTarget).cache("config", mdl.config)
      await stm.asyncCall("js_" + aRoot + "writeByteArray")
      rea.pipelines().run("js_updateProgress", {})
      createChildNode(aTarget)
    }
  } else {
    createChildNode(aTarget)
    rea.pipelines().run("js_updateProgress", { step: clds.length - 1 })
    for (let i in clds)
      await copyFileOrDir(aFileOrDir + "/" + clds[i], aOriRoot, aRoot, aTarget + "/" + clds[i], aOriConfig)
  }
}

function deleteFiles (aSels) {
  if (!aSels.length)
    return
  for (let i in aSels)
    rea.pipelines().run("js_" + (src_mode == "aws_sys" ? "aws0" : "") + "deletePath", aSels[i])
  tr.jstree().delete_node(aSels)
}

async function copyFiles (aSels) {
  if (!aSels.length)
    return
  rea.pipelines().run("copyToClipboard", { files: aSels, root: src_mode == "aws_sys" ? "aws0" : "", config: mdl.config })
}

let clipboard
rea.pipelines("c++").init(function () {
  rea.pipelines().add(aInput => {
    clipboard = aInput.data()
  }, { name: "copyToClipboard_" + rea.env().tag })
})

async function pasteFiles (aSels, aData) {
  //let prt = tr.jstree().get_parent(tr.jstree("get_selected"))
  let prt = mdl.id
  if (aSels.length) {
    if (aSels[0].split("/").pop().indexOf(".") < 0)
      prt = aSels[0]
    else
      return
  }
  rea.pipelines().run("js_updateProgress", { title: "copying...", sum: aData.files.length })
  for (let i in aData.files)
    await copyFileOrDir(aData.files[i], aData.root, src_mode == "aws_sys" ? "aws0" : "", (prt == "" ? "" : prt + "/") + aData.files[i].split("/").pop(), aData.config)
}

async function newFiles(aSels){
  let prt = mdl.id
  if (aSels.length){
    if (aSels[0].split("/").pop().indexOf(".") < 0)
      prt = aSels[0]
    else
      return
  }
  let stm = await rea.pipelines().input({
    title: "new file",
    content: {
      name: {
          value: ""
      }
    }
  }, "newfile")
  .asyncCall("js_setParam")
  let nm = stm.data()["name"]
  if (nm == ""){
      rea.pipelines().run("popMessage", {title: "warning", text: "invalid name"})
      return
  }
  let pth = (prt == "" ? "" : prt + "/") + nm
  let rt = src_mode == "aws_sys" ? "aws0" : ""
  let exist = (await rea.pipelines().input(false, "", new rea.scopeCache({path: pth, config: mdl.config}), true).asyncCall("js_" + rt + "readByteArray"))
  if (exist.data()){
      rea.pipelines().run("popMessage", {title: "warning", text: "existed file"})
      return
  }
  let ret = await rea.pipelines().input(false, "", new rea.scopeCache({path: pth, config: mdl.config, data: ""}), true).asyncCall("js_" + rt + "writeByteArray")
  if (ret.data())
    createChildNode(pth)
}

function refreshTree (aData) {
  tr.data("jstree", false).empty() //https://www.cnblogs.com/hofmann/p/12844311.html
  tr.jstree({
    core: {
      check_callback: true,
      data: aData
    },
    plugins: ["contextmenu", "search"],
    contextmenu: {
      items: e => {
        let ret = {}
        let sels = tr.jstree("get_selected")
        if (sels.length)
          ret = {
            delete: {
              label: "delete",
              action: function () {
                deleteFiles(sels)
              }
            },
            copy: {
              label: "copy",
              action: function () {
                copyFiles(sels)
              }
            },
            copy_path: {
              label: "copy path",
              action: function () {
                let cnt = ""
                for (let i in sels) {
                  if (cnt != "")
                    cnt += "\n"
                  cnt += sels[i]
                }
                copy_to_clipboard(cnt)
              }
            },
            de_select: {
              label: "deselect",
              action: function () {
                tr.jstree("deselect_all")
                cur_sel_data = null
              }
            }
          }
        if (clipboard && clipboard.files.length)
          ret["paste"] = {
            label: "paste",
            action: function () {
              pasteFiles(sels, clipboard)
            }
          }
        ret["newFile"] = {
          label: "newFile",
          action: function(){
            newFiles(sels)
          }
        }
        return ret
      },
      select_node: false,
      show_at_node: false
    }
  })
  tr.jstree(true).refresh()
}

tr.on("keyup", ".jstree-anchor", function (e) {
  if (e.ctrlKey) {
    if (e.which == 67) {  //c
      let sels = tr.jstree("get_selected")
      copyFiles(sels)
    } else if (e.which == 86) {   //v
      let sels = tr.jstree("get_selected")
      pasteFiles(sels)
    }
  } else if (e.which == 46) {  //del
    deleteFiles(sels)
  }
})

let left_node = []
let all_node = []
async function scrollTree (aData) {
  all_node = aData
  var current_node = []
  var windowHeight = $(window).height()
  var defaultNum = Math.ceil(windowHeight / 24) - 2
  if (aData.length <= defaultNum) {
    current_node = aData
  } else {
    current_node = aData.slice(0, defaultNum)
    left_node = aData.slice(defaultNum, aData.length)
  }

  await refreshTree(current_node)
}

function createNewNode (aData) {
  for (var i = 0; i < aData.length; i++) {
    $("#jstree").jstree('create_node', '#', aData[i], "last", false, true)
  }
  all_node = all_node.concat(aData)
}

window.onscroll = function () {
  var scrollTop = $(this).scrollTop();
  var scrollHeight = $('.resource-tree').height();
  var windowHeight = $(this).height();
  if (left_node.length > 0 && scrollTop + windowHeight + 10 >= scrollHeight) {
    if (left_node.length <= 5) {
      createNewNode(left_node)
      left_node = []
    } else {
      createNewNode(left_node.slice(0, 5))
      left_node = left_node.slice(5, left_node.length)
    }
  }
}

window.searchTree = function () {
  let search = document.getElementById('search_input')
  let searchValue = search.value
  if (searchValue) {
    console.warn(searchValue)
    $('#jstree').jstree(true).hide_all()
    searchNode(all_node, searchValue)
  } else {
    $('#jstree').jstree(true).show_all()
  }
}

function searchNode (lists, keyword, parent = false, nodeJson = null) {
  if (parent) {
    if (nodeJson.parentId) {
      let parentNode = lists.find(v => v.id === nodeJson.parentId)
      searchNode(lists, keyword, true, parentNode)
    }
    createNode(nodeJson)
  } else {
    for (let node of lists) {
      if (node.text.indexOf(keyword) >= 0 && node.parentId) {
        searchNode(lists, keyword, true, node)
      } else if ((node.text.indexOf(keyword) >= 0 && !node.parentId)) {
        createNode(node);
      }
    }
  }
}

function createNode (text) {
  let parent = text.parentId || '#'
  let exist = $('#jstree').jstree(true).get_node(text.id)
  if (exist) {
    tr.jstree().show_node(text.id)
  } else {
    tr.jstree().create_node([parent], text)
  }
}

/*tr.on("open_node.jstree", function (e, data) {
  console.log("lala")
  //do something
});
tr.on("after_open.jstree", function (e, data) {
  console.log("lala3")
  //do something
});
tr.on("close_node.jstree", function (e, data) {
  console.log("lala2")
  //do something
});

tr.on("ready.jstree", function(e, data){
// console.log(last_open)
// if (last_open)
//  tr.jstree().open_node(last_open)
  //tr.jstree()._open_to(last_open)

})*/

/*
tr.on("changed.jstree", function(e, data){
  console.log(data.action)
  console.log(data.selected)
})*/

var cur_sel_data
tr.on("select_node.jstree", function (e, data) {
  cur_sel_data = data
})

async function isValidFolder (aNodeID) {
  if (mdl.loaded[aNodeID])
    return null
  let ids = aNodeID.split("/")
  let dt = await src[src_mode](ids)
  if (!dt.length) {
    rea.pipelines().run("js_openWorkFile", aNodeID, "", new rea.scopeCache({
      root: src_mode == "aws_sys" ? "aws0" : "",
      config: mdl.config
    }))
    return null
  }
  return dt
}

tr.on("dblclick.jstree", async function (e) {
  let node = $(e.target).closest("li")
  let dt = await isValidFolder(node[0].id)
  if (!dt)
    return
  if (node[0].id.endsWith("/.")) {
    let dir = node[0].id.split("/")
    dir.pop()
    rea.pipelines().run("openFolder_" + rea.env().tag, dir.join("/"))
    return
  }
  if (node[0].id.endsWith("/..")) {
    let dir = node[0].id.split("/")
    dir.pop()
    if (dir.length > 1)
      dir.pop()
    if (dir.length == 1)
      dir.push("")
    rea.pipelines().run("openFolder_" + rea.env().tag, dir.join("/"))
    return
  }
  mdl.loaded[node[0].id] = true
  //let prt = tr.jstree("get_selected")
  for (let i in dt) {
    if (dt[i] != "." && dt[i] != "..") {
      let ent = prepareEntry(node[0].id + "/" + dt[i], dt[i])
      let child = JSON.parse(JSON.stringify(ent))
      child.parentId = node[0].id
      all_node.push(child)
      tr.jstree().create_node([node[0].id], ent)
    }
  }
  tr.jstree().open_node([node[0].id])
})

function getSide (nodeID, next) {
  let clds = tr.jstree().get_node(nodeID).children
  if (clds.length)
    if (!next)
      return getSide(clds[clds.length - 1], next)
    else
      return clds[0]
  else
    return nodeID
}

function getLayerSibling (nodeID, next) {
  let ret = null
  let prt = tr.jstree().get_node(nodeID).parent
  let clds = tr.jstree().get_node(prt).children

  let bk = false
  for (let i in clds) {
    if (clds[i] !== nodeID) {
      ret = clds[i]
      if (bk)
        break
    } else if (!next) {
      if (ret)
        ret = getSide(ret, next)
      break
    }
    else {
      ret = null
      bk = true
    }
  }

  if (!ret && prt != "#") {
    if (!next)
      return prt
    else
      return getLayerSibling(prt, next)
  }
  return ret
}

function getSibling (nodeID, next) {
  let ret = null
  if (next) {
    ret = getSide(nodeID, next)
    if (ret != nodeID)
      return ret
  }

  return getLayerSibling(nodeID, next)
}

rea.pipelines().add(async function (aInput) {
  if (cur_sel_data) {
    let sib = getSibling(cur_sel_data.node.id, aInput.data())
    if (sib) {
      tr.jstree().deselect_all()
      tr.jstree().select_node(sib)
      await isValidFolder(sib)
    }
  }
}, {
  name: "qml_openWorkFile",
  external: "qml"
})

rea.pipelines("c++").init(function () {
  rea.pipelines().add(async function (aInput) {
    src_mode = "file_sys"
    clearCache()
    let byusr = aInput.data() == ""
    if (!byusr)
      mdl.id = aInput.data()
    let dt = await src[src_mode](byusr ? null : aInput.data().split("/"))
    if (dt) {
      let tree_dt = [prepareEntry((mdl.id == "" ? "" : mdl.id + "/") + "..", ".."),
      prepareEntry((mdl.id == "" ? "" : mdl.id + "/") + ".", ".")]
      for (let i in dt)
        if (dt[i] != "." && dt[i] != "..")
          tree_dt.push(prepareEntry((mdl.id == "" ? "" : mdl.id + "/") + dt[i], dt[i]))
      //if (tree_dt.length)
      scrollTree(tree_dt)
      if (byusr)
        rea.pipelines().run("storageOpened", {
          tag: rea.env().tag,
          mode: src_mode,
          root: mdl.id,
          config: mdl.config
        })
    }
  }, { name: "openFolder_" + rea.env().tag })
})
/*rea.pipelines().add(function(aInput){
  aInput.setData({
    local: true,
    root: "deepsight",
    ip: "172.29.1.64:9000",
    access: "deepsight",
    secret: "deepsight"
  }).out()
}, {name: "js_setParam", type: "Partial"})
*/
rea.pipelines("c++").init(function () {
  rea.pipelines().add(async function (aInput) {
    //console.log("open aws")
    src_mode = "aws_sys"
    clearCache()
    if (aInput.data() == "") {
      let stm = await rea.pipelines().input({
        title: "aws config",
        content: {
          local: {
            type: "check",
            value: true
          },
          root: {
            value: "deepsight"
          },
          ip: {
            value: "172.29.1.64:9000" //180, 243
          },
          access: {
            value: "deepsight"
          },
          ssl: {
            type: "check",
            value: false
          },
          secret: {
            value: "deepsight"
          },
        }
      }, "openaws")
        .asyncCall("js_setParam")
      mdl.config = stm.data()
    } else
      mdl.config = aInput.data()

    let dt = await src[src_mode]()
    let tree_dt = []
    for (let i in dt)
      tree_dt.push(prepareEntry((mdl.id == "" ? "" : mdl.id + "/") + dt[i], dt[i]))
    scrollTree(tree_dt)
    if (aInput.data() == "")
      rea.pipelines().run("storageOpened", {
        tag: rea.env().tag,
        mode: src_mode,
        root: mdl.id,
        config: mdl.config
      })
  }, { name: "openAWS_" + rea.env().tag })
})

rea.pipelines().add(function (aInput) {
  if (mdl)
    aInput.scope().cache("valid", true).cache("root", src_mode == "aws_sys" ? "aws0" : "").cache("config", mdl.config).cache("dir", mdl.id)
  aInput.out()
}, { name: "js_getStorageInfo", type: "Partial", external: "qml" })

rea.pipelines().add(function (aInput) {
  aInput.setData(tr.jstree("get_selected")).out()
}, { name: "js_getSelectedFiles", type: "Partial", external: "c++" })

rea.pipelines().add(function (aInput) {
  if (mdl) {
    let rt = src_mode == "aws_sys" ? "aws0" : ""
    if (rt == aInput.scope().data("root") &&
      JSON.stringify(mdl.config) == JSON.stringify(aInput.scope().data("config"))) {
      let rt0 = mdl.id == "" ? [] : mdl.id.split("/"),
        pth = aInput.data().split("/")
      let prefix = mdl.id
      for (let i = rt0.length; i < pth.length; ++i) {
        let cur = (prefix == "" ? "" : prefix + "/") + pth[i]
        if (!tr.jstree().get_node(cur))
          tr.jstree().create_node(prefix == mdl.id ? "#" : prefix, prepareEntry(cur, pth[i]))
        prefix = cur
      }
    }
  }
}, { name: "workFileSaved" })

window.onload = e => {
  rea.pipelines("qml").init(function () {
    rea.pipelines().run("reaResourceLoaded", rea.env().tag)
  })
}
