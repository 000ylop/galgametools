@call storage="snow.ks"
@call storage="systembutton.ks"
@image layer=base page=fore storage=_24_3
@l
*start1|�V�X�e���{�^���͔�\��
�V�X�e���{�^����\�����܂�[l]
@backlay
@sysbtopt backvisible=true left=&intrandom(0,500) top=&intrandom(0,300)
@trans method=crossfade time=2000
@wt
*start2|�V�X�e���{�^���͕\����
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
�V�X�e���{�^�����\���ɂ��܂�[l]
@backlay
@sysbtopt backvisible=false
@trans method=crossfade time=2000
@wt
@jump target=*start1
