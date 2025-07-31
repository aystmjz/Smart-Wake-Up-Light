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
    darkMode: false, // 是否使用暗色主题
    xmodemState: "idle", // XMODEM状态: idle, waiting, sending, complete
    xmodemBlockNumber: 1, // 当前块编号
    selectedFile: null, // 选定的文件
    fileContent: null, // 文件内容
    filePosition: 0, // 文件当前位置
    progress: 0, //传输进度
    transferStartTime: 0, //传输开始时间
    transferTime: 0, //传输时间
    xmodemMode: "checksum", // XMODEM模式: checksum 或 crc
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
    //app.addDebugLog(log);
  },

  // 接收蓝牙数据的处理函数（由主页面调用）
  onBLEDataReceived(data) {
    app.addDebugLog(data);
    // 确保当前页面显示更新
    this.updateLogDisplay();

    // 处理XMODEM传输相关的响应
    this.handleXmodemResponse(data);
  },

  // 处理XMODEM响应
  handleXmodemResponse(data) {
    // 检查是否处于XMODEM传输状态
    if (this.data.xmodemState !== "idle" && this.data.xmodemState !== "complete") {
      const percent = Math.floor(this.data.filePosition / this.data.selectedFile.size * 100);
      this.setData({
        progress: percent
      });

      // 解析响应数据
      let response = null;

      // 如果是字符串，直接使用
      if (typeof data === 'string') {
        response = data.charCodeAt(0);
      }
      // 如果是对象且有data属性
      else if (typeof data === 'object' && data.data) {
        // 尝试获取第一个字节作为响应
        if (data.data instanceof ArrayBuffer) {
          const uint8Array = new Uint8Array(data.data);
          response = uint8Array[0];
        }
      }

      // 处理XMODEM响应
      if (response !== null) {
        switch (response) {
          case 0x43: // 'C' - 使用CRC模式
            this.addLog("XMODEM: 收到'C'，使用CRC模式");
            this.setData({
              xmodemMode: "crc",
              transferStartTime: Date.now()
            });
            this.sendNextXmodemBlock();
            break;
          case 0x06: // ACK
            this.addLog("XMODEM: 收到ACK确认");
            this.sendNextXmodemBlock();
            break;
          case 0x15: // NAK
            this.addLog("XMODEM: 收到NAK重传请求");
            this.resendCurrentXmodemBlock();
            break;
          case 0x18: // CAN
            this.addLog("XMODEM: 收到CAN取消传输信号");
            this.cancelXmodemTransfer();
            break;
          default:
            this.addLog("XMODEM: 收到未知响应: 0x" + response.toString(16));
        }
      }
    }
  },

  // 选择文件
  chooseFile() {
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

    wx.chooseMessageFile({
      count: 1,
      type: 'file',
      success: (res) => {
        const tempFilePath = res.tempFiles[0].path;
        const fileName = res.tempFiles[0].name;
        const fileSize = res.tempFiles[0].size;

        // 读取文件内容
        wx.getFileSystemManager().readFile({
          filePath: tempFilePath,
          success: (readRes) => {
            this.setData({
              selectedFile: {
                name: fileName,
                size: fileSize,
                path: tempFilePath
              },
              fileContent: readRes.data
            });
            this.addLog("已选择文件: " + fileName + " (" + fileSize + " 字节)");
          },
          fail: (err) => {
            this.addLog("读取文件失败: " + JSON.stringify(err));
            wx.showToast({
              title: "读取文件失败",
              icon: "none"
            });
          }
        });
      },
      fail: (err) => {
        this.addLog("选择文件失败: " + JSON.stringify(err));
        wx.showToast({
          title: "选择文件失败",
          icon: "none"
        });
      }
    });
  },

  // 开始XMODEM传输
  startXmodemTransfer() {
    if (!this.data.selectedFile || !this.data.fileContent) {
      wx.showToast({
        title: "请先选择文件",
        icon: "none"
      });
      return;
    }
    if (this.data.xmodemState === "idle") {
      this.setData({
        xmodemState: "waiting",
        xmodemBlockNumber: 1,
        filePosition: 0,
        xmodemMode: "checksum" // 默认使用校验和模式
      });

      this.addLog("开始XMODEM传输，等待接收方响应...");
      const buffer = new ArrayBuffer(1);
      const bufferView = new DataView(buffer);
      bufferView.setUint8(0, 0x65); //发送字符A
      wx.writeBLECharacteristicValue({
        deviceId: this.data.device.deviceId,
        serviceId: app.globalData.mserviceuuid,
        characteristicId: app.globalData.mtxduuid,
        value: buffer
      });
    } else if (this.data.xmodemState === "complete") {
      this.setData({
        xmodemState: "idle",
        selectedFile: null,
        fileContent: null
      });
    } else {
      this.cancelXmodemTransfer();
    }
    // 监听来自设备的响应，等待'C'或NAK信号开始传输
    // 实际的传输将在handleXmodemResponse中处理
  },

  // 计算校验和
  calculateChecksum(data) {
    let checksum = 0;
    for (let i = 0; i < data.length; i++) {
      checksum = (checksum + data[i]) & 0xFF;
    }
    return checksum;
  },

  // 计算CRC-16校验值 (CCITT标准)
  calculateCRC16(data) {
    let crc = 0;
    for (let i = 0; i < data.length; i++) {
      crc ^= (data[i] << 8);
      for (let j = 0; j < 8; j++) {
        if (crc & 0x8000) {
          crc = (crc << 1) ^ 0x1021;
        } else {
          crc <<= 1;
        }
        crc &= 0xFFFF;
      }
    }
    return crc;
  },

  // 发送下一个XMODEM数据块
  sendNextXmodemBlock() {
    if (!this.data.fileContent) {
      this.addLog("XMODEM: 文件内容为空");
      return;
    }

    if (this.data.filePosition >= this.data.selectedFile.size) {
      this.addLog("XMODEM: 所有数据已发送，等待接收方确认...");
      this.sendEOT();
      this.completeXmodemTransfer();
      return;
    }

    // 准备数据块
    const blockNumber = this.data.xmodemBlockNumber;
    const blockData = new Uint8Array(128);

    // 填充数据块
    let dataBytes = 0;
    for (let i = 0; i < 128; i++) {
      if (this.data.filePosition + i < this.data.selectedFile.size) {
        // 从文件中读取数据
        if (this.data.fileContent instanceof ArrayBuffer) {
          const fileBytes = new Uint8Array(this.data.fileContent);
          blockData[i] = fileBytes[this.data.filePosition + i];
        } else {
          blockData[i] = this.data.fileContent[this.data.filePosition + i];
        }
        dataBytes++;
      } else {
        // 使用EOF字符(0x1A)填充剩余空间
        blockData[i] = 0x1A;
      }
    }

    if (this.data.xmodemMode === "crc") {
      // 使用CRC模式
      const crc = this.calculateCRC16(blockData);

      // 构造XMODEM帧: SOH + 块编号 + 块编号取反 + 数据 + CRC高字节 + CRC低字节
      const frame = new Uint8Array(133);
      frame[0] = 0x01; // SOH
      frame[1] = blockNumber & 0xFF; // 块编号
      frame[2] = (~blockNumber) & 0xFF; // 块编号取反
      frame.set(blockData, 3); // 数据
      frame[131] = (crc >> 8) & 0xFF; // CRC高字节
      frame[132] = crc & 0xFF; // CRC低字节

      // 发送数据块
      const buffer = frame.buffer;
      wx.writeBLECharacteristicValue({
        deviceId: this.data.device.deviceId,
        serviceId: app.globalData.mserviceuuid,
        characteristicId: app.globalData.mtxduuid,
        value: buffer,
        success: (res) => {
          this.addLog("XMODEM-CRC: 发送数据块 #" + blockNumber + " (" + dataBytes + " 字节数据)");
          // 更新状态
          this.setData({
            xmodemState: "sending",
            xmodemBlockNumber: blockNumber + 1,
            filePosition: this.data.filePosition + dataBytes
          });
        },
        fail: (res) => {
          this.addLog("XMODEM-CRC: 发送数据块失败: " + res.errMsg);
          wx.showToast({
            title: "XMODEM传输失败",
            icon: "none"
          });
        }
      });
    } else {
      // 使用校验和模式
      const checksum = this.calculateChecksum(blockData);

      // 构造XMODEM帧: SOH + 块编号 + 块编号取反 + 数据 + 校验和
      const frame = new Uint8Array(132);
      frame[0] = 0x01; // SOH
      frame[1] = blockNumber & 0xFF; // 块编号
      frame[2] = (~blockNumber) & 0xFF; // 块编号取反
      frame.set(blockData, 3); // 数据
      frame[131] = checksum & 0xFF; // 校验和

      // 发送数据块
      const buffer = frame.buffer;
      wx.writeBLECharacteristicValue({
        deviceId: this.data.device.deviceId,
        serviceId: app.globalData.mserviceuuid,
        characteristicId: app.globalData.mtxduuid,
        value: buffer,
        success: (res) => {
          this.addLog("XMODEM: 发送数据块 #" + blockNumber + " (" + dataBytes + " 字节数据)");
          // 更新状态
          this.setData({
            xmodemState: "sending",
            xmodemBlockNumber: blockNumber + 1,
            filePosition: this.data.filePosition + dataBytes
          });
        },
        fail: (res) => {
          this.addLog("XMODEM: 发送数据块失败: " + res.errMsg);
          wx.showToast({
            title: "XMODEM传输失败",
            icon: "none"
          });
        }
      });
    }
  },

  // 重传当前XMODEM数据块
  resendCurrentXmodemBlock() {
    // 重新发送当前块，位置和块编号不变
    this.sendNextXmodemBlock();
  },

  // 发送EOT信号结束传输
  sendEOT() {
    const eotBuffer = new ArrayBuffer(1);
    const eotView = new DataView(eotBuffer);
    eotView.setUint8(0, 0x04); // EOT

    wx.writeBLECharacteristicValue({
      deviceId: this.data.device.deviceId,
      serviceId: app.globalData.mserviceuuid,
      characteristicId: app.globalData.mtxduuid,
      value: eotBuffer,
      success: (res) => {
        this.addLog("XMODEM: 发送EOT信号");
      },
      fail: (res) => {
        this.addLog("XMODEM: 发送EOT失败: " + res.errMsg);
        this.cancelXmodemTransfer();
      }
    });
  },

  // 完成XMODEM传输
  completeXmodemTransfer() {
    this.addLog("XMODEM: 文件传输完成");
    
    const elapsed = Date.now() - this.data.transferStartTime;
    this.setData({
      xmodemState: "complete",
      xmodemBlockNumber: 1,
      filePosition: 0,
      xmodemMode: "checksum",
      progress: 0,
      transferTime: (elapsed / 1000).toFixed(2)
    });

    wx.showToast({
      title: "文件传输完成",
      icon: "success"
    });
  },

  // 取消XMODEM传输
  cancelXmodemTransfer() {
    this.addLog("XMODEM: 文件传输已取消");
    this.setData({
      xmodemState: "idle",
      selectedFile: null,
      fileContent: null,
      xmodemBlockNumber: 1,
      filePosition: 0,
      xmodemMode: "checksum"
    });

    wx.showToast({
      title: "传输已取消",
      icon: "none"
    });
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