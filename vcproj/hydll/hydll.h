// hydll.h : hydll.DLL のメイン ヘッダー ファイル
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル

#include "EngineControl.h"



// ChydllApp
// このクラスの実装に関しては hydll.cpp を参照してください。
//

class ChydllApp : public CWinApp
{
public:
	ChydllApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
