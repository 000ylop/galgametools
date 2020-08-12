function Searcher() {
    var spans = document.getElementsByTagName('span');
    var styles = [];
    var text1 = [];
    var text2 = [];
    var spans_length = spans.length;

    for (var i = 0; i < spans_length; ++i) {
        var span = spans[i];
        styles.push(span.parentNode.parentNode.parentNode.parentNode.style);
        text1.push(span.parentNode.childNodes[0].nodeValue);
        text2.push(span.childNodes[0].nodeValue);
    }

    var lastValue = '';

    this.grep = function (str) {
        if (str == lastValue) {
            return;
        } else {
            lastValue = str;
        }

        var regs = [];
        var words = str.split(' ');
        for (var i = 0; i < words.length; ++i) {
            if (words[i] != '') {
                regs.push(new RegExp(words[i].replace(/(\W)/g, "\\$1"), 'i'));
            }
        }

        for (var i = 0; i < spans_length; ++i) {
            var span = spans[i];
            var matched = true;
            for (var j = 0; j < regs.length; ++j) {
                if (!regs[j].test(text1[i]) && !regs[j].test(text2[i])) {
                    matched = false;
                    break;
                }
            }

            styles[i].display = matched ? '' : 'none';
        }
    }
}

var searcher;
function init() {
    searcher = new Searcher();
    setInterval(onTimer, 400);
}

function onTimer() {
    searcher.grep(document.getElementById('grepword').value);
}

window.onload = init;
