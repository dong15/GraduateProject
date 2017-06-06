// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����:
#include <WinSock2.h>
#include <windows.h>
#include <windowsx.h>

// C ��Ÿ�� ��� �����Դϴ�.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
#include <d3d11.h>
#include <d3dx11.h>

#define	FRAME_BUFFER_WIDTH		640
#define	FRAME_BUFFER_HEIGHT		480

#include <mmsystem.h>
#include <math.h>

#include <d3dcompiler.h>  	//���̴� ������ �Լ��� ����ϱ� ���� ��� ����
#include <D3DX10Math.h>	//Direct3D ���� �Լ��� ����ϱ� ���� ��� ����

#define VS_SLOT_CAMERA				0x00
#define VS_SLOT_WORLD_MATRIX			0x01
#define PS_SLOT_COLOR	0x00

#include <stdio.h>
#include <iostream>
#include <fstream>
#include "..\Server\2017Server\protocol.h"



#pragma comment (lib, "ws2_32.lib")