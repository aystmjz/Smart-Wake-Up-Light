var app = getApp();
var util = require('../../../utils/util.js');
Page({
  /**
   * 页面的初始数据
   */
  data: {
    items: [
      { name: '0', value: '常规模组', checked: 'true'},
      { name: '1', value: 'BT16模组'},
      { name: '2', value: '指定UUID'},
    ],
      serviceuuid:"",
      txduuid:"",
      rxduuid:"",
      inputen:false,
      nowsel:0,
    mtext:"",
  },
  /**
   * 生命周期函数--监听页面加载
   */
  onLoad: function (options) {
    app.readUUID()
    this.nowsel = app.globalData.muuidSel
    switch (this.nowsel) {
      case 0: this.setData({
        serviceuuid: "0000FFE0-0000-1000-8000-00805F9B34FB",
        txduuid: "0000FFE1-0000-1000-8000-00805F9B34FB",
        rxduuid: "0000FFE1-0000-1000-8000-00805F9B34FB",
        inputen: true,
        'items[0].checked':true,
        'items[1].checked': false,
        'items[2].checked': false,
      })
        break;
      case 1: this.setData({
        serviceuuid: "0000FFE0-0000-1000-8000-00805F9B34FB",
        txduuid: "0000FFE2-0000-1000-8000-00805F9B34FB",
        rxduuid: "0000FFE1-0000-1000-8000-00805F9B34FB",
        inputen: true,
        'items[0].checked': false,
        'items[1].checked':true,
        'items[2].checked': false,
      })
        break;
      case 2: this.setData({
        serviceuuid: app.globalData.usrserviceuuid,
        txduuid: app.globalData.usrtxduuid,
        rxduuid: app.globalData.usrrxduuid,
        inputen: false,
        'items[0].checked': false,
        'items[1].checked': false,
        'items[2].checked': true,
      })
        break;
    }
    console.log(this.data.serviceuuid)
  },
  radioChange(e) {
    console.log('radio发生change事件，携带value值为：', e.detail.value)
    let index = e.currentTarget.dataset.id;
    switch (e.detail.value){
      case "0": this.setData({
        serviceuuid: "0000FFE0-0000-1000-8000-00805F9B34FB",
        txduuid: "0000FFE1-0000-1000-8000-00805F9B34FB",
        rxduuid: "0000FFE1-0000-1000-8000-00805F9B34FB",
        inputen: true
      })
        this.nowsel=0
        app.globalData.muuidSel = 0
      break;
      case "1": this.setData({
        serviceuuid: "0000FFE0-0000-1000-8000-00805F9B34FB",
        txduuid: "0000FFE2-0000-1000-8000-00805F9B34FB",
        rxduuid: "0000FFE1-0000-1000-8000-00805F9B34FB",
        inputen: true
      })
        this.nowsel = 1
        app.globalData.muuidSel =1
        break;
      case "2": this.setData({
        serviceuuid: app.globalData.usrserviceuuid,
        txduuid: app.globalData.usrtxduuid,
        rxduuid: app.globalData.usrrxduuid,
        inputen: false
      })
        this.nowsel = 2
        app.globalData.muuidSel = 2
        break;
        
    }
    console.log(this.data.serviceuuid)
    console.log(app.globalData.usrrxduuid)
    console.log('radio发生change事件，index ：', this.nowsel)
  },
    backok(){
      console.log(this.data.serviceuuid)
      if (!util.isUUID(this.data.serviceuuid)){
      this.setData({
        mtext: "错误的ServiceUUID 格式",
      })
      return
    }
      if (!util.isUUID(this.data.txduuid)) {
        this.setData({
          mtext: "错误的Notify UUID 格式",
        })
        return
      }
      if (!util.isUUID(this.data.rxduuid)) {
        this.setData({
          mtext: "错误的Write UUID 格式",
        })
        return
      }
      app.globalData.mserviceuuid = this.data.serviceuuid
      app.globalData.mtxduuid = this.data.txduuid
      app.globalData.mrxduuid = this.data.rxduuid
    if(this.nowsel==2){ //保存用户的设置
      app.saveusrUUID(this.data.serviceuuid, this.data.txduuid, this.data.rxduuid)
    }
      app.savelastsel(this.nowsel)
    //wx.navigateTo({
    //  url: '/pages/index/index',
    //})
      wx.navigateBack({
        delta: 1
      })
  },
  bindReplaceInput: function (e) {
    var value = e.detail.value
    var pos = e.detail.cursor
    var tet = util.strToUUID(value)
    // 直接返回对象，可以对输入进行过滤处理，同时可以控制光标的位置
    return {
      value: tet,
    }
  },
  inputend1: function (e){
    this.setData({
      serviceuuid: e.detail.value,
    })
  },
  inputend2: function (e) {
    this.setData({
      rxduuid: e.detail.value,
    })
  },
  inputend3: function (e) {
    this.setData({
      txduuid: e.detail.value,
    })
  }


})