#include "stdafx.h"
#include "Mesh.h"

CMesh::CMesh(ID3D11Device *pd3dDevice)
{
	m_nBuffers = 0;
	m_ppd3dVertexBuffers = NULL;
	m_pnVertexStrides = NULL;
	m_pnVertexOffsets = NULL;

	m_pd3dPositionBuffer = NULL;
	m_pd3dColorBuffer = NULL;

	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_nSlot = 0;
	m_nStartVertex = 0;

	m_pd3dIndexBuffer = NULL;
	m_nIndices = 0;
	m_nStartIndex = 0;
	m_nBaseVertex = 0;
	m_nIndexOffset = 0;
	m_dxgiIndexFormat = DXGI_FORMAT_R32_UINT;

	m_pd3dRasterizerState = NULL;

	m_nReferences = 0;
}

CMesh::~CMesh()
{
	if (m_pd3dRasterizerState) m_pd3dRasterizerState->Release();
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();

	if (m_ppd3dVertexBuffers) delete[] m_ppd3dVertexBuffers;
	if (m_pnVertexStrides) delete[] m_pnVertexStrides;
	if (m_pnVertexOffsets) delete[] m_pnVertexOffsets;
}

void CMesh::AssembleToVertexBuffer(int nBuffers, ID3D11Buffer **ppd3dBuffers, UINT *pnBufferStrides, UINT *pnBufferOffsets)
{
	ID3D11Buffer **ppd3dNewVertexBuffers = new ID3D11Buffer*[m_nBuffers + nBuffers];
	UINT *pnNewVertexStrides = new UINT[m_nBuffers + nBuffers];
	UINT *pnNewVertexOffsets = new UINT[m_nBuffers + nBuffers];

	if (m_nBuffers > 0)
	{
		for (int i = 0; i < m_nBuffers; i++)
		{
			ppd3dNewVertexBuffers[i] = m_ppd3dVertexBuffers[i];
			pnNewVertexStrides[i] = m_pnVertexStrides[i];
			pnNewVertexOffsets[i] = m_pnVertexOffsets[i];
		}
		if (m_ppd3dVertexBuffers) delete[] m_ppd3dVertexBuffers;
		if (m_pnVertexStrides) delete[] m_pnVertexStrides;
		if (m_pnVertexOffsets) delete[] m_pnVertexOffsets;
	}

	for (int i = 0; i < nBuffers; i++)
	{
		ppd3dNewVertexBuffers[m_nBuffers + i] = ppd3dBuffers[i];
		pnNewVertexStrides[m_nBuffers + i] = pnBufferStrides[i];
		pnNewVertexOffsets[m_nBuffers + i] = pnBufferOffsets[i];
	}

	m_nBuffers += nBuffers;
	m_ppd3dVertexBuffers = ppd3dNewVertexBuffers;
	m_pnVertexStrides = pnNewVertexStrides;
	m_pnVertexOffsets = pnNewVertexOffsets;
}

void CMesh::AddRef()
{
	m_nReferences++;
}

void CMesh::Release()
{
	m_nReferences--;
	if (m_nReferences <= 0) delete this;
}

void CMesh::Render(ID3D11DeviceContext *pd3dDeviceContext)
{
	pd3dDeviceContext->IASetVertexBuffers(m_nSlot, m_nBuffers, m_ppd3dVertexBuffers, m_pnVertexStrides, m_pnVertexOffsets);
	pd3dDeviceContext->IASetIndexBuffer(m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset);
	pd3dDeviceContext->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dDeviceContext->RSSetState(m_pd3dRasterizerState);

	if (m_pd3dIndexBuffer)
		pd3dDeviceContext->DrawIndexed(m_nIndices, m_nStartIndex, m_nBaseVertex);
	else
		pd3dDeviceContext->Draw(m_nVertices, m_nStartVertex);
}

void CMesh::CreateRasterizerState(ID3D11Device *pd3dDevice)
{
}

CCubeMesh::CCubeMesh(ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth, D3DXCOLOR d3dxColor) : CMesh(pd3dDevice)
{
	m_nVertices = 8;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;

	CDiffusedVertex pd3dxvPositions[8];
	pd3dxvPositions[0] = CDiffusedVertex(D3DXVECTOR3(-fx, -fy, -fz), d3dxColor + RANDOM_COLOR);
	pd3dxvPositions[1] = CDiffusedVertex(D3DXVECTOR3(-fx, -fy, +fz), d3dxColor + RANDOM_COLOR);
	pd3dxvPositions[2] = CDiffusedVertex(D3DXVECTOR3(+fx, -fy, +fz), d3dxColor + RANDOM_COLOR);
	pd3dxvPositions[3] = CDiffusedVertex(D3DXVECTOR3(+fx, -fy, -fz), d3dxColor + RANDOM_COLOR);
	pd3dxvPositions[4] = CDiffusedVertex(D3DXVECTOR3(-fx, +fy, -fz), d3dxColor + RANDOM_COLOR);
	pd3dxvPositions[5] = CDiffusedVertex(D3DXVECTOR3(-fx, +fy, +fz), d3dxColor + RANDOM_COLOR);
	pd3dxvPositions[6] = CDiffusedVertex(D3DXVECTOR3(+fx, +fy, +fz), d3dxColor + RANDOM_COLOR);
	pd3dxvPositions[7] = CDiffusedVertex(D3DXVECTOR3(+fx, +fy, -fz), d3dxColor + RANDOM_COLOR);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(D3DXVECTOR3) * m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pd3dxvPositions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	D3DXCOLOR pd3dxColors[8];
	for (int i = 0; i < 8; i++) pd3dxColors[i] = RANDOM_COLOR;

	d3dBufferDesc.ByteWidth = sizeof(D3DXCOLOR) * m_nVertices;
	d3dBufferData.pSysMem = pd3dxColors;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dColorBuffer);

	ID3D11Buffer *ppd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dColorBuffer };
	UINT pnBufferStrides[2] = { sizeof(D3DXVECTOR3), sizeof(D3DXCOLOR) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, ppd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_nIndices = 36;

	UINT pnIndices[36];
	pnIndices[0] = 3; pnIndices[1] = 1; pnIndices[2] = 0;
	pnIndices[3] = 2; pnIndices[4] = 1; pnIndices[5] = 3;
	pnIndices[6] = 0; pnIndices[7] = 5; pnIndices[8] = 4;
	pnIndices[9] = 1; pnIndices[10] = 5; pnIndices[11] = 0;
	pnIndices[12] = 3; pnIndices[13] = 4; pnIndices[14] = 7;
	pnIndices[15] = 0; pnIndices[16] = 4; pnIndices[17] = 3;
	pnIndices[18] = 1; pnIndices[19] = 6; pnIndices[20] = 5;
	pnIndices[21] = 2; pnIndices[22] = 6; pnIndices[23] = 1;
	pnIndices[24] = 2; pnIndices[25] = 7; pnIndices[26] = 6;
	pnIndices[27] = 3; pnIndices[28] = 7; pnIndices[29] = 2;
	pnIndices[30] = 6; pnIndices[31] = 4; pnIndices[32] = 5;
	pnIndices[33] = 7; pnIndices[34] = 4; pnIndices[35] = 6;

	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(UINT) * m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pnIndices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);
}

CCubeMesh::~CCubeMesh()
{
}

void CCubeMesh::CreateRasterizerState(ID3D11Device *pd3dDevice)
{
	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroMemory(&d3dRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	d3dRasterizerDesc.CullMode = D3D11_CULL_BACK;
	d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	pd3dDevice->CreateRasterizerState(&d3dRasterizerDesc, &m_pd3dRasterizerState);
}

void CCubeMesh::Render(ID3D11DeviceContext *pd3dDeviceContext)
{
	CMesh::Render(pd3dDeviceContext);
}

CHumanoidMesh::CHumanoidMesh(ID3D11Device *pd3dDevice) : CMesh(pd3dDevice)
{
	std::ifstream mesh;
	mesh.open("model\\Mentis.pos");
	//파일을 열어 정점의 개수와 정점의 위치 벡터들을 순서대로 읽는다.
	//int nReadBytes = ::fread(&m_nVertices, sizeof(UINT), 1, pFile);
	mesh >> m_nVertices;
	D3DXVECTOR3*pd3dxvPositions = new D3DXVECTOR3[m_nVertices];
	//nReadBytes = ::fread(pd3dxvPositions, sizeof(D3DXVECTOR3), m_nVertices, pFile);
	for (int i = 0; i < m_nVertices; ++i)
		mesh >> pd3dxvPositions[i].x >> pd3dxvPositions[i].y >> pd3dxvPositions[i].z;
	mesh.close();

	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	//정점은 두 개의 요소를 가지고 각 요소마다 하나의 버퍼를 생성한다.
	/*정점의 첫 번째 요소는 위치 벡터이고 위치 벡터 한 개는 12바이트이다. 정점의 첫 번째 요소를 나타내는 버퍼를 생성한다.*/
	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(D3DXVECTOR3) * m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pd3dxvPositions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);
	delete[] pd3dxvPositions;

	//정점 두 번째 요소는 색상 벡터이고 색상 벡터 한 개는 16바이트이다.
	D3DXCOLOR *pd3dxColors = new D3DXCOLOR[m_nVertices];
	for (int i = 0; i < m_nVertices; i++) pd3dxColors[i] = RANDOM_COLOR;

	//정점의 두 번째 요소를 나타내는 버퍼를 생성한다.
	d3dBufferDesc.ByteWidth = sizeof(D3DXCOLOR) * m_nVertices;
	d3dBufferData.pSysMem = pd3dxColors;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dColorBuffer);
	delete[] pd3dxColors;

	//정점을 나타내는 두 개의 버퍼와 정보를 입력 조립기로 전달할 수 있는 형태로 구성한다.
	ID3D11Buffer *ppd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dColorBuffer };
	UINT pnBufferStrides[2] = { sizeof(D3DXVECTOR3), sizeof(D3DXCOLOR) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, ppd3dBuffers, pnBufferStrides, pnBufferOffsets);

	//이진 파일을 열어 인덱스의 개수와 인덱스를 읽는다.
	mesh.open("model\\Mentis.inx");
	mesh >> m_nIndices;
	m_nIndices *= 3;
	UINT *pnIndices = new UINT[m_nIndices];
	for (int i = 0; i < m_nIndices; ++i)
		mesh >> pnIndices[i];
	mesh.close();

	//인덱스 버퍼를 생성한다.
	::ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(UINT) * m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	::ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pnIndices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);

	delete[] pnIndices;
}

CHumanoidMesh::~CHumanoidMesh()
{
}