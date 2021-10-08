const path = require('path')
const HtmlWebpackPlugin = require('html-webpack-plugin')
const CopyWebpackPlugin = require('copy-webpack-plugin')
const glob = require("glob")

const srcDir = path.resolve(__dirname, './src');
const distDir = path.resolve(__dirname, './public');
const plugins = [];

//https://blog.csdn.net/wanglei1991gao/article/details/79541551
function getEntry() {

  let files = glob.sync(srcDir+'/js/*.js'),
      entry = {},
      entryFileName,
      outputHtmlName;

  for(let i = 0; i < files.length; i++){
      let matchs = /js\/(\S*).js/.exec(files[i]);
      entryFileName = outputHtmlName = matchs[1]; //得到apps/question/index这样的文件名
      if(/^_\w*/.test(entryFileName) || /\/_\w*/.test(entryFileName))
      {
          continue;
      }
      entryFileName  =  'js/'+entryFileName;
      entry[entryFileName] = files[i]
      //生成html配置
      plugins.push(new HtmlWebpackPlugin({
          // 生成出来的html文件名
          filename: distDir + '/html/' + outputHtmlName + '.html',
          // 每个html的模版，这里多个页面使用同一个模版
          template: srcDir + '/html/' + outputHtmlName + '.html',
          // 自动将引用插入body
          inject: 'body',
          title:outputHtmlName,
          // 每个html引用的js模块，也可以在这里加上vendor等公用模块
          chunks: [entryFileName]
      }));
  }

  console.log('> entry' + JSON.stringify(entry))
  plugins.push(new CopyWebpackPlugin({
    patterns: [{
      from: srcDir + "/assets",
      to: distDir + "/assets"
    }]
  }))
  return entry;
}

module.exports = {
    devtool: 'eval-source-map',
    entry:  getEntry(),
    output: {
      path: __dirname + "/public",//打包后的文件存放的地方
      filename: "[name].js"//"[name]_[chunkhash:8].js"//打包后输出文件的文件名
    },
    devServer: {
      contentBase: "./public",//本地服务器所加载的页面所在的目录
      historyApiFallback: true,//不跳转
      inline: true//实时刷新
    },
    module: {
      rules: [
        {
          test: /\.(scss|css)$/,
          use: ["style-loader", "css-loader", "sass-loader"], //从右往左加载
        },
        {
          test: /\.(gif|png|jpg)$/,
          loader: "url-loader"
        },
        {
          test: /\.jsx?$/, // jsx/js文件的正则
          exclude: /node_modules/, // 排除 node_modules 文件夹
          use: {
          // loader 是 babel
            loader: 'babel-loader',
            options: {
              // babel 转义的配置选项
              babelrc: false,
              presets: [
                // 添加 preset-react
                require.resolve('@babel/preset-react'),
                [require.resolve('@babel/preset-env'), {modules: false}]
              ],
              cacheDirectory: true
            }
          }
         }
      ]
    },
    plugins: plugins,
    watch: true
  }