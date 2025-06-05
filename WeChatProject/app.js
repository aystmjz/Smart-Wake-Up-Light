// app.js
App({
  onLaunch() {
    // 展示本地存储能力
    const logs = wx.getStorageSync('logs') || []
    logs.unshift(Date.now())
    wx.setStorageSync('logs', logs)
    // 登录
    wx.login({
      success: res => {
        // 发送 res.code 到后台换取 openId, sessionKey, unionId
      }
    })
  },

  readUUID: function () {
    this.globalData.usrserviceuuid = wx.getStorageSync('usrserviceuuid') || "0000FFE0-0000-1000-8000-00805F9B34FB"
    this.globalData.usrrxduuid = wx.getStorageSync('usrrxduuid') || "0000FFE1-0000-1000-8000-00805F9B34FB"
    this.globalData.usrtxduuid = wx.getStorageSync('usrtxduuid') || "0000FFE1-0000-1000-8000-00805F9B34FB"
    this.globalData.muuidSel = wx.getStorageSync('lastsel') || 0
    console.log(this.globalData.usrrxduuid)
    console.log("Read = ", this.globalData.usrserviceuuid, this.globalData.usrtxduuid, this.globalData.usrrxduuid)
  },
  readSetting: function () {
    this.readUUID()
    this.globalData.mautoSendInv = wx.getStorageSync('autoSendInv') || 100
    this.globalData.msendText = wx.getStorageSync('sendText') || "1024"
    this.globalData.muuidSel = wx.getStorageSync('lastsel') || 0
    switch (this.globalData.muuidSel) {
      case 0:
        this.globalData.mserviceuuid = "0000FFE0-0000-1000-8000-00805F9B34FB"
        this.globalData.mtxduuid = "0000FFE1-0000-1000-8000-00805F9B34FB"
        this.globalData.mrxduuid = "0000FFE1-0000-1000-8000-00805F9B34FB"
        break;
      case 1:
        this.globalData.mserviceuuid = "0000FFE0-0000-1000-8000-00805F9B34FB"
        this.globalData.mtxduuid = "0000FFE2-0000-1000-8000-00805F9B34FB"
        this.globalData.mrxduuid = "0000FFE1-0000-1000-8000-00805F9B34FB"
        break;
      case 2:
        this.globalData.mserviceuuid = this.globalData.usrserviceuuid
        this.globalData.mtxduuid = this.globalData.usrtxduuid
        this.globalData.mrxduuid = this.globalData.usrrxduuid
        break;
    }
    console.log("readSetting ", this.globalData.mautoSendInv, this.globalData.msendText)
  },
  saveSetting(time, text) {
    wx.setStorageSync('autoSendInv', time)
    wx.setStorageSync('sendText', text)
    console.log("WriteSetting ", time, text)
  },
  savelastsel(sel) {
    wx.setStorageSync('lastsel', sel)
    console.log("Writelastsel ", sel)
  },
  saveusrUUID: function (id_s, id_t, id_r) {
    this.globalData.usrserviceuuid = id_s
    this.globalData.usrrxduuid = id_r
    this.globalData.usrtxduuid = id_t
    wx.setStorageSync('usrserviceuuid', id_s)
    wx.setStorageSync('usrrxduuid', id_r)
    wx.setStorageSync('usrtxduuid', id_t)
    console.log("Save usr = ", id_s, id_t, id_r)
  },
  globalData: {
    userInfo: null,
    mserviceuuid: "0000FFE0-0000-1000-8000-00805F9B34FB",
    mtxduuid: "0000FFE1-0000-1000-8000-00805F9B34FB",
    mrxduuid: "0000FFE1-0000-1000-8000-00805F9B34FB",

    usrserviceuuid: "0000FFE0-0000-1000-8000-00805F9B34FB",
    usrtxduuid: "0000FFE1-0000-1000-8000-00805F9B34FB",
    usrrxduuid: "0000FFE1-0000-1000-8000-00805F9B34FB",
    muuidSel: 0,
    mautoSendInv: 10,
    msendText: "",
    ble_device: null,
    refresh: null,
  }
})