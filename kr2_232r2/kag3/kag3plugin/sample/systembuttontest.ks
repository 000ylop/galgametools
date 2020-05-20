@call storage="snow.ks"
@call storage="systembutton.ks"
@image layer=base page=fore storage=_24_3
@l
*start1|システムボタンは非表示
システムボタンを表示します[l]
@backlay
@sysbtopt backvisible=true left=&intrandom(0,500) top=&intrandom(0,300)
@trans method=crossfade time=2000
@wt
*start2|システムボタンは表示中
背景を入れ替えます[l]
@backlay
@image layer=base page=back storage=_24
@trans method=crossfade time=2000
@wt
@l
@backlay
@image layer=base page=back storage=_24_3
@trans children=false method=crossfade time=2000
@wt
@l
システムボタンを非表示にします[l]
@backlay
@sysbtopt backvisible=false
@trans method=crossfade time=2000
@wt
@jump target=*start1
