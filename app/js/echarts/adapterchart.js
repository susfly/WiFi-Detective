// 基于准备好的dom，初始化echarts实例
var adapterChart = echarts.init(document.getElementById('adapterchart'));

// 指定图表的配置项和数据
// 指定图表的配置项和数据
var labelTop = {
    normal : {
        label : {
            show : true,
            position : 'center',
        },
        labelLine : {
            show : false
        }
    }
};
var labelFromatter = {
    normal : {
        label : {
            textStyle: {
                fontSize: 16
            }
        }
    },
}
var labelBottom = {
    normal : {
        labelLine : {
            show : false
        }
    },
    emphasis: {
        color: 'rgba(0,0,0,0)'
    }
};
var radius = [60, 55];

option_adapter = {
    color:['#DF9499','#66CEA7','#97B552'],
    series : [
        {
            type : 'pie',
            center : ['50%', '50%'],
            radius : radius,
            x: '0%',
            itemStyle : labelFromatter,
            data : [
                {name:'', value:10, itemStyle : labelBottom},
                {name:'好', value:90, itemStyle : labelTop}
            ]
        }
    ]
};

// 使用刚指定的配置项和数据显示图表。
adapterChart.setOption(option_adapter);
