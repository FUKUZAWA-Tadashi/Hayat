// hydll.h : hydll.DLL �̃��C�� �w�b�_�[ �t�@�C��
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH �ɑ΂��Ă��̃t�@�C�����C���N���[�h����O�� 'stdafx.h' ���C���N���[�h���Ă�������"
#endif

#include "resource.h"		// ���C�� �V���{��

#include "EngineControl.h"



// ChydllApp
// ���̃N���X�̎����Ɋւ��Ă� hydll.cpp ���Q�Ƃ��Ă��������B
//

class ChydllApp : public CWinApp
{
public:
	ChydllApp();

// �I�[�o�[���C�h
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
