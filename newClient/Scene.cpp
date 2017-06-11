#include "stdafx.h"
#include "Scene.h"


CScene::CScene()
{
	m_pShaders = NULL;
	m_nShaders = 0;
	texNum = 0;
}

CScene::~CScene()
{
}

void CScene::BuildObjects(ID3D11Device *pd3dDevice)
{
	m_nShaders = 1;
	m_pShaders = new CShader*[m_nShaders];

	m_pShaders[0] = new CTexturedShader(texNum);
	m_pShaders[0]->CreateShader(pd3dDevice);
	m_pShaders[0]->BuildObjects(pd3dDevice);
}

void CScene::ReleaseObjects()
{
	for (int j = 0; j < m_nShaders; j++) m_pShaders[j]->ReleaseObjects();
	if (m_pShaders) delete[] m_pShaders;
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::ProcessInput()
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i]->AnimateObjects(fTimeElapsed);
	}
}

void CScene::Render(ID3D11DeviceContext*pd3dDeviceContext, CCamera *pCamera)
{
	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i]->Render(pd3dDeviceContext, pCamera);
	}
}

void CScene::setTex(int texNum)
{
	this->texNum = texNum;
}