@call storage="snow.ks"
@call storage="clock.ks"
@image layer=base page=fore storage=_24_3
*start1|���v�͔�\��
���v��\�����܂�[l]
@backlay
@clockopt backvisible=true
@trans method=crossfade time=2000
@wt
*start2|���v�͕\����
�w�i�����ւ��܂�[l]
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
���v���\���ɂ��܂�[l]
@backlay
@clockopt backvisible=false
@trans method=crossfade time=2000
@wt
@jump target=*start1
