// 基于准备好的dom，初始化echarts实例
 var eventpieChart = echarts.init(document.getElementById('eventpiechart'));
 isEventPieChartInit = true;

 // 指定图表的配置项和数据
 option_event_pie = {
     tooltip: {
         trigger: 'item',
         formatter: "{a} <br/>{b}: {c} ({d}%)"
     },
     legend: {
         orient: 'vertical',
         x: 'right',
         y: 'center',
         data:['N/A'],
         textStyle: {
           color: '#333',
           fontSize: '12',
         }
     },
     color:['#2EC7C9', '#B6A2DE','#5AB1EF','#FFB980','#DF9499','#97B552'],
     series: [
         {
             name:'访问来源',
             type:'pie',
             radius: ['70%', '80%'],
             center: ['23%', '50%'],
             avoidLabelOverlap: false,
             label: {
                 normal: {
                     show: false,
                     position: 'center'
                 },
                 emphasis: {
                     show: false,
                     textStyle: {
                         fontSize: '30',
                         fontWeight: 'bold'
                     }
                 }
             },
             labelLine: {
                 normal: {
                     show: false
                 }
             },
             data:[
                 {value:0, name:'N/A'}
             ]
         }
     ]
 };

eventpieChart.setOption(option_event_pie);

var refreshEventPie = function () {
    console.log(wifidb);
    if (mainGuid == null) {
        return;
    }
    if (wifidb == null || wifidb == undefined) {
        return;
    }
    wifidb.forAllEventStatWeek(function(err, data) {
        if (err) {
            console.log(err);
            return;
        }
        if (data.length == 0) {
            eventpieChart.setOption(option_event_pie);
            return;
        }
        var legend_data = [];
        for (var i in data) {
            legend_data[i] = data[i].name;
        }
        option_event_pie.series[0].data = data;
        option_event_pie.legend.data = legend_data;
         // 使用刚指定的配置项和数据显示图表。
        eventpieChart.setOption(option_event_pie);
    });
};

refreshEventPie();
