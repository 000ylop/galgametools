var bdShare=bdShare||{};(function(){var r=new Date().getTime();var p=new Date().getTime();var n=new Date().getTime();var c=window,w=document,x=w.body,u=w.documentElement,a={},d="",g=w.title,J=[],q="",b={},v=w.getElementById("bdshare_js").attributes.data,m=/MSIE 6.0/.test(navigator.userAgent),I=w.compatMode!="CSS1Compat",v=v.nodeValue.replace(/&amp;/g,"&").split("&")||"";try{var y=c.location||w.location;a.href=y.href;a.search=y.search;a.hash=y.hash;a.protocol=y.protocol;a.hostname=y.hostname;a.pathname=y.pathname;d=encodeURIComponent(a.href)}catch(H){setTimeout(function(){var e=c.location||w.location;a.href=e.href;a.search=e.search;a.hash=e.hash;a.protocol=e.protocol;a.hostname=e.hostname;a.pathname=e.pathname;d=encodeURIComponent(a.href)},0)}for(var F=v.length;F--;){var C=v[F].split("=");J[C[0]]=C[1]}var z={bdPref:"bds_",bdNums:J.mini?7:15,bdMini:J.mini||false,bdType:J.type||"",bdLeft:J.pos||"right",bdTitle:"\u5206\u4eab\u5230",bdText:g,weiboText:"",bdMore:"\u66f4\u591a...",bdPopTitle:"\u5206\u4eab\u5230",bdImg:J.img||0,bdImgW:24,bdTop:0,bdIds:"bdshare",bdIdsl:J.mini?"bdshare_m":"bdshare_l",bdUid:J.uid||"",bdPic:"",searchPic:J.searchPic||0,bdComment:"",bdDesc:"",wbUid:"",tongji:"on",review:"on",render:true,snsKey:{},bdHost:"http://bdimg.share.baidu.com/static/",bdJump:"http://share.baidu.com/",bdApi:"http://api.share.baidu.com/",bdStatistics:"http://share.baidu.com/commit",bdCommit:"http://s.share.baidu.com/",bdMshare:"http://s.share.baidu.com/mshare",bdCout:"http://nsclick.baidu.com/v.gif?pid=307",bdWList:"([http|https]://[a-zA-Z0-9_.]+.baidu.com)",bdList:["mshare,\u4e00\u952e\u5206\u4eab,mshare","qzone,QQ\u7a7a\u95f4,qqkj","tsina,\u65b0\u6d6a\u5fae\u535a,xlwb","baidu,\u767e\u5ea6\u641c\u85cf,bdsc","renren,\u4eba\u4eba\u7f51,rrw","tqq,\u817e\u8baf\u5fae\u535a,txwb","bdxc,\u767e\u5ea6\u76f8\u518c,bdxc","kaixin001,\u5f00\u5fc3\u7f51,kxw","tqf,\u817e\u8baf\u670b\u53cb,txpy","tieba,\u767e\u5ea6\u8d34\u5427,bdtb","douban,\u8c46\u74e3\u7f51,db","tsohu,\u641c\u72d0\u5fae\u535a,shwb","bdhome,\u767e\u5ea6\u65b0\u9996\u9875,bdhome","sqq,QQ\u597D\u53CB,sqq","thx,\u548c\u8baf\u5fae\u535a,thx","qq,QQ\u6536\u85cf,qqsc","taobao,\u6211\u7684\u6dd8\u5b9d,tjh","hi,\u767e\u5ea6\u7a7a\u95f4,bdkj","msn,MSN,msn","sohu,\u641c\u72d0\u767d\u793e\u4f1a,shbsh","t163,\u7f51\u6613\u5fae\u535a,wywb","qy,\u5947\u827a\u5947\u8c08,qyqt","meilishuo,\u7f8e\u4e3d\u8bf4,mls","mogujie,\u8611\u83c7\u8857,mgj","diandian,\u70b9\u70b9\u7f51,diandian","huaban,\u82b1\u74e3,huaban","leho,\u7231\u4e50\u6d3b,leho","share189,\u624B\u673A\u5FEB\u4F20,share189","duitang,\u5806\u7cd6,duitang","hx,\u548c\u8baf,hexun","tfh,\u51e4\u51f0\u5fae\u535a,fhwb","fx,\u98de\u4fe1,feixin","youdao,\u6709\u9053\u4e91\u7b14\u8bb0,youdao","sdo,\u9EA6\u5E93\u8BB0\u4E8B,sdo","qingbiji,\u8F7B\u7B14\u8BB0,qingbiji","ifeng,\u51E4\u51F0\u5FEB\u535A,ifeng","people,\u4EBA\u6C11\u5FAE\u535A,people","xinhua,\u65B0\u534E\u5FAE\u535A,xinhua","ff,\u996d\u5426,fanfou","mail,\u90ae\u4ef6\u5206\u4eab,mail","kanshou,\u641C\u72D0\u968F\u8EAB\u770B,kanshou","yaolan,\u6447\u7BEE\u7A7A\u95F4,yaolan","wealink,\u82e5\u90bb\u7f51,wealink","tuita,\u63A8\u4ED6,tuita","xg,\u9c9c\u679c,xianguo","ty,\u5929\u6daf\u793e\u533a,tianya","fbook,Facebook,facebook","twi,Twitter,twitter","ms,Myspace,myspace","deli,delicious,delicious","s51,51\u6E38\u620F\u793e\u533a,51shequ","s139,139\u8bf4\u5ba2,shuoke","linkedin,linkedin,LinkedIn","copy,\u590d\u5236\u7f51\u5740,kaobei","print,\u6253\u5370,print"]}||{},h={_parseJson:function(E){E=E.replace(/^\s*{|}\s*$/g,"");var D=E.split(",");var K={};function j(R){if(R.indexOf("{")==0){return h._parseJson(R)}else{var Q=R.split(".");var S=window;for(var P=0,O=Q.length;P<O;P++){var T=Q[P];if(typeof S=="object"&&T in S){S=S[T]}else{return R}}return S}}for(var l=0,B=D.length;l<B;l++){var N=D[l];var s=N.indexOf(":");if(s){var M=N.substring(0,s).replace(/^\s*[\'\"]?|[\"\']?\s*$/g,"");var e=N.substring(s+1).replace(/^\s*/g,"");var L=e.charAt(0);if(L=="'"||L=='"'){do{var k=e.replace(/\s*$/g,"");if(k.substring(k.length-1)==L){break}else{e=e+","+D[++l]}}while(l<D.length-1);e=e.replace(/^\s*[\'\"]?|[\"\']?\s*$/g,"")}else{if(L=="{"){do{var k=e.replace(/\s*$/g,"");if(k.substring(k.length-1)=="}"){break}else{e=e+","+D[++l]}}while(l<D.length-1);e=j(e)}else{if(!/^[0-9]*$/.test(e)){e=j(e)}else{e=e.replace(/^\s*[\'\"]?|[\"\']?\s*$/g,"")}}}K[M]=e}}return K},_createScriptTag:function(i,e,j){i.setAttribute("type","text/javascript");j&&i.setAttribute("charset",j);i.setAttribute("src",e);document.getElementsByTagName("head")[0].appendChild(i)},_removeScriptTag:function(i){if(i.clearAttributes){i.clearAttributes()}else{for(var e in i){if(i.hasOwnProperty(e)&&"parentNode"!=e){delete i[e]}}}if(i&&i.parentNode){i.parentNode.removeChild(i)}i=null},callByBrowser:function(D,B,l){var k=document.createElement("SCRIPT"),s=0,L=l||{},j=L.charset,K=B||function(){},E=L.timeOut||0,i;k.onload=k.onreadystatechange=function(){if(s){return}var e=k.readyState;if("undefined"==typeof e||e=="loaded"||e=="complete"){s=1;try{K();clearTimeout(i)}finally{k.onload=k.onreadystatechange=null;h._removeScriptTag(k)}}};if(E){i=setTimeout(function(){k.onload=k.onreadystatechange=null;h._removeScriptTag(k);L.onfailure&&L.onfailure()},E)}h._createScriptTag(k,D,j)},on:function(j,e,i){if(j.addEventListener){j.addEventListener(e,i,false)}else{if(j.attachEvent){j.attachEvent("on"+e,i)}}},unon:function(j,e,i){if(j.removeEventListener){j.removeEventListener(e,i,false)}else{if(j.detachEvent){j.detachEvent("on"+e,i)}}},html:function(i,e){var k=w.createElement(e||"div");for(var j in i){j=="style"?k[j].cssText=i[j]:k[j]=i[j]}if(e=="link"){w.getElementsByTagName("head")[0].appendChild(k)}else{return x.insertBefore(k,(e=="iframe"?x.firstChild:q.nextSibling))}},list:function(j){var k=z.bdPref,e=j.split(","),B=e[1],i="#",l=k+e[0]+" "+e[2];return{name:B,url:i,cls:l}},list_s:function(K){var E=[],l=inner="",B=z,e=B.bdList,j=B.bdNums,D=(B.bdNums<e.length)?'<li><a href="#" class="bds_more">'+B.bdMore+"</a></li>":"";while(j--){l=h.list(e[j]),E.push('<a href="'+l.url+'" class="'+l.cls+'">'+l.name+"</a>")}inner='<iframe id="bdsIfr" style="position:absolute;display:none;z-index:9999;" frameborder="0"></iframe>';inner+='<div id="'+B.bdIdsl+'"><div id="'+B.bdIdsl+'_c"><h6>'+B.bdTitle+"</h6>";inner+="<ul><li>"+E.reverse().join("</li><li>")+"</li>"+D+'</ul><p><a href="#" class="goWebsite">\u767e\u5ea6\u5206\u4eab</a></p></div></div>';if(K){var k=h.html({id:"bdshare_s",innerHTML:inner,style:"display:none"});k.style.display="block";var i=h.children(k);k=i[i.length-1];h.on(k,"mouseover",function(P){var P=P||window.event;var O=P.target||P.srcElement;var N=h.getPosition(O);var M=P.relatedTarget||P.fromElement;var L=h.getMousePos(P);var s=L.x-N.l;var Q=L.y-N.t;O.buttontype=1;O.x=s;O.y=Q;if(h.contains(k,M)){return}k.time=+new Date})}else{return inner}},scroll:function(e,i){h.on(c,"scroll",function(s){var j=h.getSize(),k=z,l=k.bdTop;if(k.bdType=="slide"){if(m||I){h.setAttr(w.getElementById(k.bdIdsl),"display:block");_bdS.style.top=(j.t+(l==0?(j.h/2-(w.getElementById(k.bdIdsl).offsetHeight-11)/2):parseInt(l)))+"px"}else{if(k.bdTop!=0){_bdS.style.top=parseInt(k.bdTop)+"px"}}}if(i=="pop"&&(m||I)){e.style.top=(j.t+j.h/2-e.offsetHeight/2)+"px";h.setAttr(q,"top:"+e.style.top)}})},getSize:function(){return{t:(x.scrollTop||u.scrollTop),l:(x.scrollLeft||u.scrollLeft),w:(u.clientWidth||x.clientWidth),h:(c.innerHeight||u.clientHeight||x.clientHeight)}},getPosition:function(Q){var T=h.getSize(),j=Q,O={t:0,l:0},i=/gecko/.test(navigator.userAgent),N=function(s,e){O.t+=s,O.l+=e};if(j&&j!=x){if(j.getBoundingClientRect){var P=j.getBoundingClientRect(),R=Q.ownerDocument,D=R.body,l=R.documentElement,k=l.clientTop||D.clientTop||0,L=l.clientLeft||D.clientLeft||0;if(P.top==P.bottom){var B=j.style.display;j.style.display="block";j.style.display=B}N(P.top+T.t-k,P.left+T.l-L)}else{var M=w.defaultView;while(j){N(j.offsetTop,j.offsetLeft);var K=M.getComputedStyle(j,null);if(i){var E=parseInt(K.getPropertyValue("border-left-width"),10)||0,S=parseInt(K.getPropertyValue("border-top-width"),10)||0;N(S,E);if(j!=Q&&K.getPropertyValue("overflow")!="visible"){N(S,E)}}j=j.offsetParent}j=Q.parentNode;while(j&&j!=x){N(-j.scrollTop,-j.scrollLeft);j=j.parentNode}}}return O},setAttr:function(j,i){var k=i.split(";"),e=k.length;while(e--){if(k[e]){var l=k[e].split(":");if(l){j.style[l[0]]=l[1]}}}},children:function(j){for(var e=[],i=j.firstChild;i;i=i.nextSibling){if(i.nodeType==1){e.push(i)}}return e},generateRandom:function(B,D){var e=B.length;var k="";for(var j=1;j<=D;j++){var l=Math.floor(e*Math.random());k=k+B.charAt(l)}return k},generateLinkid:function(){var i=(+new Date).toString(36);var e=h.generateRandom("0123456789abcdefghijklmnopqrstuvwxyz",3);return i+e},contains:function(i,j){if(!j||!i){return false}if(i==j){return true}var e=j.parentNode;while(e!=null&&typeof(e.tagName)!="undefind"){if(e==i){return true}e=e.parentNode}return false},getMousePos:function(e){if(e.pageX||e.pageY){return{x:e.pageX,y:e.pageY}}if(document.documentElement&&document.documentElement.scrollTop){return{x:e.clientX+document.documentElement.scrollLeft-document.documentElement.clientLeft,y:e.clientY+document.documentElement.scrollTop-document.documentElement.clientTop}}else{if(document.body){return{x:e.clientX+document.body.scrollLeft-document.body.clientLeft,y:e.clientY+document.body.scrollTop-document.body.clientTop}}}},_isCookieValidKey:function(e){return(new RegExp('^[^\\x00-\\x20\\x7f\\(\\)<>@,;:\\\\\\"\\[\\]\\?=\\{\\}\\/\\u0080-\\uffff]+\x24')).test(e)},getCookieRaw:function(i){if(h._isCookieValidKey(i)){var j=new RegExp("(^| )"+i+"=([^;]*)(;|\x24)"),e=j.exec(document.cookie);if(e){return e[2]||null}}return null},setCookieRaw:function(j,k,i){if(!h._isCookieValidKey(j)){return}i=i||{};var e=i.expires;if("number"==typeof i.expires){e=new Date();e.setTime(e.getTime()+i.expires)}document.cookie=j+"="+k+(i.path?"; path="+i.path:"")+(e?"; expires="+e.toGMTString():"")+(i.domain?"; domain="+i.domain:"")+(i.secure?"; secure":"")},getCookie:function(e){var i=h.getCookieRaw(e);if("string"==typeof i){i=decodeURIComponent(i);return i}return null},setCookie:function(i,j,e){h.setCookieRaw(i,encodeURIComponent(j),e)}},o=function(e){h.html({href:z.bdHost+"css/bdsstyle.css?cdnversion=20121228",rel:"stylesheet",type:"text/css"},"link");q=q||h.html({style:"display:none;"+(e||""),frameBorder:0},"iframe")},t=function(K,O,N,e){var M=/(#\d+\-[a-zA-Z\d]+\-\d+\-\d+\-[a-f\d]{32}$)/g,s=(a.search+a.hash).replace(/&/g,"#"),j=s.match(M)||"",P=new RegExp(z.bdWList,"ig"),E=a.protocol+"//"+a.hostname+a.pathname+a.search;if(P.test(a.href)&&j==""&&typeof K=="undefined"){return}var L=[];if(bdShare.fn.b.length>0){L.push("button")}if(bdShare.fn.t.length>0){L.push("tools")}if(bdShare.fn.conf.bdType){L.push(bdShare.fn.conf.bdType)}var D=(function(){for(var T=0,R=L.length,U={},S=[];T<R;T++){!(L[T] in U)&&S.push(L[T])&&(U[L[T]]="")}return S.join(",")})();if(z.tongji=="on"||j!=""||typeof K!="undefined"){if(j!=""&&K!="3072"){j=j.toString().replace("#","");(new Image()).src=z.bdCommit+"back?url="+encodeURIComponent(E)+"&sign="+encodeURIComponent(j)+"&title="+encodeURIComponent(g.substr(0,300))}var k=bdShare.loadTime?(+new Date-bdShare.loadTime):"";if(typeof K!="undefined"){(new Image()).src=z.bdCout+"&type="+K+"&sign="+j+"&uid="+z.bdUid+"&desturl="+encodeURIComponent(w.referrer)+"&linkid="+f+"&button="+D+"&loadTime="+k}else{var Q=[0,0,0,0];for(var l=0,B=L.length;l<B;l++){Q[{tools:0,button:1,slide:2}[L[l]]]=1}bdShare._LogPool=bdShare._LogPool||[];bdShare._LogPool.push({key:"cite",api:{uid:z.bdUid||"",linkid:f,type:parseInt(Q.reverse().join(""),2),mini:z.bdMini?1:0,loadTime:k}})}}},G={create:function(){var E=z,L=E.bdList,i=L.length,k=[],j="",M=h.getSize(),e=w.getElementById(E.bdIds+"_pop"),D=((m||I)?"absolute":"fixed"),K=((m||I)?M.t:0);if(!e){while(i--){j=h.list(L[i]),k.push('<a href="'+j.url+'" class="'+j.cls+'">'+j.name+"</a>")}var B=h.html({id:E.bdIds+"_pop",style:"position:"+D,innerHTML:"<div><h5></h5><ul><li>"+k.reverse().join("</li><li>")+'</li></ul><p><a href="#" class="goWebsite">\u767e\u5ea6\u5206\u4eab</a></p></div>'});A.bind(B,"mouseover","a");A.bind(B,"click","b",function(){B.style.display="none";q.style.display="none"});h.on(B,"mouseover",function(R){var R=R||window.event;var Q=R.target||R.srcElement;var P=h.getPosition(Q);var N=h.getMousePos(R);var s=N.x-P.l;var S=N.y-P.t;Q.buttontype=2;Q.x=s;Q.y=S;var O=R.relatedTarget||R.fromElement;if(h.contains(B,O)){return}B.time=+new Date})}var l=B||e;l.getElementsByTagName("h5")[0].innerHTML=(b.title||E.bdPopTitle)+"<b>&nbsp;</b>";h.setAttr(l,"display:block;");h.setAttr(l,"left:"+(M.w/2-l.offsetWidth/2)+"px;top:"+(K+M.h/2-l.offsetHeight/2)+"px");h.setAttr(q,"position:"+D+";display:block;z-index:10000;width:"+l.offsetWidth+"px;height:"+l.offsetHeight+"px;left:"+l.style.left+";top:"+l.style.top);h.scroll(l,"pop");h.on(c,"resize",function(O){var s=h.getSize(),N="left:"+(s.w/2-l.offsetWidth/2)+"px;top:"+(K+s.h/2-l.offsetHeight/2)+"px";h.setAttr(l,N);h.setAttr(q,N)});if(E.bdType!="slide"){w.getElementById(z.bdIdsl).style.display="none";w.getElementById("bdsIfr").style.display="none"}window.bdShareActivity&&bdShareActivity.check()}},A={bind:function(i,e,l,k){var j=this;i.addEventListener?i.addEventListener(e,function(s){return j._action(s,s.target,l,i,k)},false):i.attachEvent("on"+e,function(s){s=c.event;return j._action(s,s.srcElement,l,i,k)})},_action:function(B,i,D,s,l){if(i.nodeName==D.toUpperCase()){if(D=="a"||D=="span"){var E=i.className,j=E=="goWebsite"?E:E.split(" ")[0].split("_")[1],k=this;if(j){(j=="more"&&D=="span")?k._tools():i.onclick=function(K,e){return function(P){var P=P||window.event;var O=P.target||P.srcElement;var N=h.getPosition(O);var M=h.getMousePos(P);var L=M.x-N.l;var Q=M.y-N.t;if(K=="more"){k._tools()}else{if(K=="copy"){k._copy(k)}else{if(K=="print"){k._print()}else{k._jump(K,e,i,L,Q)}}}return false}}(j,s)}return false}if(typeof l==="function"){l()}}if(typeof B.preventDefault==="function"){B.preventDefault();B.stopPropagation()}else{B.returnValue=false;B.cancelBubble=true}},_tools:function(){t(3072);G.create()},_copy:function(i){_this=i;var e=new _this._browsers();if(e.name=="IE"){window.clipboardData.setData("text",document.title+" "+a.href);alert("\u6807\u9898\u548c\u94fe\u63a5\u590d\u5236\u6210\u529f\uff0c\u60a8\u53ef\u4ee5\u63a8\u8350\u7ed9QQ/MSN\u4e0a\u7684\u597d\u53cb\u4e86\uff01")}else{window.prompt("\u60a8\u4f7f\u7528\u7684\u662f\u975eIE\u6838\u5fc3\u6d4f\u89c8\u5668\uff0c\u8bf7\u6309\u4e0b Ctrl+C \u590d\u5236\u4ee3\u7801\u5230\u526a\u8d34\u677f",document.title+" "+a.href)}},_mail:function(e){},_print:function(){var e=document.getElementById("bdshare_pop");e&&(e.style.display="none");window.print();e&&(e.style.display="block");return},_jump:function(am,al,e,V,U){var ak=z,T=ak.bdPic,B=ak.bdText,R=ak.weiboText,ac=ak.wbUid,D=d,ab=ak.bdDesc,Q=ak.bdComment,O=ak.bdMiniWindow||0;b=(al.getAttribute("data")&&al.getAttribute("data")!="")?h._parseJson(al.getAttribute("data")):b;if(b!=""){T=encodeURIComponent(b.pic||"")||T;D=encodeURIComponent(b.url||"")||D;Q=encodeURIComponent(b.comment||"")||Q;ab=encodeURIComponent(b.desc||"")||ab;ac=b.wbuid||ac;if(am=="tsina"){if(b.text&&b.weibotext){B=encodeURIComponent((b.text+b.weibotext).substr(0,300))}else{if(b.weibotext){B=encodeURIComponent((B+b.weibotext).substr(0,300))}else{if(b.text){B=encodeURIComponent((b.text+R).substr(0,300))}else{B=encodeURIComponent((B+R).substr(0,300))}}}}else{B=encodeURIComponent((b.text||"").substr(0,300))||encodeURIComponent(B.substr(0,300))}}else{if(am=="tsina"){B=encodeURIComponent((B+R).substr(0,300))}else{B=encodeURIComponent(B.substr(0,300))}}B=encodeURIComponent(B);n=new Date().getTime()+3000;var ae=r.toString(32)+p.toString(32)+n.toString(32);var ao=e.buttontype;var k=Math.floor(bdShare.XY.loadedX);var j=Math.floor(bdShare.XY.loadedY);var N=ao>0?1:0;var Y=Math.floor(e.x);var W=Math.floor(e.y);var K=Math.floor(V);var E=Math.floor(U);var X=Math.floor(e.offsetWidth);var ah=Math.floor(e.offsetHeight);var M=ao;var Z=+new Date;Z=Z-al.time;var L=[k,j,N,Y,W,K,E,X,ah,M,Z].join(".");var i;switch(z.bdType){case"button":i=1;break;case"slide":i=2;break;default:i=0;break}var an=am=="mail"?"_mail":am;var P="?click=1&url="+D+"&uid="+ak.bdUid+"&to="+an+"&type=text&relateUid="+ac+"&pic="+T+"&title="+B+"&key="+(ak.snsKey[am]||"")+"&sign="+ak.review+"&desc="+ab+"&comment="+Q+"&searchPic="+ak.searchPic+"&l="+ae+"&linkid="+f+"&sloc="+L+"&apiType=0&buttonType="+i+"&firstime="+h.getCookie("bdshare_firstime");if(am=="mshare"){var aj=ak.bdMshare+P}else{var aj=ak.bdCommit+P}var S=ak.bdCommit+"commit"+P+"&t="+Math.random();if(am!="goWebsite"){bdShare.fn._getSc("share");var ag=function(){var l=[];return function(ap){var s=l.push(new Image)-1;l[s].onload=function(){l[s]=l[s].onload=null};l[s].src=ap}}();setTimeout(function(){ag(S)},1500);if(am=="bdxc"){var ai=window,aj=document,ad="_bdXC",aa;if(ai[ad]){if(window._bdXC_loaded){ai[ad].reInit()}}else{aa=aj.createElement("script");aa.setAttribute("charset","utf-8");aa.src="http://xiangce.baidu.com/zt/collect/mark.js?"+(new Date()).getTime();aj.getElementsByTagName("head")[0].appendChild(aa)}}else{if(am=="mail"){var af=z.bdCommit+"sendmail";window.open(af+P,"_blank")}else{if(1==O){c.open(aj,"bdShare","height=400, width=600")}else{c.open(aj)}}}}else{c.open(ak.bdJump)}},_browsers:function(){var i={};var e=navigator.userAgent.toLowerCase();var k,j;if(e.match(/msie ([\d.]+)/)){j=e.match(/msie ([\d.]+)/);i.name="IE";i.version=j[1]}else{if(e.match(/firefox\/([\d.]+)/)){j=e.match(/firefox\/([\d.]+)/);i.name="firefox";i.version=j[1]}else{if(e.match(/chrome\/([\d.]+)/)){j=e.match(/chrome\/([\d.]+)/);i.name="chrome";i.version=j[1]}else{if(e.match(/opera.([\d.]+)/)){j=e.match(/opera.([\d.]+)/);i.name="opera";i.version=j[1]}else{if(e.match(/safari.([\d.]+)/)){j=e.match(/safari.([\d.]+)/);i.name="safari";i.version=j[1]}else{i.name="\u672a\u77e5\u7684\u6d4f\u89c8\u5668";i.version="\u672a\u77e5\u7684\u7248\u672c\u53f7"}}}}}return i}};var f=h.generateLinkid();bdShare.fn=(function(){return{init:function(){p=new Date().getTime()+1000;this.b=[];this.t=[];this.s=[];this.type="load";this._getShare();var l=this;if(typeof bds_config!="undefined"){for(var K in bds_config){if(K=="bdPopTitle"||K=="bdText"||K=="weiboText"||K=="bdPic"||K=="wbUid"||K=="tongji"||K=="render"||K=="snsKey"||K=="review"||K=="bdComment"||K=="bdDesc"||K=="bdTop"||K=="searchPic"||K=="bdUrl"||K=="bdMiniWindow"){z[K]=(K=="render"||K=="snsKey"||K=="bdPopTitle"||K=="bdText"||K=="weiboText")?bds_config[K]:encodeURIComponent(bds_config[K])}}}this.conf=z;this.imgW=z.bdImgW;if(this.conf.render){if(this.b.length>0||this.t.length>0){this._share()}if(this.conf.bdType=="slide"){this._slide()}}this._getSc(this.type);var D=this.t;for(var B=0;B<D.length;B++){var E=D[B].getElementsByTagName("a");for(var s=0;s<E.length;s++){!(E[s].getAttribute("href"))&&E[s].setAttribute("href","#")}}bdShare.XY=bdShare.XY||{};function k(j){var j=j||window.event;var i=h.getMousePos(j);bdShare.XY.loadedX=i.x;bdShare.XY.loadedY=i.y;if(bdShare.XY.loadedX>0&&bdShare.XY.loadedY>0){h.unon(document,"mousemove",k)}}h.on(document,"mousemove",k);for(var B=0,e=this.t.length;B<e;B++){(function(i){var j=i;h.on(l.t[i],"mouseover",function(Q){var Q=Q||window.event;var P=Q.target||Q.srcElement;var O=h.getPosition(P);var M=h.getMousePos(Q);var L=M.x-O.l;var R=M.y-O.t;P.buttontype=0;P.x=L;P.y=R;var N=Q.relatedTarget||Q.fromElement;if(h.contains(l.t[j],N)){return}l.t[j].time=+new Date})})(B)}t();if(h.getCookie("bdshare_firstime")==null){h.setCookie("bdshare_firstime",new Date()*1,{path:"/",expires:new Date().setFullYear(2022)-new Date()})}},_getSc:function(i){this.type=i;var j=w.querySelectorAll?w.querySelectorAll(".shareCount"):w.getElementsByTagName("*"),e=j.length;while(e--){if(/\sshareCount|shareCount\s|shareCount$/.test(j[e].className)){this.s.push(j[e])}}if(this.s.length>0){h.callByBrowser(this.conf.bdApi+"getnum?url="+d+"&callback=bdShare.fn._getShare&type="+i+"&t="+new Date().getTime())}},_getShare:function(B){var k=B?true:false,L=[];if(w.querySelectorAll){this.b=w.querySelectorAll(".bdshare_b");this.t=w.querySelectorAll(".bdshare_t")}else{var K=w.getElementsByTagName("*"),j=K.length;while(j--){var D=K[j];if(/\sbdshare_t|bdshare_t\s/.test(D.className)){this.t.push(D)}if(/\sbdshare_b|bdshare_b\s|bdshare_b$/.test(D.className)){this.b.push(D)}}}if(k){var L=this.s,i=L.length,E;while(i--){if(/\sshareCount|shareCount\s|shareCount$/.test(L[i].className)){E=this.type=="share"?(B.num[0]<10000?parseInt(L[i].innerHTML)+1:B.num[1]):(B.errno*1==0?B.num[1]:0);L[i].innerHTML=decodeURI(E);L[i].setAttribute("title","\u7d2f\u8ba1\u5206\u4eab"+B.num[0]+"\u6b21");L[i].onclick=function(){G.create()}}}this.s=[]}},_share:function(){h.list_s(true);var K=this.b,P=this.t,i=K.length,Q=P.length,e=w.getElementById(this.conf.bdIdsl),j=w.getElementById("bdsIfr"),M=this.conf.bdList,N=M.length;h.setAttr(e,"display:none;");A.bind(e,"mouseover","a");if(i>0){while(i--){if(K[i].id=="bdshare"){A.bind(K[i],"click","img",function(){G.create()});A.bind(K[i],"mouseover","a",function(){G.create()});this._s(K[i],e,j,K[i],"b")}}}if(Q>0){while(Q--){if(P[Q].id=="bdshare"){A.bind(P[Q],"click","span",function(){G.create()});A.bind(P[Q],"mouseover","a");var L=h.children(P[Q]),B=L.length,R="";while(B--){var E=L[B];if(/bds_more/.test(E.className)){R=L[B];if(I){R.style.height=(/bds_tools_32/.test(P[Q].className)?38:17)+"px"}this._s((R==""?P[Q]:R),e,j,P[Q],R)}else{for(var k=0;k<N;k++){if(M[k]){var D=M[k].split(","),O=new RegExp(this.conf.bdPref+D[0]+"$","ig");if(M[k]){var D=M[k].split(","),O=new RegExp(this.conf.bdPref+D[0]+"$","ig");if(O.test(E.className)){if(D[1]=="拷贝"){E.title="复制网址"}else{if(!E.title){E.title="\u5206\u4eab\u5230"+D[1]}}}}}}}}}}}},_s:function(B,e,i,l,K){var j=this,k=[],E,D=(K==""?"none":"block");this._popShow(B,{over:function(s){k=h.getPosition(B);b=(l.getAttribute("data")&&l.getAttribute("data")!="")?h._parseJson(l.getAttribute("data")):"";h.setAttr(e,"display:"+D);h.setAttr(e,"left:"+(((h.getSize().w-s.clientX)<e.offsetWidth)?(k.l-e.offsetWidth+B.offsetWidth):k.l)+"px");h.setAttr(e,"top:"+(((h.getSize().h-s.clientY)<e.offsetHeight)?(k.t-e.offsetHeight):k.t+B.offsetHeight)+"px");h.setAttr(i,"display:"+D+";width:"+e.offsetWidth+"px;height:"+e.offsetHeight+"px;left:"+e.style.left+";top:"+e.style.top)},out:function(s){E=setTimeout(function(){e.style.display="none";i.style.display="none"},100);j._popShow(e,{over:function(){clearTimeout(E);e.style.display="block";i.style.display="block"},out:function(){e.style.display="none";i.style.display="none"}})}})},_slide:function(){if(!c._bdS){var K=this.conf.bdLeft=="right"?"left":"right",B=this.conf.bdHost+"images/"+this.conf.bdLeft.substring(0,1)+this.conf.bdImg+".gif";c._bdS=h.html({id:this.conf.bdIds,innerHTML:'<img src="'+B+'" alt="" style="float:'+K+';margin-top:58px;"/>'+h.list_s(false)});var i=h.children(_bdS);var e=i[i.length-1];h.on(e,"mouseover",function(S){var S=S||window.event;var R=S.target||S.srcElement;var Q=h.getPosition(R);var P=S.relatedTarget||S.fromElement;var O=h.getMousePos(S);var N=O.x-Q.l;var T=O.y-Q.t;R.buttontype=1;R.x=N;R.y=T;if(h.contains(e,P)){return}e.time=+new Date})}var E=this,j=_bdS,M=h.getSize(),L=w.getElementById(this.conf.bdIdsl),k=w.getElementById("bdsIfr"),s=(!m&&!I)?"fixed":"absolute",D=(m?M.t:0)+(this.conf.bdTop==0?(M.h<L.offsetHeight?-40:(M.h/2-L.offsetHeight/2)):parseInt(this.conf.bdTop));h.setAttr(j,"overflow:hidden;height:330px;position:"+s+";top:"+D+"px;"+this.conf.bdLeft+":0;"+((K=="right")?K+":"+(M.w-this.imgW)+"px":""));h.setAttr(L,K+":"+this.imgW+"px;display:none");A.bind(j,"click","img",function(){t(3072);G.create()});A.bind(j,"mouseover","img",l);A.bind(L,"mouseover","a");function l(){h.setAttr(L,"display:block;");if((L.offsetWidth+E.imgW)==j.offsetWidth){return}tm=setTimeout(function(){if((L.offsetWidth+E.imgW)!=j.offsetWidth){E._move(j,0,E.imgW,L.offsetWidth,k)}},100);E._popShow(L,{over:function(){h.setAttr(k,"height:"+L.offsetHeight+"px;"+E.conf.bdLeft+":0;top:0;display:block")},out:function(){}});E._popShow(j,{over:function(){},out:function(){if(j.offsetWidth==(L.offsetWidth+E.imgW)){clearTimeout(tm);E._move(j,0,L.offsetWidth+E.imgW,-L.offsetWidth,k)}k.style.display="none"}})}h.scroll(j,"slide");h.on(c,"resize",function(){h.setAttr(j,((K=="right")?K+":"+(h.getSize().w-E.imgW)+"px":""))})},_move:function(e,E,B,k,i){var D=e.style,s=i.style,l=this,j=6;setInterval(function(){if(E<j){E++;D.width=Math.ceil(k*E/j+B)+"px";s.width=Math.ceil(k*E/j+B)-l.imgW+"px"}else{return}},10)},_popShow:function(k,e){if(w.all){k.onmouseenter=function(l){j(c.event)};k.onmouseleave=i}else{k.onmouseover=function(l){l.relatedTarget==null?j(l):(!(this===l.relatedTarget||this.compareDocumentPosition(l.relatedTarget)==20)&&j(l))};k.onmouseout=function(l){l.relatedTarget==null?i(l):(!(this===l.relatedTarget||this.compareDocumentPosition(l.relatedTarget)==20)&&i(l))}}function j(l){e.over(l)}function i(l){e.out(l)}}}})();o();bdShare.fn.init()})();