// pages/debug/debug.js
const app = getApp();

Page({
  data: {
    device: null,
    connected: false,
    logText: "", // 调试信息文本
    sendText: "", // 发送的文本
    connectState: "未连接",
    autoScroll: true,
    scrollTop: 0,
    darkMode: false // 是否使用暗色主题
  },

  onLoad(options) {
    // 页面加载时检查连接状态
    if (app.globalData.ble_device) {
      this.setData({
        device: app.globalData.ble_device,
        connected: true,
        connectState: "已连接 " + (app.globalData.ble_device.name || "")
      });
    }
  },

  onShow() {
    // 页面显示时，如果全局有设备信息则更新
    if (app.globalData.ble_device) {
      this.setData({
        device: app.globalData.ble_device,
        connected: true,
        connectState: "已连接 " + (app.globalData.ble_device.name || "")
      });
    }
    
    // 页面显示时更新日志显示
    this.updateLogDisplay();
  },

  // 更新日志显示
  updateLogDisplay() {
    const logText = app.getDebugLogs();
    this.setData({
      logText: logText
    });
    
    // 自动滚动到底部
    if (this.data.autoScroll) {
      setTimeout(() => {
        this.setData({
          scrollTop: 999999
        });
      }, 100);
    }
  },

  // 添加日志信息（用于页面内添加）
  addLog(log) {
    app.addDebugLog(log);
  },

  // 接收蓝牙数据的处理函数（由主页面调用）
  onBLEDataReceived(data) {
    app.addDebugLog(data);
    // 确保当前页面显示更新
    this.updateLogDisplay();
  },

  // 发送数据
  sendData() {
    if (!this.data.connected || !this.data.device) {
      wx.switchTab({
        url: '/pages/bluetooth/connect/connect',
      });
      wx.showToast({
        title: "请先连接蓝牙设备",
        icon: "none"
      });
      return;
    }

    if (!this.data.sendText.trim()) {
      wx.showToast({
        title: "请输入要发送的内容",
        icon: "none"
      });
      return;
    }

    const buffer = new ArrayBuffer(this.data.sendText.length);
    const dataView = new DataView(buffer);
    for (let i = 0; i < this.data.sendText.length; i++) {
      dataView.setUint8(i, this.data.sendText.charCodeAt(i));
    }

    wx.writeBLECharacteristicValue({
      deviceId: this.data.device.deviceId,
      serviceId: app.globalData.mserviceuuid,
      characteristicId: app.globalData.mtxduuid,
      value: buffer,
      success: (res) => {
        app.addDebugLog("发送成功: " + this.data.sendText);
        this.setData({
          sendText: ""
        });
        this.updateLogDisplay();
      },
      fail: (res) => {
        app.addDebugLog("发送失败: " + res.errMsg);
        this.updateLogDisplay();
        wx.showToast({
          title: "数据发送失败",
          icon: "none"
        });
      }
    });
  },

  // 处理输入框变化
  onSendTextChange(e) {
    this.setData({
      sendText: e.detail.value
    });
  },

  // 清空日志
  clearLog() {
    app.clearDebugLogs();
    this.setData({
      logText: ""
    });
  },

  // 复制日志到剪贴板
  copyLog() {
    if (!this.data.logText) {
      wx.showToast({
        title: "没有日志可复制",
        icon: "none"
      });
      return;
    }

    wx.setClipboardData({
      data: this.data.logText,
      success: () => {
        wx.showToast({
          title: "已复制到剪贴板",
          icon: "success"
        });
      }
    });
  },

  // 切换自动滚动
  toggleAutoScroll() {
    this.setData({
      autoScroll: !this.data.autoScroll
    });
    wx.showToast({
      title: this.data.autoScroll ? "已开启自动滚动" : "已关闭自动滚动",
      icon: "none"
    });
  },

  // 切换暗色主题
  toggleDarkMode() {
    this.setData({
      darkMode: !this.data.darkMode
    });
  }
});