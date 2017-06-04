#include "stdafx.h"
#include "Scene.h"


CScene::CScene()
{
	m_ppObjects = NULL;
	m_nObjects = 0;
}

CScene::~CScene()
{
}

void CScene::BuildObjects(ID3D11Device *pd3dDevice)
{
	CShader *pShader = new CShader();
	pShader->CreateShader(pd3dDevice);

	//��ü(������ü ��ü�� ��� ��ü) 2���� �����Ѵ�.
	m_nObjects = 2;
	m_ppObjects = new CGameObject*[m_nObjects];

	CHumanoidMesh *pHumanMesh = new CHumanoidMesh(pd3dDevice);
	CGameObject *pHumanObject = new CGameObject();
	pHumanObject->SetMesh(pHumanMesh);
	pHumanObject->SetShader(pShader);
	//��� ��ü�� ��ġ�� (20.0f, 0.0f, 0.0f)�� �����Ѵ�.
	pHumanObject->m_d3dxmtxWorld._41 = 0.0f;
	m_ppObjects[0] = pHumanObject;
	pHumanObject->m_d3dxmtxWorld._43 = 300.0f;
	pHumanObject->Rotate(-90.0f, 180.0f, 0.0f);
	m_ppObjects[1] = pHumanObject;
}

void CScene::ReleaseObjects()
{
	//���� ��ü ����Ʈ�� �� ��ü�� ��ȯ(Release)�ϰ� ����Ʈ�� �Ҹ��Ų��.
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
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
	for (int i = 0; i < m_nObjects; i++) m_ppObjects[i]->Animate(fTimeElapsed);
}

void CScene::Render(ID3D11DeviceContext*pd3dDeviceContext, CCamera *pCamera)
{
	//��� ���� ��ü���� �������Ѵ�.
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i]->Render(pd3dDeviceContext);
	}
}