var bdShare=bdShare||{};bdShare.ready=bdShare.ready||function(b,c){c=c||document;if(/complete/.test(c.readyState)){b()}else{if(c.addEventListener){if("interactive"==c.readyState){b()}else{c.addEventListener("DOMContentLoaded",b,false)}}else{var a=function(){a=new Function;b()};void function(){try{c.body.doScroll("left")}catch(d){return setTimeout(arguments.callee,10)}a()}();c.attachEvent("onreadystatechange",function(){("complete"==c.readyState)&&a()})}}};bdShare.loadScript=bdShare.loadScript||function(b){var a=document.createElement("script");a.src=b;a.charset="utf-8";bdShare.ready(function(){document.getElementsByTagName("script")[0].parentNode.appendChild(a)})};bdShare.imgshareStartTime=new Date().getTime();if(!bdShare.ApiPVLogger){bdShare.loadScript("http://static.tieba.baidu.com/tb/static-common/js/pic_share/logger.js")}bdShare.loadScript("http://static.tieba.baidu.com/tb/static-common/js/pic_share/imgshare.js");