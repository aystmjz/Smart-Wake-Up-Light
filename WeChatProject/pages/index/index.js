const app = getApp();

Page({
  data: {
    device: null, // 蓝牙设备信息
    connected: false, // 连接状态
    readyRec: false, // 是否准备接收数据
    dataSynced: false, // 数据是否已同步
    Cmd: 0,
    Temp: 0, // 温度
    Hum: 0, // 湿度
    Led: false, // LED状态
    Fan: false, // 空调状态
    sendText: "", // 文本框文本
    sendExtra: "", // 发送的额外文本
    sendData: "", // 发送文本
    connectState: "未连接", // 当前连接状态
    deviceadd: "  ", // 设备MAC地址
    selectedTime: "", // 存储用户选择的时间
    alarmEnabled: false, // 闹钟开关
    selectedDays: {
      mon: false,
      tue: false,
      wed: false,
      thu: false,
      fri: false,
      sat: false,
      sun: false,
    }, // 选择的日期
    alarmTime: "07:00", // 设置的闹钟时间
    alarmModes: ["5min", "10min", "20min", "30min", "40min", "60min"], // 闹钟模式选项
    selectedMode: "5min", // 当前选择的模式
    hourlyTimeAnnouncement: false, // 整点报时
    alarmMuzicEnabled: false, // 音乐开关
    commands: [{
        command: "TIME",
        description: "网络授时",
      },
      {
        command: "RESET",
        description: "重启设备",
      },
      {
        command: "VOICE",
        description: "唤醒语音助手",
      },
      {
        command: "NAME",
        description: "更改设备名称",
      },
    ],
  },

  showCommandTips: function () {
    const commandList = this.data.commands.map(
      (item) => `${item.command} | ${item.description}`
    );

    wx.showActionSheet({
      itemList: commandList,
      success: (res) => {
        console.log("选择第" + res.tapIndex + "个命令");
        const selectedCommand = this.data.commands[res.tapIndex].command;
        if (selectedCommand === "NAME") {
          this.setData({
            sendText: selectedCommand + "+" + (this.data.device == null ? "Unnamed" : this.data.device.name),
          });
        } else {
          this.setData({
            sendText: selectedCommand,
          });
        }
      },
      fail: (res) => {
        console.log("取消操作");
      },
    });
  },

  // 页面加载时
  onShow(options) {
    if (!app.globalData.refresh) {
      return;
    }
    app.globalData.refresh = false;
    this.data.connectState = "正在连接";
    if (this.data.connected) {
      if (app.globalData.ble_device.deviceId == this.data.device.deviceId) {
        wx.showToast({
          title: "已连接",
          icon: "none",
        });
        return;
      }
      wx.closeBLEConnection({
        deviceId: this.data.device.deviceId,
        success: function (res) {
          console.log("断开蓝牙设备成功");
        },
        fail: function (res) {
          wx.showToast({
            title: "断开蓝牙设备失败",
            icon: "none",
          });
          console.log("断开蓝牙设备失败");
          return;
        },
      });
      this.setData({
        connected: false,
      });
    }
    if (this.connected) {
      wx.showToast({
        title: "重新连接请先重启小程序",
        icon: "none",
      })
      return;
    }
    //获取传递过来的设备信息
    this.data.device = app.globalData.ble_device;
    if (!this.data.device || !this.data.device.deviceId) {
      wx.showToast({
        title: "设备ID不存在",
        icon: "none",
      });
      return;
    }
    const deviceId = this.data.device.deviceId;
    this.setData({
      deviceadd: "MAC " + deviceId,
    });
    this.connectToDevice(deviceId);
  },

  // 连接蓝牙设备
  connectToDevice(deviceId) {
    wx.createBLEConnection({
      deviceId,
      success: (res) => {
        this.setData({
          connected: true,
          connectState: "已连接 " + this.data.device.name,
        });
        this.getBLEDeviceServices(deviceId);
      },
      fail: (res) => {
        console.error("蓝牙连接失败", res);
        this.setData({
          connected: false,
          connectState: "连接失败",
        });
      },
    });
  },

  // 获取蓝牙设备的服务
  getBLEDeviceServices(deviceId) {
    wx.getBLEDeviceServices({
      deviceId,
      success: (res) => {
        for (let i = 0; i < res.services.length; i++) {
          if (res.services[i].uuid === app.globalData.mserviceuuid) {
            this.getBLEDeviceCharacteristics(deviceId, res.services[i].uuid);
            return;
          }
        }
        this.setData({
          connectState: "找不到目标服务UUID",
        });
      },
    });
  },

  // 获取蓝牙设备的特征值
  getBLEDeviceCharacteristics(deviceId, serviceId) {
    wx.getBLEDeviceCharacteristics({
      deviceId,
      serviceId,
      success: (res) => {
        for (let i = 0; i < res.characteristics.length; i++) {
          const item = res.characteristics[i];
          if (item.properties.notify) {
            wx.notifyBLECharacteristicValueChange({
              deviceId,
              serviceId,
              characteristicId: item.uuid,
              state: true,
              success: (res) => {
                console.log("开启通知成功");
                this.setData({
                  readyRec: true,
                });
              },
            });
          }
        }
      },
    });

    wx.onBLECharacteristicValueChange((characteristic) => {
      const buf = new Uint8Array(characteristic.value);
      const recStr = String.fromCharCode.apply(null, buf);

      // 解析数据
      let data;
      try {
        data = JSON.parse(recStr);
      } catch (e) {
        console.log("收到非JSON数据: ", recStr);
        app.addDebugLog(recStr);
        return;
      }

      console.log("收到JSON数据:", recStr);
      // 更新主页面数据
      this.setData({
        Temp: data.Temp,
        Hum: data.Hum,
        alarmEnabled: data.Alarm,
        alarmTime: data.Time,
        selectedDays: data.Days,
        selectedMode: data.Mode,
        hourlyTimeAnnouncement: data.Buzzer,
        alarmMuzicEnabled: data.Muzic,
        dataSynced: true,
      });

      wx.showToast({
        title: "已同步",
        icon: "none",
      });
    });
  },

  sendData() {
    if (!this.data.connected) {
      this.data.dataSynced = false;
      wx.switchTab({
        url: '/pages/bluetooth/connect/connect', 
      });
      wx.showToast({
        title: "请先连接蓝牙设备",
        icon: "none",
      });
      return;
    }

    if (!this.data.dataSynced) {
      wx.showToast({
        title: "请等待数据同步",
        icon: "none",
      });
      return;
    }

    const now = new Date();

    if (this.data.sendText === "TIME") {
      // 使用本地时间戳（秒级）
      const timestamp = Math.floor(Date.now() / 1000);
      this.data.sendExtra = this.data.sendText + "+" + timestamp;
    } else {
      this.data.sendExtra = this.data.sendText;
    }

    this.data.sendData = JSON.stringify({
      Cmd: this.data.Cmd,
      Temp: this.data.Temp,
      Hum: this.data.Hum,
      Alarm: this.data.alarmEnabled,
      Time: this.data.alarmTime,
      Days: this.data.selectedDays,
      Mode: this.data.selectedMode,
      Buzzer: this.data.hourlyTimeAnnouncement,
      Muzic: this.data.alarmMuzicEnabled,
      Extra: this.data.sendExtra,
    });

    console.log("待发送数据数据:" + this.data.sendData);

    const buffer = new ArrayBuffer(this.data.sendData.length);
    const dataView = new DataView(buffer);
    for (let i = 0; i < this.data.sendData.length; i++) {
      dataView.setUint8(i, this.data.sendData.charCodeAt(i));
    }

    wx.writeBLECharacteristicValue({
      deviceId: this.data.device.deviceId,
      serviceId: app.globalData.mserviceuuid,
      characteristicId: app.globalData.mtxduuid,
      value: buffer,
      success: (res) => {
        if (this.data.sendText === "TIME" && this.data.Cmd == 0) {
          wx.showModal({
            title: "同步时间成功",
            content: now.toString(),
            showCancel: false,
            confirmText: "确定",
          });
        } else if (this.data.sendText === "RESET" && this.data.Cmd == 0) {
          wx.switchTab({
            url: '/pages/debug/debug',
          });
          wx.showToast({
            title: "设备正在重启",
            icon: "none",
          });
        } else if (this.data.sendText === "VOICE" && this.data.Cmd == 0) {
          wx.showModal({
            title: "语音助手已唤醒",
            showCancel: false,
            confirmText: "确定",
          });

        } else if (this.data.sendText.includes("NAME") && this.data.Cmd == 0) {
          wx.showModal({
            title: "已更改设备名称",
            content: "请重新连接设备",
            showCancel: false,
            confirmText: "确定",
          });
        } else {
          wx.showToast({
            title: "数据发送成功",
            icon: "none",
          });
        }
        if (this.data.Cmd == 0) {
          this.setData({
            sendText: "",
          });
        }
        console.log("数据发送成功", res);
        this.data.connected = true;
      },
      fail: (res) => {
        wx.showToast({
          title: "数据发送失败",
          icon: "none",
        });
        console.error("数据发送失败", res);
        this.data.connected = false;
        this.selectComponent()
      },
    });
  },

  // 处理文本输入
  onCommandInput(e) {
    this.data.sendText = e.detail.value;
  },

  syncData() {
    this.data.Cmd = 0;
    this.sendData();
  },

  onLedChange(event) {
    const ledStatus = event.detail.value;
    this.setData({
      Led: ledStatus,
    });
    this.data.Cmd = 1;
    this.sendData();
  },

  onFanChange(event) {
    const fanStatus = event.detail.value;
    this.setData({
      Fan: fanStatus,
    });
    this.data.Cmd = 2;
    this.sendData();
  },

  // 时间选择改变时的回调函数
  onTimeChange(event) {
    const time = event.detail.value; // 获取用户选择的时间
    this.setData({
      selectedTime: time, // 更新数据
    });
  },

  // 闹钟开关变化
  onAlarmSwitchChange(e) {
    this.setData({
      alarmEnabled: e.detail.value,
    });
  },

  // 日期选择变化
  onWeekdaysChange(e) {
    const selectedDays = e.detail.value.reduce((acc, day) => {
      acc[day] = true;
      return acc;
    }, {});

    this.setData({
      selectedDays,
    });
  },

  // 时间选择变化
  onTimeChange(e) {
    this.setData({
      alarmTime: e.detail.value,
    });
  },

  // 闹钟模式选择变化
  onModeChange(e) {
    this.setData({
      selectedMode: this.data.alarmModes[e.detail.value],
    });
  },

  // 整点报时开关变化
  hourlyTimeAnnouncement(e) {
    this.setData({
      hourlyTimeAnnouncement: e.detail.value,
    });
  },

  // 音乐开关开关变化
  onAlarmMuzicChange(e) {
    this.setData({
      alarmMuzicEnabled: e.detail.value,
    });
  },
});