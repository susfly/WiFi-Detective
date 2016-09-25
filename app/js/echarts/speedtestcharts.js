console.log(__dirname + '/js/echarts/rj-speedtest-net.js')
var speedTest = require(__dirname + '/js/echarts/rj-speedtest-net.js');
var speedtestchart = echarts.init(document.getElementById("speed-test-chart"));

var updateGauge = function (speed) {
    var option_tmp = speedtestchart.getOption();
    option_tmp.series[0].data[0].value = speed.toFixed(2);
    speedtestchart.setOption(option_tmp);
};

$("#a_speedtest_btn").click(function(){
    console.log("start to test");
    $("#speedtest-ping").html("N/A");
    $("#speedtest-download").html("N/A");
    $("#speedtest-upload").html("N/A");
    $("#speedtest-loading").css("display", "block");
    test = speedTest({maxTime: 10000});
    console.log(test);
    test.on('data', function(data) {
      console.log("speedtest data callback");
      console.dir(data);
    });

    test.on('error', function(err) {
       if (err.code === 'ENOTFOUND') {
           console.error(logSymbols.error, 'Please check your internet connection');
	   } else {
	       console.error(err);
	   }
    });

    test.on('done', function () {
        console.log("speedtest done");
    });

    test.on('downloadspeed', function(speed) {
      console.log('Download speed: ', speed.toFixed(2) + 'Mbps');
      updateGauge(speed);
    });

    test.on('uploadspeed', function(speed) {
      console.log('Upload speed: ', speed.toFixed(2) + 'Mbps');
      updateGauge(speed);
      $("#speedtest-loading").css("display", "none");
      $("#speed-test-btn-div").css("display", "table");
      $("#speed-testing").html("正在测试...");
      updateGauge(0);
    });

    test.on('testserver', function(server) {
      $("#speed-test-btn-div").css("display", "none");
      console.log('Using server by ' + server.sponsor + ' in ' + server.name + ', ' + server.country + ' (' + (server.distMi * 0.621371).toFixed(0) + 'mi, ' + (server.bestPing).toFixed(0) + 'ms)');
      $("#speedtest-ping").html((server.bestPing).toFixed(0) + ' ms');
      $("#speed-testing").html("正在测试 " + server.sponsor + ' in ' + server.name + ', ' + server.country + ' (' + (server.distMi * 0.621371).toFixed(0) + 'mi)' + '...');
    });

    test.on('downloadspeedprogress', function (speed) {
        console.log("downloadspeedprogress " + speed);
        $("#speedtest-download").html(speed.toFixed(2) + " Mbps");
        updateGauge(speed);
    });

    test.on('uploadspeedprogress', function (speed) {
        console.log("uploadspeedprogress " + speed);
        $("#speedtest-upload").html(speed.toFixed(2) + " Mbps");
        updateGauge(speed);
    });
    test.on('downloadprogress', function(progress) {
        console.log('Download progress:', progress);
    });
    test.on('uploadprogress', function(progress) {
        console.log('Upload progress:', progress);
    });
    test.on('bestservers', function(servers) {
        console.log('Closest servers:');
        console.dir(servers);
    });
});

option_st = {
    tooltip : {
        formatter: "{a} <br/>{c} {b}"
    },
    toolbox: {
        show : true,
        feature : {
            mark : {show: false},
            restore : {show: false},
            saveAsImage : {show: false}
        }
    },
    series : [
        {
            name: '速度',
            type: 'gauge',
            z: 3,
            min: 0,
            max: 100,
            splitNumber: 10,
            radius: '70%',
            axisLine: {            // 坐标轴线
                lineStyle: {       // 属性lineStyle控制线条样式
                    width: 10
                }
            },
            axisTick: {            // 坐标轴小标记
                length: 15,        // 属性length控制线长
                lineStyle: {       // 属性lineStyle控制线条样式
                    color: 'auto'
                }
            },
            splitLine: {           // 分隔线
                length: 20,         // 属性length控制线长
                lineStyle: {       // 属性lineStyle（详见lineStyle）控制线条样式
                    color: 'auto'
                }
            },
            title : {
                textStyle: {       // 其余属性默认使用全局文本样式，详见TEXTSTYLE
                    fontWeight: 'bolder',
                    fontSize: 20,
                    fontStyle: 'italic'
                }
            },
            detail : {
                textStyle: {       // 其余属性默认使用全局文本样式，详见TEXTSTYLE
                    fontWeight: 'bolder'
                }
            },
            data:[{value: 0, name: 'Mbps'}]
        }
    ]
};

speedtestchart.setOption(option_st);
