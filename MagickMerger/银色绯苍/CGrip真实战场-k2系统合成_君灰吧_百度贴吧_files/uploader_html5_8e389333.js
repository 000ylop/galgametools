_.Module.define({path:"common/component/image_uploader_manager/uploader_html5",requires:["common/component/ImageUploader"],sub:{initial:function(a){var b={container:null,width:200,height:50,isAutoUp:false,queueLen:10,maxParallel:16,maxSize:3*1024*1024,limitSize:10*1024*1024,maxWidth:1000,maxHeight:1000,uploadUrl:"",getUploadParams:null};this._options=$.extend(b,a||{});this._uploadQueue=[];this._fileStatusArr=[];this._currentUploadNum=0;this._uploadTimer=null;this._activeTotal=0;this._typeFilter="image/jpeg, image/jpg, image/png, image/gif, image/bmp";this._reset()},start:function(){this._trigger("onStartUpload");this._startProgress()},stop:function(){this._stopProgress()},deleteFile:function(c){for(var b=0;b<this._uploadQueue.length;b++){var a=this._uploadQueue[b];if(a&&a.id==c){a.uploader.distroy();if(a.isStarted&&a.percent<100){this._activeTotal--}this._uploadQueue.splice(b,1);break}}},clearList:function(){this._uploadQueue.length=0},reUploadError:function(a){this._resetErrorStatus(a);this.start()},_reset:function(){this._initUI();this._bindEvents()},_initUI:function(){this._fileInput=$('<input type="file" id="multi_file" style="display:none" multiple accept="'+this._typeFilter+'" />').get(0)},_bindEvents:function(){var a=this;this._fileInput.onclick=function(){a._fileInput.value=""};this._fileInput.onchange=function(){a._selectFile(a._fileInput.files)};if(this._options.container){this._options.container.bind("click",function(){a._fileInput.value="";a._fileInput.click()})}},_selectFile:function(c){var e=[];for(var b=0,d=c.length;b<d&&b<this._options.queueLen;b++){var a=c[b];if(a.type!=""&&this._typeFilter.indexOf(a.type)>=0){e.push(this._add(a))}}this._trigger("onFileSelected",{imageList:e,percent:0})},_add:function(d){var b=this,a=b._uploadQueue,c={name:d.name,size:d.size,id:"img_"+this._makeToken()+"_"+a.length+"_"+d.type,percent:0,status:"",errorCode:0,errorMessage:null,source:d,isStarted:false},e;a.push(c);if(d.size>this._options.limitSize){c.errorCode=1;c.errorMessage="\u6587\u4ef6\u5927\u5c0f\u8d85\u51fa\u9650\u5236";this._trigger("error",c);c.percent=100;c.isStarted=true;return c}e=b.use("common/component/ImageUploader",this._options.uploadUrl,null,{maxWidth:this._options.maxWidth,maxHeight:this._options.maxHeight});c.uploader=e;e.id=c.id;e.bind("success",this._uploadedHandler,this);e.bind("error",this._uploadedHandler,this);e.bind("progress",function(f){return function(g,h,i){f.percent=Math.min(f.percent+parseInt(20*h/i,10),99)}}(c));return c},_upload:function(){var a=0;var d=true;for(var c=0;c<this._uploadQueue.length;c++){var b=this._uploadQueue[c];if(!b){continue}if(b.percent<100){d=false}if(this._activeTotal<this._options.maxParallel&&!b.isStarted){b.isStarted=true;b.percent=5;if(this.getUploadParams){b.uploader.setVariable(this.getUploadParams(b.id))}b.uploader.upload(b.source,this._options.maxSize);this._activeTotal++}if(b.percent<50){b.percent+=5}a+=b.percent/this._uploadQueue.length}if(d){this._percent=100;this.stop();this._trigger("onComplete")}else{this._percent=parseInt(a,10);this._trigger("onProgressListen")}},_uploadedHandler:function(a,d,c){var e=a.target.id;this._activeTotal--;var b=this._getFileById(e);if(!b){return false}if(a.type=="error"){b.errorCode=2;b.errorMessage="\u7f51\u7edc\u9519\u8bef";this._trigger("error",b);b.percent=100}if(a.type=="success"){b.response=c.data||c||null;b.percent=100}a.target.unbind("success",this._uploadedHandler);a.target.unbind("error",this._uploadedHandler)},_trigger:function(a,b){this.trigger(a,b||{imageList:this._uploadQueue,percent:this._percent})},_getFileById:function(c){for(var b=0;b<this._uploadQueue.length;b++){var a=this._uploadQueue[b];if(a&&a.id==c){return a}}return null},_resetErrorStatus:function(c){for(var b=0;b<this._uploadQueue.length;b++){var a=this._uploadQueue[b];if(a&&a.errorCode!==0){if(!c||c&&a.id==c){a.percent=0;a.errorCode=0;a.errorMessage=null;a.isStarted=false;a.response=null;if(c){break}}}}},_startProgress:function(){if(!this._uploadTimer){var a=this;this._uploadTimer=setInterval(function(){a._upload()},1000)}},_stopProgress:function(){clearInterval(this._uploadTimer);this._uploadTimer=null},_makeToken:function(){var a=function(){return(((1+Math.random())*65536)|0).toString(16).substring(1)};return(a()+a()+"_"+a()+a()+a())}}});