; Config.tjs �ɂ� freeSaveDataMode �� false �ɁA
; saveThumbnail �� true �ɐݒ肷��K�v������܂�
@call storage="rclick_tjs.ks"
@image layer=base page=fore storage=_24_3
*loop|�E�N���b�N���Ă�
@cm
�E[link]�N���b�N[endlink]���Ă݂Ă�������[l]
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
�͂�[p]
@jump target=*loop
