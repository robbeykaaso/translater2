global.rea = require('reajs')
require("./text/style.scss")

var ace = require('brace');
require('brace/mode/html');
require('brace/mode/css');
require('brace/mode/javascript');
require('brace/mode/json');
require('brace/theme/solarized_light');
 
var editor = ace.edit('editor');
editor.getSession().setMode('ace/mode/html');
editor.setTheme('ace/theme/solarized_light');

var qljs = document.querySelector('#ql-js')
qljs.addEventListener('click', function() {
    try  {
        eval(editor.getValue())
    }catch(exception) {
        alert(exception)
    }
})

var qlpy = document.querySelector('#ql-py')
qlpy.addEventListener('click', function() {
    rea.pipelines().run("evalPyScript0", editor.getValue(), "", new rea.scopeCache({remote: true}))
})

rea.pipelines("c++").init(function(){
    rea.pipelines().add(function(aInput){
        editor.setValue(aInput.data())
        var type = JSON.parse(aInput.data()) ? "json" : aInput.scope().data("suffix")
        switch(type) {
            case 'javascript': editor.getSession().setMode('ace/mode/javascript'); break;
            case 'scss':
            case 'css': editor.getSession().setMode('ace/mode/css'); break;
            case 'html': editor.getSession().setMode('ace/mode/html'); break;
            case 'json': editor.getSession().setMode('ace/mode/json'); break;
            default: break;
        }
        // if (type == "json"){
        //     editor.formatLine(0, editor.getLength(), "code-block", true)
        // }
        aInput.out()
    }, {name: "updateQuillAttr_" + rea.env().tag})
})

rea.pipelines("c++").init(function(){
    rea.pipelines().add(function(aInput){
        aInput.setData(editor.getValue()).out()
    }, {name: "getQuillText_" + rea.env().tag,
        type: "Partial",
        external: "c++"})
})