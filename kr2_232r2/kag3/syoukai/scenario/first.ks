@call storage="snow.ks"
; ↑ 雪 plug-in を読み込む
@wait time=200

@eval exp="f.bg=''"
; ↑現在読み込まれている背景を記憶している

; マクロ changebg_and_clear の定義
@macro name="changebg_and_clear"
@if exp="mp.storage != f.bg"
; ↑既に読み込まれている背景と同じならば切り替え処理は行わない
@eval exp="f.bg = mp.storage"
@backlay
@layopt layer=message0 page=back visible=false
@trans method=crossfade time=300
@wt
@image * layer=base page=back
@trans method=crossfade time=300
@wt
@cm
@layopt layer=message0 page=back visible=true
@trans method=crossfade time=300
@wt
@endif
@endmacro

; マクロ loadbg の定義
@macro name="loadbg"
@image * layer=base
@eval exp="f.bg = mp.storage"
@endmacro

; マクロ start_select の定義
@macro name="start_select"
@backlay
@nowait
@history output=false
@current page=back
@endmacro

; マクロ end_select の定義
@macro name="end_select"
@trans method=crossfade time=400
@wt
@endnowait
@history output=true
@current page=fore
@endmacro

*syokai_start|吉里吉里とKAGの紹介 - メニュー
@startanchor

; 背景に画像を読み込み、メッセージレイヤにメニューを描画
@backlay
@loadbg storage="_24_5" page=back
@current page=back
@cm
@layopt layer=message0 page=back visible=true
@nowait
@history output=false
@style align=center
[font size=40 color=0x00ffff]吉里吉里とKAGの紹介[resetfont][r]
[r]
[link target="*about_kirikiri"]吉里吉里とは[endlink][r]
[link target="*about_kag"]KAGとは[endlink][r]
[r]
[r]
[r]
[r]
[r]
[r]
[r]
[link exp="kag.shutdown()" color=0xff0000 hint="吉里吉里/KAGの紹介を終了します"]終了[endlink]
@endnowait
@history output=true
@current page=fore

; メッセージレイヤのトランジション
@trans method=crossfade time=800
@wt

; 通過記録
@record

; 選択肢が選択されるまで停止
@s

*to_syokai_start
; syokai_start に戻る
@backlay
@layopt layer=message0 page=back visible=false
@trans method=crossfade time=300
@wt
@jump target=*syokai_start


*about_kirikiri|吉里吉里とは
@changebg_and_clear storage="_24_4"
　吉里吉里は、TJSというスクリプト言語を使っていろいろな事をするためのソフトウェアです。[l][r]
　TJSはJavaとJavaScriptを足して３で割ったような言語で、CやC++に比べれば習得しやすい言語だと思います。[l][r]
　吉里吉里ではこのTJSで本体を制御することにより、さまざまなアプリケーションを作成することができます。[l][r]
　特にマルチメディア系の機能が強く、比較的静的な表現を用いる２Ｄゲームに適しています。[p]
*about_kirikiri2|
@cm
　吉里吉里は、レイヤと呼ばれる画面を何枚も重ね合わせて画面を構成します。[l]レイヤはアルファブレンドによる重ね合わせが可能で、階層構造を採ることもできます。[l][r]
　レイヤには標準でPNG/JPEG/ERI/BMPを読み込み可能で、Susie-pluginで読み込み可能な形式を拡張することもできます。[l][r]
　描画はあまり複雑なことはできませんが、半透明矩形の描画やアンチエイリアス可能な文字表示、画像の拡大縮小や変形を行う事ができます。[l][r]
　AVI/MPEGやSWF(Macromedia Flash)をムービーとして再生させることができます。[p]
*about_kirikiri3|
@cm
　吉里吉里ではCD-DA、MIDIシーケンスデータ、PCMを再生させることができ、それぞれ音量調節が可能です。[l]PCMは無圧縮の.WAVファイルのほか、プラグインで再生可能な形式を拡張でき、OggVorbisも再生することができます。[l][r]
　PCMは複数を同時に再生することができます。[l]CD-DAやMIDIシーケンスデータでも無理矢理やろうと思えば複数同時再生できます。[p]
*about_kirikiri4
@cm
　その他、周辺ツールとして、
複数のファイルを一つにまとめたり、単体で実行可能なファイルを作成することができる[font color=0xffff00]Releaser[resetfont]、[l]
吉里吉里本体の設定を行う[font color=0xffff00]吉里吉里設定[resetfont]、[l]
制作者側でフォントを用意し、プレーヤ側にフォントがインストールされて無くても使えるようにする[font color=0xffff00]レンダリング済みフォント作成ツール[resetfont]、[l]
透明度を持った画像フォーマット間の相互変換を行う[font color=0xffff00]透過画像フォーマットコンバータ[resetfont]があります。[l]
[r]
[r]
@start_select
[link target=*to_syokai_start]メニューに戻る[endlink]
@end_select
[s]

*about_kag|KAGとは
@changebg_and_clear storage="_24_4"
　KAGは、ビジ○アルノベルやサウンドノベルのようなノベル系ゲームや、選択肢を選んでストーリーが進むような文字ベースのアドベンチャーゲームを作成するためのキットです。[l][r]
　KAGは吉里吉里をゲームエンジンとして動作させるためのスクリプトで、それ自体はTJSスクリプトで書かれています。[l]KAG用のスクリプトは「シナリオ」と呼ばれ、TJSスクリプトとはまた別のものです。[l]TJSスクリプトはプログラミングの知識がかなり必要になりますが、シナリオはより簡単で記述しやすいものです。[l][r]
　KAGは吉里吉里の上に成り立つシステムなので、吉里吉里の機能のほとんどはKAGで使用できます。[p]
*about_kag3|
@cm
　KAGの文字表示は、ご覧の通りのアンチエイリアス文字表示に加え、[l][r]
[font size=60]大きな文字[resetfont]を表示したり、[l][r]
[ruby text="る"]ル[ruby text="び"]ビ[ruby text="を"]を[ruby text="ふ"]振[ruby text="っ"]っ[ruby text="た"]た[ruby text="り"]り、[l][font shadow=false edge=true edgecolor=0xff0000]縁取り文字にしたり[resetfont]、[l][r]
[style align=center]センタリングしてみたり[r]
[style align=right]右詰めしてみたり[r][resetstyle]
[l]
[graph storage="ExQuestion.png" alt="!?"]のような特殊記号を表示してみたり、[l][r]
と、いろいろできます。[p]
*about_kag4|
@position vertical=true
　また、縦書き表示をすることもできます。[l][r]
　縦書きでも横書きと全く同じ機能を使用することができます。[p]
@layopt layer=message0 visible=false
@layopt layer=message1 visible=true
@current layer=message1
@position frame=messageframe left=20 top=280 marginl=16 margint=16 marginr=0 marginb=16 draggable=true vertical=false
　このようにメッセージ枠の中にメッセージを表示させることもできます。[l]アドベンチャーゲームで良くあるタイプです。[p]
@layopt layer=message1 visible=false
@layopt layer=message0 visible=true
@current layer=message0
@position vertical=false
*about_kag5|
@cm
　立ち絵はこのように(あいかわらず[ruby text="・"]う[ruby text="・"]にですみません)
@backlay
@image storage=uni page=back layer=0 visible=true opacity=255
@trans method=crossfade time=1000
@wt
アルファブレンドによる重ね合わせが可能です。[l][r]
　このように
@backlay
@layopt page=back layer=0 opacity=128
@trans method=crossfade time=1000
@wt
薄く表示することもできます。[l][r]
　標準の状態で３つまで重ね合わせて表示できます。[p]
@backlay
@layopt page=back layer=0 visible=false
@trans method=crossfade time=300
@wt
*about_kag6|
@cm
　トランジション(画面切り替え)には標準で３つの種類があります。[l][r]
　一つは単純なクロスフェード、[l]
@backlay
@layopt page=back layer=message0 visible=false
@trans method=crossfade time=300
@wt
@backlay
@image storage="_24" page=back layer=base
@trans method=crossfade time=3000
@wt
@backlay
@image storage="_24_4" page=back layer=base
@trans method=crossfade time=3000
@wt
@backlay
@layopt page=back layer=message0 visible=true
@trans method=crossfade time=300
@wt
[l][r]
　もう一つはスクロール効果を出すことのできるスクロールトランジション、[l]
@backlay
@layopt page=back layer=message0 visible=false
@trans method=crossfade time=300
@wt
@backlay
@image storage="_24" page=back layer=base
@trans method=scroll from=right stay=stayfore time=3000
@wt
@backlay
@image storage="_24_4" page=back layer=base
@trans method=scroll from=bottom stay=nostay time=3000
@wt
@backlay
@layopt page=back layer=message0 visible=true
@trans method=crossfade time=300
@wt
[l][r]
　そして最後は制作者が自由にパターンを作成できるユニバーサルトランジションです。[l][r]
　ユニバーサルトランジションはルール画像と呼ばれるグレースケールの画像を用意し、その画像の暗いところからより早く切り替えが始まるものです。[l][r]
　たとえば、[l]
@image layer=base page=fore storage="trans1"
このようなルール画像であれば・・・[l]
@backlay
@layopt page=back layer=message0 visible=false
@image storage="_24_4" page=back layer=base
@trans method=crossfade time=300
@wt
@backlay
@image storage="_24" page=back layer=base
@trans method=universal rule="trans1" vague=64 time=3000
@wt
@backlay
@image storage="_24_4" page=back layer=base
@trans method=universal rule="trans1" vague=64 time=3000
@wt
@backlay
@layopt page=back layer=message0 visible=true
@trans method=crossfade time=300
@wt
[r]
　たとえば、[l]
@image layer=base page=fore storage="nami"
このようなルール画像であれば・・・[l]
@backlay
@layopt page=back layer=message0 visible=false
@image storage="_24_4" page=back layer=base
@trans method=crossfade time=300
@wt
@backlay
@image storage="_24" page=back layer=base
@trans method=universal rule="nami" vague=64 time=3000
@wt
@backlay
@image storage="_24_4" page=back layer=base
@trans method=universal rule="nami" vague=64 time=3000
@wt
@backlay
@layopt page=back layer=message0 visible=true
@trans method=crossfade time=300
@wt
[r]
　という感じで、いろいろ作ることができます。[p]
*about_kag7|KAGとは
@cm
　BGMとしてはCD-DA、MIDI、PCMのいずれかを使用できます。[l]効果音にはPCMを使用できます。[l]いずれもフェードなどの音量制御ができます。[l][r]
　PCMは標準で無圧縮の.WAVを再生できます。[l]また、プラグインで再生可能な形式を拡張でき、OggVorbisも再生できます。[l][r]
　ムービーはAVI/MPEG/SWFを再生できます。[p]
*about_kag8|
@cm
　KAGの変数は文字列でも数値でも入れることができ、変数の数は無制限、文字列の長さも無制限、数値は整数だけでなく実数も扱うことができます。[l]これはKAGの変数の仕様というか、KAGのベースとなっているTJSの仕様です。[l][r]
　変数にはゲーム変数とシステム変数の２種類あって、ゲーム変数は栞とともに読み込まれたり保存されたりしますが、システム変数は栞とは関係なく、常に同じ内容を保つことができるものです。[l][r]
　変数を使った例を示します・・・[p]
@eval exp="f.v1 = intrandom(1, 9)"
@eval exp="f.v2 = intrandom(1, 9)"
@eval exp="f.ans = f.v1 * f.v2"
@eval exp="f.input = ''"
*about_kag_var_0|計算問題
@cm
　計算問題です。[emb exp="f.v1"] × [emb exp="f.v2"] は？[r]
[font size=20](下の入力欄に入力したら、よこの「OK」をクリックしてください)[resetfont][r]
[r]
@start_select
　[edit name="f.input" length=200 opacity=80 bgcolor=0x000000 color=0xffffff] [link target=*about_kag_var_1]　　　OK　　　[endlink][r]
[r]
　[link target=*about_kag_9]面倒なのでとばす[endlink]
@end_select
@eval exp="kag.fore.messages[0].links[0].object.focus()"
; ↑入力欄にフォーカスを設定する
; 「システム - 前に戻る」でこの位置に戻れるようにここで通過記録を行う
@record
[s]

*about_kag_var_1
@commit
@jump cond="str2num(f.input) == f.ans" target=*about_kag_var_correct
@cm
　不正解！[l][r]
　もう一度入力してください。[p]
@jump target=*about_kag_var_0

*about_kag_var_correct
@cm
　正解です！[p]
@jump target=*about_kag_9

*about_kag_9|
@cm
@snowinit forevisible=true backvisible=false
　KAGの大きな特徴として、その高い拡張性とカスタマイズ性が挙げられます。[l]KAGだけでは実現できないような機能も、TJSを使って直接吉里吉里を制御すればいろいろな事ができます。[l][r]
　たとえば、KAG用プラグインとして「雪」を表示させるプラグインを読み込めば、このように雪を表示させることができます。[l]ほかにもトランジションの種類を増やすプラグインなどがあります。[l][r]
　また、KAGそのものがTJSスクリプトで書かれているため、スクリプトを変更すれば隅々にわたって動作をカスタマイズする事ができます。[p]
@backlay
@snowopt backvisible=false
@trans method=crossfade time=1000
@wt
@snowuninit
*about_kag_fin|KAGの紹介おしまい
@cm
　KAGの紹介はこれでおしまいです。[l][r]
　みなさんも是非吉里吉里/KAGを使ってすばらしいゲームを作ってください。[l][r]
[r]
@start_select
[link exp="System.shellExecute('http://www.piass.com/kpc/')" hint="吉里吉里/KAG推進委員会を開きます"]吉里吉里/KAG推進委員会[endlink][r]
[link exp="System.shellExecute('http://kikyou.info/tvp/')" hint="ダウンロードページを開きます"]吉里吉里ダウンロードページ[endlink][r]
[r]
[link target=*to_syokai_start]メニューに戻る[endlink]
@end_select
[s]
