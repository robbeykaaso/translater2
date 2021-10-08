//global.$ = require('jquery')
global.rea = require('reajs')
require("./text/style.scss")

require("highlight.js/styles/monokai-sublime.css")
global.hljs = require("highlight.js")

hljs.configure({   // optionally configure hljs
    languages: ['javascript', 'python', 'json']
});

require("quill/dist/quill.snow.css")
let quill = require("quill");
const rea = require('reajs');

var toolbarOptions = [
    ['bold', 'italic', 'underline', 'strike'],        // toggled buttons
    ['blockquote', 'code-block'],
  
//    [{ 'header': 1 }, { 'header': 2 }],               // custom button values
//    [{ 'list': 'ordered'}, { 'list': 'bullet' }],
//    [{ 'script': 'sub'}, { 'script': 'super' }],      // superscript/subscript
//    [{ 'indent': '-1'}, { 'indent': '+1' }],          // outdent/indent
//    [{ 'direction': 'rtl' }],                         // text direction
  
//    [{ 'size': ['small', false, 'large', 'huge'] }],  // custom dropdown
//    [{ 'header': [1, 2, 3, 4, 5, 6, false] }],
  
//    [{ 'color': [] }, { 'background': [] }],          // dropdown with defaults from theme
//    [{ 'font': [] }],
//    [{ 'align': [] }],
    ['link', 'image', 'video']
  ];

var editor = new quill('#editor', {
    modules: {
        syntax: true,
        toolbar: "#toolbar"//toolbarOptions
    },
    placeholder: "edit...",
    theme: 'snow'
  });

//algebra
//alert(mth.simplify('2 * 3 * x', { x: 4 }).toString()) // '24'
//alert(mth.simplify('x^2 + x + 3 + x^2').toString()) // '2 * x ^ 2 + x + 3'
//alert(mth.simplify('x * y * -x / (x ^ 2)').toString()) // '-y'
//alert(mth.derivative('2x^2 + 3x + 4', 'x').toString()) // '4 * x + 3'
//alert(mth.derivative('sin(2x)', 'x').toString()) // '2 * cos(2 * x)'
//custom algebra
// set(`s1 + 3 = s2
// s2 + 3 + s5 = s3
// s2 + s1 = s4`)
// get(["s2", "s3", "s4"], {s1: 3, s5: 4})
//html control
// rea.pipelines().run("updateHtmlAttr_region_edt_gridder0_ide_html", "https://www.baidu.com", "", new rea.scopeCache({type: "url"}))
// var auto_input = `var input = document.getElementsByClassName("s_ipt")[0];
//                       input.value = "hello world";
// 					     document.getElementById('su').click();`
// setTimeout(function(){
//  rea.pipelines().run("updateHtmlAttr_region_edt_gridder0_ide_html", auto_input, "", new rea.scopeCache({type: "script"}))
// }, 1000);

var mth = require("mathjs");
const { pipelines } = require('reajs');
var exprs = {}
function set(aData){
    let dts = aData.split("\n")
    for (let i in dts){
        let dt = dts[i].split("=")
        if (dt.length == 2)
            exprs[dt[1].trim()] = dt[0].trim()
    }
}

function calcRecursive(aExpr, aSymbol){
    let vs = aExpr.split(" "), ss = {}
    for (let j in vs){
        if (exprs[vs[j]])
            aSymbol[vs[j]] = calcRecursive(exprs[vs[j]], aSymbol)
        ss[vs[j]] = aSymbol[vs[j]]
    }
    return mth.parse(aExpr).evaluate(ss)
}

function get(aTarget, aSymbol){ //aTarget = [s1, s2, ...], aSymbol = {x1: xxx, x2: xxx}
    let ret = ""
    for (let i in aTarget){
        let v = aTarget[i]
        if (exprs[v])
            ret += v + " = " + calcRecursive(exprs[v], aSymbol) + "\n"
    }
    alert(ret)
}

var qljs = document.querySelector('#ql-js')
qljs.addEventListener('click', function() {
    try  {
        eval(editor.getText())
    }catch(exception) {
        alert(exception)
    }
})

var qlpy = document.querySelector('#ql-py')
qlpy.addEventListener('click', function() {
    rea.pipelines().run("evalPyScript0", editor.getText(), "", new rea.scopeCache({remote: true}))
})

rea.pipelines("c++").init(function(){
    rea.pipelines().add(function(aInput){
        editor.setText(aInput.data())
        if (aInput.scope().data("suffix") == "json"){
            editor.formatLine(0, editor.getLength(), "code-block", true)
        }
        aInput.out()
    }, {name: "updateQuillAttr_" + rea.env().tag})
})

rea.pipelines("c++").init(function(){
    rea.pipelines().add(function(aInput){
        aInput.setData(editor.getText()).out()
    }, {name: "getQuillText_" + rea.env().tag,
        type: "Partial",
        external: "c++"})
})

/*rea.pipelines("c++").init(function(){
    rea.pipelines().find(rea.env().pip + "_doCommand").nextF(function(aInput){
        console.log(aInput.data())
        if (aInput.data())
            editor.history.redo()
        else
            editor.history.undo()
    })
})*/