; Config.tjs にて freeSaveDataMode は false に、
; saveThumbnail は true に設定する必要があります
@call storage="rclick_tjs.ks"
@image layer=base page=fore storage=_24_3
*loop|右クリックしてね
@cm
右[link]クリック[endlink]してみてください[l]
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
はい[p]
@jump target=*loop
