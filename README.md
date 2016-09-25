# WiFi-Detective
  这个项目的目的在于能够将wifi的基本信息及无线网络体验整合并直观呈现出来。

  采用electron + nodejs框架开发。

  项目定位目标人群：无线工作者和喜欢倒腾无线的人
  
# 下载地址
  32位
  
  64位

# 1.0.0 版本主打产品功能：
    无线网卡和网络的清晰和可视化

  功能点一：
      当前连接网络的ssid，bssid，信道，及信号强度
  功能点二：    
      实时显示当前的信号强度、速率、空口时延、上行流量、下行流量、同频干扰数目
  功能点三：
      呈现当前网卡的基本信息，包括网卡名称、网卡支持频段、最高速率、网关IP、当前IP地址等
  功能点四：
      记录网卡可能影响用户体验的关键网络事件，包括无线网络掉线，无线漫游事件，ip获取慢等问题
  功能点五：
      集成业界专业的speedtest测速功能，测速更加准确。
    
# 32位编译 & 打包方法：
    npm install
    npm run-script prepublish-ia32
    npm run-script package-ia32

# 64位编译 & 打包方法
    npm install
    npm run-script prepublish-ia32
    npm run-script package-ia32
    

