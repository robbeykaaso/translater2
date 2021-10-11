global.rea = require('reajs')
require("./text/style.scss")

var ace = require('brace');
require('brace/mode/html');
require('brace/mode/css');
require('brace/mode/javascript');
require('brace/mode/json');
require('brace/theme/solarized_light');
require('brace/ext/searchbox')

var editor = ace.edit('editor');
editor.getSession().setMode('ace/mode/html');
editor.setTheme('ace/theme/solarized_light');
//editor.execCommand('find')

var qljs = document.querySelector('#ql-js')
qljs.addEventListener('click', function () {
    try {
        eval(editor.getValue())
    } catch (exception) {
        alert(exception)
    }
})

var qlpy = document.querySelector('#ql-py')
qlpy.addEventListener('click', function () {
    rea.pipelines().run("evalPyScript0", editor.getValue(), "", new rea.scopeCache({ remote: true }))
})

rea.pipelines("c++").init(function () {
    rea.pipelines().add(function (aInput) {
        var type = JSON.parse(aInput.data()) ? "json" : aInput.scope().data("suffix")
        if (type === 'json') {
            editor.setValue(formatJson(aInput.data()))
        } else {
            editor.setValue(aInput.data())
        }
        switch (type) {
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
    }, { name: "updateQuillAttr_" + rea.env().tag })
})

rea.pipelines("c++").init(function () {
    rea.pipelines().add(function (aInput) {
        aInput.setData(editor.getValue()).out()
    }, {
        name: "getQuillText_" + rea.env().tag,
        type: "Partial",
        external: "c++"
    })
})

function transitionJsonToString (jsonObj) {
    var _jsonObj = null;
    if (Object.prototype.toString.call(jsonObj) !== "[object String]") {
        try {
            _jsonObj = JSON.stringify(jsonObj);
        } catch (error) {
            console.error(error);
        }
    } else {
        try {
            jsonObj = jsonObj.replace(/(\')/g, '\"');
            _jsonObj = JSON.stringify(JSON.parse(jsonObj));
        } catch (error) {
            console.error(error);
        }
    }
    return _jsonObj;
}

function formatJson (jsonObj) {
    var formatted = '';
    var pad = 0;
    var PADDING = '    ';
    var jsonString = transitionJsonToString(jsonObj);
    if (!jsonString) {
        return jsonString;
    }
    var _index = [];
    var _indexStart = null;
    var _indexEnd = null;
    var jsonArray = [];
    jsonString = jsonString.replace(/([\{\}])/g, '\r\n$1\r\n');
    jsonString = jsonString.replace(/([\[\]])/g, '\r\n$1\r\n');
    jsonString = jsonString.replace(/(\,)/g, '$1\r\n');
    jsonString = jsonString.replace(/(\r\n\r\n)/g, '\r\n');
    jsonString = jsonString.replace(/\r\n\,/g, ',');
    jsonArray = jsonString.split('\r\n');
    jsonArray.forEach(function (node, index) {
        var num = node.match(/\"/g) ? node.match(/\"/g).length : 0;
        if (num % 2 && !_indexStart) {
            _indexStart = index
        }
        if (num % 2 && _indexStart && _indexStart != index) {
            _indexEnd = index
        }
        if (_indexStart && _indexEnd) {
            _index.push({
                start: _indexStart,
                end: _indexEnd
            })
            _indexStart = null
            _indexEnd = null
        }
    })
    _index.reverse().forEach(function (item, index) {
        var newArray = jsonArray.slice(item.start, item.end + 1)
        jsonArray.splice(item.start, item.end + 1 - item.start, newArray.join(''))
    })
    jsonString = jsonArray.join('\r\n');
    jsonString = jsonString.replace(/\:\r\n\{/g, ':{');
    jsonString = jsonString.replace(/\:\r\n\[/g, ':[');
    jsonArray = jsonString.split('\r\n');
    jsonArray.forEach(function (item, index) {
        console.log(item)
        var i = 0;
        var indent = 0;
        var padding = '';
        if (item.match(/\{$/) || item.match(/\[$/)) {
            indent += 1
        } else if (item.match(/\}$/) || item.match(/\]$/) || item.match(/\},$/) || item.match(/\],$/)) {
            if (pad !== 0) {
                pad -= 1
            }
        } else {
            indent = 0
        }
        for (i = 0; i < pad; i++) {
            padding += PADDING
        }
        formatted += padding + item + '\r\n'
        pad += indent
    })
    return formatted.trim();
}
