#include "stdafx.h"
#include "Shader.h"

ID3D11Buffer *CShader::m_pd3dcbWorldMatrix = NULL;

BYTE *ReadCompiledEffectFile(WCHAR *pszFileName, int *pnReadBytes)
{
	FILE *pFile = NULL;
	::_wfopen_s(&pFile, pszFileName, L"rb");
	::fseek(pFile, 0, SEEK_END);
	int nFileSize = ::ftell(pFile);
	BYTE *pByteCode = new BYTE[nFileSize];
	::rewind(pFile);
	*pnReadBytes = ::fread(pByteCode, sizeof(BYTE), nFileSize, pFile);
	::fclose(pFile);
	return(pByteCode);
}

CShader::CShader()
{
	m_ppObjects = NULL;
	m_nObjects = 0;

	m_pd3dVertexShader = NULL;
	m_pd3dVertexLayout = NULL;
	m_pd3dPixelShader = NULL;
}

CShader::~CShader()
{
	if (m_pd3dVertexShader) m_pd3dVertexShader->Release();
	if (m_pd3dVertexLayout) m_pd3dVertexLayout->Release();
	if (m_pd3dPixelShader) m_pd3dPixelShader->Release();
}

void CShader::BuildObjects(ID3D11Device *pd3dDevice)
{
}

void CShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}
}

void CShader::AnimateObjects(float fTimeElapsed)
{
	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->Animate(fTimeElapsed);
	}
}

void CShader::OnPrepareRender(ID3D11DeviceContext *pd3dDeviceContext)
{
	pd3dDeviceContext->IASetInputLayout(m_pd3dVertexLayout);
	pd3dDeviceContext->VSSetShader(m_pd3dVertexShader, NULL, 0);
	pd3dDeviceContext->PSSetShader(m_pd3dPixelShader, NULL, 0);
}

void CShader::CreateVertexShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11VertexShader **ppd3dVertexShader, D3D11_INPUT_ELEMENT_DESC *pd3dInputLayout, UINT nElements, ID3D11InputLayout **ppd3dVertexLayout)
{
	HRESULT hResult;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *pd3dShaderBlob = NULL, *pd3dErrorBlob = NULL;
	/*����(pszFileName)���� ���̴� �Լ�(pszShaderName)�� �������Ͽ� �����ϵ� ���̴� �ڵ��� �޸� �ּ�(pd3dShaderBlob)�� ��ȯ�Ѵ�.*/
	if (SUCCEEDED(hResult = D3DX11CompileFromFile(pszFileName, NULL, NULL, pszShaderName, pszShaderModel, dwShaderFlags, 0, NULL, &pd3dShaderBlob, &pd3dErrorBlob, NULL)))
	{
		//�����ϵ� ���̴� �ڵ��� �޸� �ּҿ��� ����-���̴��� �����Ѵ�. 
		pd3dDevice->CreateVertexShader(pd3dShaderBlob->GetBufferPointer(), pd3dShaderBlob->GetBufferSize(), NULL, ppd3dVertexShader);
		//�����ϵ� ���̴� �ڵ��� �޸� �ּҿ� �Է� ���̾ƿ����� ���� ���̾ƿ��� �����Ѵ�. 
		pd3dDevice->CreateInputLayout(pd3dInputLayout, nElements, pd3dShaderBlob->GetBufferPointer(), pd3dShaderBlob->GetBufferSize(), ppd3dVertexLayout);
		pd3dShaderBlob->Release();
	}
}

void CShader::CreatePixelShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11PixelShader **ppd3dPixelShader)
{
	HRESULT hResult;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *pd3dShaderBlob = NULL, *pd3dErrorBlob = NULL;
	/*����(pszFileName)���� ���̴� �Լ�(pszShaderName)�� �������Ͽ� �����ϵ� ���̴� �ڵ��� �޸� �ּ�(pd3dShaderBlob)�� ��ȯ�Ѵ�.*/
	if (SUCCEEDED(hResult = D3DX11CompileFromFile(pszFileName, NULL, NULL, pszShaderName, pszShaderModel, dwShaderFlags, 0, NULL, &pd3dShaderBlob, &pd3dErrorBlob, NULL)))
	{
		//�����ϵ� ���̴� �ڵ��� �޸� �ּҿ��� �ȼ�-���̴��� �����Ѵ�. 
		pd3dDevice->CreatePixelShader(pd3dShaderBlob->GetBufferPointer(), pd3dShaderBlob->GetBufferSize(), NULL, ppd3dPixelShader);
		pd3dShaderBlob->Release();
	}
}

void CShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputLayout);
	CreateVertexShaderFromFile(pd3dDevice, L"VS.fx", "VS", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"PS.fx", "PS", "ps_5_0", &m_pd3dPixelShader);
	//�����ϵ� ���̴� �ڵ��� �̸��� VS.fxo�� PS.fxo�̴�.

	CreateShaderVariables(pd3dDevice);
}

void CShader::Render(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext);
	//���̴� ��ü�� ��� ���� ��ü���� �������Ѵ�.
	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			if (m_ppObjects[j]->m_pTexture) UpdateShaderVariables(pd3dDeviceContext, m_ppObjects[j]->m_pTexture);
			m_ppObjects[j]->Render(pd3dDeviceContext);
		}
	}
}

void CShader::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
	//���� ��ȯ ����� ���� ��� ���۸� �����Ѵ�.
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VS_CB_WORLD_MATRIX);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pd3dDevice->CreateBuffer(&bd, NULL, &m_pd3dcbWorldMatrix);
}

void CShader::ReleaseShaderVariables()
{
	//���� ��ȯ ����� ���� ��� ���۸� ��ȯ�Ѵ�.
	if (m_pd3dcbWorldMatrix) m_pd3dcbWorldMatrix->Release();
}

void CShader::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, D3DXMATRIX *pd3dxmtxWorld)
{
	//���� ��ȯ ����� ��� ���ۿ� �����Ѵ�.
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbWorldMatrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	VS_CB_WORLD_MATRIX *pcbWorldMatrix = (VS_CB_WORLD_MATRIX *)d3dMappedResource.pData;
	D3DXMatrixTranspose(&pcbWorldMatrix->m_d3dxmtxWorld, pd3dxmtxWorld);
	pd3dDeviceContext->Unmap(m_pd3dcbWorldMatrix, 0);

	//��� ���۸� ����̽��� ����(VS_SLOT_WORLD_MATRIX)�� �����Ѵ�.
	pd3dDeviceContext->VSSetConstantBuffers(VS_SLOT_WORLD_MATRIX, 1, &m_pd3dcbWorldMatrix);
}

void CShader::UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext, CTexture *pTexture)
{
	pd3dDeviceContext->PSSetShaderResources(PS_SLOT_TEXTURE, pTexture->m_nTextures, pTexture->m_ppd3dsrvTextures);
	pd3dDeviceContext->PSSetSamplers(PS_SLOT_SAMPLER_STATE, pTexture->m_nTextures, pTexture->m_ppd3dSamplerStates);
}

CSceneShader::CSceneShader()
{
}

CSceneShader::~CSceneShader()
{
}

void CSceneShader::CreateShader(ID3D11Device *pd3dDevice)
{
	CShader::CreateShader(pd3dDevice);
}

void CSceneShader::BuildObjects(ID3D11Device *pd3dDevice)
{
	CCubeMesh *pCubeMesh = new CCubeMesh(pd3dDevice, 12.0f, 12.0f, 12.0f, D3DCOLOR_XRGB(0, 0, 128));

	int xObjects = 5, yObjects = 5, zObjects = 5, i = 0;
	m_nObjects = (xObjects * 2 + 1)*(yObjects * 2 + 1)*(zObjects * 2 + 1);
	m_ppObjects = new CGameObject*[m_nObjects];

	float fxPitch = 12.0f * 2.5f;
	float fyPitch = 12.0f * 2.5f;
	float fzPitch = 12.0f * 2.5f;
	CRotatingObject *pRotatingObject = NULL;
	for (int x = -xObjects; x <= xObjects; x++)
	{
		for (int y = -yObjects; y <= yObjects; y++)
		{
			for (int z = -zObjects; z <= zObjects; z++)
			{
				pRotatingObject = new CRotatingObject();
				pRotatingObject->SetMesh(pCubeMesh);
				//pObject->SetShader(pShader);
				pRotatingObject->SetPosition(fxPitch*x, fyPitch*y, fzPitch*z);
				pRotatingObject->SetRotationAxis(D3DXVECTOR3(0.0f, 1.0f, 0.0f));
				pRotatingObject->SetRotationSpeed(10.0f*(i % 10));
				m_ppObjects[i++] = pRotatingObject;
			}
		}
	}
}

CPlayerShader::CPlayerShader()
{
}

CPlayerShader::~CPlayerShader()
{
}

void CPlayerShader::CreateShader(ID3D11Device *pd3dDevice)
{
	CShader::CreateShader(pd3dDevice);
}

void CPlayerShader::BuildObjects(ID3D11Device *pd3dDevice)
{
	m_nObjects = 1;
	m_ppObjects = new CGameObject*[m_nObjects];

	ID3D11DeviceContext *pd3dDeviceContext = NULL;
	pd3dDevice->GetImmediateContext(&pd3dDeviceContext);

	CMesh *pAirplaneMesh = new CAirplaneMesh(pd3dDevice, 20.0f, 20.0f, 4.0f, D3DCOLOR_XRGB(0, 255, 0));
	CAirplanePlayer *pAirplanePlayer = new CAirplanePlayer(pd3dDevice);
	pAirplanePlayer->SetMesh(pAirplaneMesh);
	pAirplanePlayer->CreateShaderVariables(pd3dDevice);
	pAirplanePlayer->ChangeCamera(pd3dDevice, FIRST_PERSON_CAMERA, 0.0f);

	CCamera *pCamera = pAirplanePlayer->GetCamera();
	pCamera->SetViewport(pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	pCamera->GenerateProjectionMatrix(1.01f, 50000.0f, ASPECT_RATIO, 60.0f);
	pCamera->GenerateViewMatrix();

	pAirplanePlayer->SetPosition(D3DXVECTOR3(0.0f, 10.0f, -50.0f));
	m_ppObjects[0] = pAirplanePlayer;

	if (pd3dDeviceContext) pd3dDeviceContext->Release();
}

void CPlayerShader::Render(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera)
{
	//3��Ī ī�޶��� �� �÷��̾ �������Ѵ�.
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA)
	{
		CShader::Render(pd3dDeviceContext, pCamera);
	}
}

CTexturedShader::CTexturedShader(int texNum)
{
	this->texNum = texNum;
}

CTexturedShader::~CTexturedShader()
{
}

void CTexturedShader::CreateShader(ID3D11Device *pd3dDevice)
{
	CShader::CreateShader(pd3dDevice);

	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputLayout);
	CreateVertexShaderFromFile(pd3dDevice, L"VS.fx", "VSTexturedColor", "vs_4_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"PS.fx", "PSTexturedColor", "ps_4_0", &m_pd3dPixelShader);
}

void CTexturedShader::BuildObjects(ID3D11Device *pd3dDevice)
{
	//�ؽ��� ���ο� ����� ���÷� ���� ��ü�� �����Ѵ�.
	ID3D11SamplerState *pd3dSamplerState = NULL;
	D3D11_SAMPLER_DESC d3dSamplerDesc;
	ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = 0;
	pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);

	/*������ü�� ������ �ؽ��� ��ü�� �����Ѵ�. �̹��� ������ � �̹����� ��������Ƿ� ������ ������ �̸��� ����϶�.*/
	ID3D11ShaderResourceView *pd3dTexture = NULL;
	CTexture **ppTextures = new CTexture*[12];
	ppTextures[0] = new CTexture(1);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("resource\\Texture\\Character\\Mentis.jpg"), NULL, NULL, &pd3dTexture, NULL);
	ppTextures[0]->SetTexture(0, pd3dTexture, pd3dSamplerState);
	ppTextures[1] = new CTexture(1);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("resource\\Texture\\Character\\Beetle.jpg"), NULL, NULL, &pd3dTexture, NULL);
	ppTextures[1]->SetTexture(0, pd3dTexture, pd3dSamplerState);
	ppTextures[2] = new CTexture(1);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("resource\\Texture\\Character\\Bee.jpg"), NULL, NULL, &pd3dTexture, NULL);
	ppTextures[2]->SetTexture(0, pd3dTexture, pd3dSamplerState);

	ppTextures[3] = new CTexture(1);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("resource\\Texture\\forest\\concrete_roadblock_d.jpg"), NULL, NULL, &pd3dTexture, NULL);
	ppTextures[3]->SetTexture(0, pd3dTexture, pd3dSamplerState);
	ppTextures[4] = new CTexture(1);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("resource\\Texture\\forest\\Concrite Bar.jpg"), NULL, NULL, &pd3dTexture, NULL);
	ppTextures[4]->SetTexture(0, pd3dTexture, pd3dSamplerState);
	ppTextures[5] = new CTexture(1);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("resource\\Texture\\forest\\MADEIRA2.jpg"), NULL, NULL, &pd3dTexture, NULL);
	ppTextures[5]->SetTexture(0, pd3dTexture, pd3dSamplerState);
	ppTextures[6] = new CTexture(1);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("resource\\Texture\\forest\\MADEIRA3.jpg"), NULL, NULL, &pd3dTexture, NULL);
	ppTextures[6]->SetTexture(0, pd3dTexture, pd3dSamplerState);
	ppTextures[7] = new CTexture(1);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("resource\\Texture\\forest\\TELHA.jpg"), NULL, NULL, &pd3dTexture, NULL);
	ppTextures[7]->SetTexture(0, pd3dTexture, pd3dSamplerState);
	ppTextures[8] = new CTexture(1);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("resource\\Texture\\forest\\TELHA2.jpg"), NULL, NULL, &pd3dTexture, NULL);
	ppTextures[8]->SetTexture(0, pd3dTexture, pd3dSamplerState);
	ppTextures[9] = new CTexture(1);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("resource\\Texture\\forest\\TELHA3.jpg"), NULL, NULL, &pd3dTexture, NULL);
	ppTextures[9]->SetTexture(0, pd3dTexture, pd3dSamplerState);
	ppTextures[10] = new CTexture(1);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("resource\\Texture\\forest\\WoodTexture.png"), NULL, NULL, &pd3dTexture, NULL);
	ppTextures[10]->SetTexture(0, pd3dTexture, pd3dSamplerState);

	ppTextures[11] = new CTexture(1);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("resource\\Texture\\forest\\images.jpg"), NULL, NULL, &pd3dTexture, NULL);
	ppTextures[11]->SetTexture(0, pd3dTexture, pd3dSamplerState);

	CMesh *pMentisMeshTextured = new CMentisMesh(pd3dDevice, 12.0f, 12.0f, 12.0f);
	CMesh *pBeetleMeshTextured = new CBeetleMesh(pd3dDevice, 12.0f, 12.0f, 12.0f);
	CMesh *pBeeMeshTextured = new CBeeMesh(pd3dDevice, 12.0f, 12.0f, 12.0f);

	CMesh *pWallMeshTextured = new CWallMesh(pd3dDevice, 12.0f, 12.0f, 12.0f);
	CMesh *pWall2MeshTextured = new CWallMesh2(pd3dDevice, 12.0f, 12.0f, 12.0f);
	CMesh *plogMeshTextured = new ClogMesh(pd3dDevice, 12.0f, 12.0f, 12.0f);
	CMesh *pHouseMeshTextured = new CHouseMesh(pd3dDevice, 12.0f, 12.0f, 12.0f);

	CMesh *pGroundMeshTextured = new CTexturedGroundMesh(pd3dDevice, 5000.0f, 0.5f, 5000.0f);

	//�ؽ��� ���ε� ������ü�� ����� �ؽ��� ������ ����� ������ü�� ����� ��ġ�� ���̴�.
	m_nObjects = 11;
	m_ppObjects = new CGameObject*[m_nObjects];

	CRotatingObject *pRotatingObject = NULL;
	pRotatingObject = new CRotatingObject();
	pRotatingObject->SetMesh(pMentisMeshTextured);
	pRotatingObject->SetTexture(ppTextures[0]);
	pRotatingObject->SetPosition(0.0, 0.0, 500.0);
	pRotatingObject->SetRotationAxis(D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	pRotatingObject->SetRotationSpeed(0);
	pRotatingObject->Rotate(-90.0, 180.0f, 0.0f);
	m_ppObjects[0] = pRotatingObject;

	pRotatingObject = new CRotatingObject();
	pRotatingObject->SetMesh(pBeetleMeshTextured);
	pRotatingObject->SetTexture(ppTextures[1]);
	pRotatingObject->SetPosition(700.0, 0.0, 500.0);
	pRotatingObject->SetRotationAxis(D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	pRotatingObject->SetRotationSpeed(0);
	pRotatingObject->Rotate(-90.0, 180.0f, 0.0f);
	m_ppObjects[1] = pRotatingObject;

	pRotatingObject = new CRotatingObject();
	pRotatingObject->SetMesh(pBeeMeshTextured);
	pRotatingObject->SetTexture(ppTextures[2]);
	pRotatingObject->SetPosition(-700.0, 0.0, 500.0);
	pRotatingObject->SetRotationAxis(D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	pRotatingObject->SetRotationSpeed(0);
	pRotatingObject->Rotate(-90.0, 180.0f, 0.0f);
	m_ppObjects[2] = pRotatingObject;

	pRotatingObject = new CRotatingObject();
	pRotatingObject->SetMesh(pGroundMeshTextured);
	pRotatingObject->SetTexture(ppTextures[11]);
	pRotatingObject->SetPosition(0.0, -225.0, 0.0);
	pRotatingObject->SetRotationAxis(D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	pRotatingObject->SetRotationSpeed(0);
	pRotatingObject->Rotate(0.0, 0.0f, 0.0f);
	m_ppObjects[3] = pRotatingObject;

	pRotatingObject = new CRotatingObject();
	pRotatingObject->SetMesh(pWallMeshTextured);
	pRotatingObject->SetTexture(ppTextures[4]);
	pRotatingObject->SetPosition(-2500.0, 350.0, 0.0);
	pRotatingObject->SetRotationAxis(D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	pRotatingObject->SetRotationSpeed(0);
	pRotatingObject->Rotate(90.0, 0.0f, 90.0f);
	pRotatingObject->Scale(3550.0, 700.0, 175.0);
	m_ppObjects[4] = pRotatingObject;

	pRotatingObject = new CRotatingObject();
	pRotatingObject->SetMesh(pWallMeshTextured);
	pRotatingObject->SetTexture(ppTextures[4]);
	pRotatingObject->SetPosition(2500.0, 350.0, 0.0);
	pRotatingObject->SetRotationAxis(D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	pRotatingObject->SetRotationSpeed(0);
	pRotatingObject->Rotate(90.0, 180.0f, 90.0f);
	pRotatingObject->Scale(3550.0, 700.0, 175.0);
	m_ppObjects[5] = pRotatingObject;

	pRotatingObject = new CRotatingObject();
	pRotatingObject->SetMesh(pWallMeshTextured);
	pRotatingObject->SetTexture(ppTextures[4]);
	pRotatingObject->SetPosition(-1450.0, 350.0, 2500.0);
	pRotatingObject->SetRotationAxis(D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	pRotatingObject->SetRotationSpeed(0);
	pRotatingObject->Rotate(90.0, 90.0f, 90.0f);
	pRotatingObject->Scale(1550.0, 700.0, 175.0);
	m_ppObjects[6] = pRotatingObject;

	pRotatingObject = new CRotatingObject();
	pRotatingObject->SetMesh(pWallMeshTextured);
	pRotatingObject->SetTexture(ppTextures[4]);
	pRotatingObject->SetPosition(1450.0, 350.0, 2500.0);
	pRotatingObject->SetRotationAxis(D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	pRotatingObject->SetRotationSpeed(0);
	pRotatingObject->Rotate(90.0, 90.0f, 90.0f);
	pRotatingObject->Scale(1550.0, 700.0, 175.0);
	m_ppObjects[7] = pRotatingObject;

	pRotatingObject = new CRotatingObject();
	pRotatingObject->SetMesh(pWallMeshTextured);
	pRotatingObject->SetTexture(ppTextures[4]);
	pRotatingObject->SetPosition(-1450.0, 350.0, -2500.0);
	pRotatingObject->SetRotationAxis(D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	pRotatingObject->SetRotationSpeed(0);
	pRotatingObject->Rotate(90.0, -90.0f, 90.0f);
	pRotatingObject->Scale(1550.0, 700.0, 175.0);
	m_ppObjects[8] = pRotatingObject;

	pRotatingObject = new CRotatingObject();
	pRotatingObject->SetMesh(pWallMeshTextured);
	pRotatingObject->SetTexture(ppTextures[4]);
	pRotatingObject->SetPosition(1450.0, 350.0, -2500.0);
	pRotatingObject->SetRotationAxis(D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	pRotatingObject->SetRotationSpeed(0);
	pRotatingObject->Rotate(90.0, -90.0f, 90.0f);
	pRotatingObject->Scale(1550.0, 700.0, 175.0);
	m_ppObjects[9] = pRotatingObject;

	pRotatingObject = new CRotatingObject();
	pRotatingObject->SetMesh(pWall2MeshTextured);
	pRotatingObject->SetTexture(ppTextures[3]);
	pRotatingObject->SetPosition(0.0, -230.0, 300.0);
	pRotatingObject->SetRotationAxis(D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	pRotatingObject->SetRotationSpeed(0);
	pRotatingObject->Rotate(-90.0, 0.0f, 0.0f);
	pRotatingObject->Scale(70.0, 70.0, 120.0);
	m_ppObjects[10] = pRotatingObject;

	CreateShaderVariables(pd3dDevice);

	delete[] ppTextures;
}