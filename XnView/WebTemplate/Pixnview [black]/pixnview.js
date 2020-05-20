function onloadThumb(){
  showForm();
  tweakTitle();
  ifNoHeader();
  imageCounter();
}

function onloadPage() {
  tweakTitle();
  ifNoHeader();
}


function showForm() {
  document.getElementById("findmoreform").style.display="inline";
}

function imageCounter() {
  var otherImages = 0 //number of images in the page that are not part of the album (logo, banner etc...)
  var images = document.getElementsByTagName("img");
  var header = document.getElementsByTagName("h2")[0];
  var albumImages = images.length - otherImages;
  if (albumImages < 2)
    header.innerHTML = header.innerHTML + " (" + albumImages + " image)";
  else
    header.innerHTML = header.innerHTML + " (" + albumImages + " images)";
}

function ifNoHeader() {
  var header = document.getElementsByTagName("h2")[0];
  if (header.innerHTML == "Album: ")
    header.innerHTML = header.innerHTML + "Untitled";
}

function tweakTitle() {
  var title = document.getElementsByTagName("h1")[0];
  var header = document.getElementsByTagName("h2")[0];
  var xnViewHeader = header.innerHTML.substring(7, header.innerHTML.length);
  if (xnViewHeader) {
    if (title.innerHTML.match(/^ - /)) {
      title.innerHTML = title.innerHTML.substring(2, title.innerHTML.length);
      document.title = xnViewHeader;
    }
    else document.title = document.title + ": " + xnViewHeader;
  }
  else {
    document.getElementById("findmoreform").setAttribute("style","display:none;");
    if (title.innerHTML.match(/^ - /)) {
      title.innerHTML = title.innerHTML.substring(2, title.innerHTML.length);
      document.title = "My Web Albums";
    }
  }
}