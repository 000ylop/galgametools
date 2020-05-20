/*
'
'**************************************
' Visuddhi - http://visuddhi.com
' Copyright (c) 2006 - Visuddhi
' All right reserved. 
'**************************************
'
*/

function correctThumb(){
  var max = maxer();
  if (document.all){
    document.styleSheets[0].addRule('table.dia', 'width:' + (max + 20) + 'px;'); 
    document.styleSheets[0].addRule('table.dia', 'height:' + (max + 20) + 'px;'); 
    document.styleSheets[0].addRule('div.smalldesc', 'width:' + (max + 20) + 'px;'); 
  } else {
    document.styleSheets[0].cssRules[6].style.width = (max + 20) + 'px';
    document.styleSheets[0].cssRules[6].style.height = (max + 20) + 'px';
    document.styleSheets[0].cssRules[7].style.width = (max + 20) + 'px';
  }
}

function correctPic(){
  var max = maxer();
  if (document.all){
    document.styleSheets[0].addRule('table.image', 'width:' + (max + 40) + 'px;'); 
    document.styleSheets[0].addRule('table.image', 'height:' + (max + 40) + 'px;'); 
  } else {
    document.styleSheets[0].cssRules[11].style.width = (max + 40) + 'px';
    document.styleSheets[0].cssRules[11].style.height = (max + 40) + 'px';
  }
}


function maxer(){
  var maxw = 0; var maxh = 0;
  var thumbs = document.getElementsByTagName('img');
  for (var i=0; i<thumbs.length; i++){
    if (thumbs[i].width > maxw){ maxw = thumbs[i].width; }
    if (thumbs[i].height > maxh){ maxh = thumbs[i].height; }
  }
  var max = maxh; if (maxw > maxh){ max = maxw; }
  return max;
}
