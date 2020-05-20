@call storage="snow.ks"
@call storage="clock.ks"
@image layer=base page=fore storage=_24_3
*start1|時計は非表示
時計を表示します[l]
@backlay
@clockopt backvisible=true
@trans method=crossfade time=2000
@wt
*start2|時計は表示中
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
時計を非表示にします[l]
@backlay
@clockopt backvisible=false
@trans method=crossfade time=2000
@wt
@jump target=*start1
