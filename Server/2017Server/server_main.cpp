
#include <WinSock2.h>
#include <winsock.h>
#include <Windows.h>

#include <iostream>
#include <thread>
#include <vector>
#include <unordered_set>
#include <mutex>

using namespace std;

#include "protocol.h"

HANDLE ghIOCP;
SOCKET gsServer;

enum EVENTTYPE { E_RECV, E_SEND };

struct WSAOVERLAPPED_EX {
	WSAOVERLAPPED over;
	WSABUF wsabuf;
	unsigned char IOCP_buf[MAX_BUFF_SIZE];
	EVENTTYPE event_type;
};

struct ClientInfo {
	int x, y;
	volatile bool bConnected;

	SOCKET s;
	WSAOVERLAPPED_EX recv_over;
	unsigned char packet_buf[MAX_PACKET_SIZE];
	int prev_recv_size;
	int curr_packet_size;
	unordered_set <int> view_list;
	mutex vl_lock;
};

ClientInfo gclients[MAX_USER];

void error_display(char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L"에러" << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
	while (true);
}

bool IsNear(int from, int to) {
	return VIEW_RADIUS * VIEW_RADIUS >= ((gclients[from].x - gclients[to].x) 
		*(gclients[from].x - gclients[to].x) 
		+ (gclients[from].y - gclients[to].y) 
		* (gclients[from].y - gclients[to].y));
}


void InitializeServer()
{
	std::wcout.imbue(std::locale("korean"));

	for (int i = 0; i < MAX_USER; ++i) {
		gclients[i].bConnected = false;
	}


	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	ghIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);

	gsServer = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr = INADDR_ANY;

	::bind(gsServer, reinterpret_cast<sockaddr *>(&ServerAddr), sizeof(ServerAddr));
	int res = listen(gsServer, 5);
	if (0 != res)
		error_display("Init Listen Error : ", WSAGetLastError());
}

void CloseServer()
{
	closesocket(gsServer);
	WSACleanup();
}

void NetworkError()
{
	std::cout << "Something Wrong\n";
}

void SendPacket(int cl, unsigned char *packet)
{
	std::cout << "Send Packet[" << static_cast<int>(packet[1]) << "] to Client : " << cl << std::endl;
	WSAOVERLAPPED_EX *send_over = new WSAOVERLAPPED_EX;
	ZeroMemory(send_over, sizeof(*send_over));
	send_over->event_type = E_SEND;
	memcpy(send_over->IOCP_buf, packet, packet[0]);
	send_over->wsabuf.buf = reinterpret_cast<CHAR *>(send_over->IOCP_buf);
	send_over->wsabuf.len = packet[0];
	DWORD send_flag = 0;
	WSASend(gclients[cl].s, &send_over->wsabuf, 1, NULL,
		send_flag, &send_over->over, NULL);
}

void SendPositionPacket(int to, int object)
{
	sc_packet_pos packet;
	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_POS;
	packet.x = gclients[object].x;
	packet.y = gclients[object].y;

	SendPacket(to, reinterpret_cast<unsigned char *>(&packet));
}

void SendPutPlayerPacket(int target_client, int new_client)
{
	sc_packet_put_player packet;
	packet.id = new_client;
	packet.size = sizeof(packet);
	packet.type = SC_PUT_PLAYER;
	packet.x = gclients[new_client].x;
	packet.y = gclients[new_client].y;

	SendPacket(target_client, reinterpret_cast<unsigned char *>(&packet));
}

void SendRemovePlayerPacket(int target_client, int new_client)
{
	sc_packet_remove_player packet;
	packet.id = new_client;
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_PLAYER;

	SendPacket(target_client, reinterpret_cast<unsigned char *>(&packet));
}

void ProcessPacket(int cl, unsigned char *packet)
{
	std::cout << "Packet [" << packet[1] << "] from Client :" << cl << std::endl;
	switch (packet[1])
	{
	case CS_UP:
		if (0 < gclients[cl].y) gclients[cl].y--;
		break;
	case CS_DOWN:
		if (BOARD_HEIGHT - 1 > gclients[cl].y) gclients[cl].y++;
		break;
	case CS_LEFT:
		if (0 < gclients[cl].x) gclients[cl].x--;
		break;
	case CS_RIGHT:
		if (BOARD_WIDTH - 1 > gclients[cl].x) gclients[cl].x++;
		break;
	default:
		std::cout << "Unknown Packet Type from Client[" << cl << "]\n";
		exit(-1);
	}

	unordered_set<int> new_view_list;
	for (int i = 0; i < MAX_USER;++i)
		if ((gclients[i].bConnected) && (IsNear(cl, i)))
			new_view_list.insert(i);

	for (auto id : new_view_list) {
		gclients[cl].vl_lock.lock();
		// 보이지 않다가 보이게 된 객체 처리 : 시야 리스트에 존재하지 않았던 객체
		if (0 == gclients[cl].view_list.count(id)) {
			gclients[cl].view_list.insert(id);
			gclients[cl].vl_lock.unlock();
			SendPutPlayerPacket(cl, id);
			gclients[cl].vl_lock.lock();
			if (0 == gclients[id].view_list.count(cl)) {
				gclients[id].view_list.insert(cl);
				gclients[cl].vl_lock.unlock();
				SendPutPlayerPacket(id, cl);
			}
			else {
				gclients[cl].vl_lock.unlock();
				SendPositionPacket(id, cl);
			}
		}

		else {
			gclients[cl].vl_lock.unlock();
			// 계속 보이고 있는 객체 처리
			gclients[id].vl_lock.lock();
			if (0 == gclients[id].view_list.count(cl)) {
				gclients[id].view_list.insert(cl);
				gclients[id].vl_lock.unlock();
				SendPutPlayerPacket(id, cl);
			}
			else {
				gclients[id].vl_lock.unlock();
				SendPositionPacket(id, cl);
			}
		}
	}
	gclients[cl].vl_lock.lock();
	unordered_set <int> localviewlist = gclients[cl].view_list;
	gclients[cl].vl_lock.unlock();
	for (auto id : localviewlist) {
		if (0 == new_view_list.count(id)) {
			gclients[cl].vl_lock.lock();
			gclients[cl].view_list.erase(id);
			gclients[cl].vl_lock.unlock();
			SendRemovePlayerPacket(cl, id);
			gclients[id].vl_lock.lock();
			if (0 != gclients[id].view_list.count(cl)) {
				gclients[id].view_list.erase(cl);
				gclients[id].vl_lock.unlock();
				SendRemovePlayerPacket(id, cl);
			}
			else {

				gclients[id].vl_lock.unlock();
			}
		}

	}
}

void DisconnectClient(int cl)
{
	closesocket(gclients[cl].s);
	gclients[cl].bConnected = false;
	for (int i = 0; i < MAX_USER;++i)
		if (true == gclients[i].bConnected) {
			gclients[i].vl_lock.lock();
			if (0 != gclients[i].view_list.count(cl)) {
				SendRemovePlayerPacket(i, cl);
				gclients[i].view_list.erase(cl);
				gclients[i].vl_lock.unlock();
			}
			else {

				gclients[i].vl_lock.unlock();
			}
		}
}

void WorkerThread()
{
	while (true)
	{
		DWORD io_size;
		unsigned long long cl;
		WSAOVERLAPPED_EX *pOver;
		BOOL is_ok = GetQueuedCompletionStatus(ghIOCP, &io_size, &cl,
			reinterpret_cast<LPWSAOVERLAPPED *>(&pOver), INFINITE);

		std::cout << "GQCS : Event";
		if (false == is_ok)
		{
			int err_no = WSAGetLastError();
			if (64 == err_no) DisconnectClient(cl);
			else error_display("GQCS Error : ", WSAGetLastError());
		}

		if (0 == io_size) {
			DisconnectClient(cl);
			continue;
		}

		if (E_RECV == pOver->event_type) {
			std::cout << "  data from Client :" << cl;
			int to_process = io_size;
			unsigned char *buf_ptr = gclients[cl].recv_over.IOCP_buf;
			unsigned char packet_buf[MAX_PACKET_SIZE];
			int psize = gclients[cl].curr_packet_size;
			int pr_size = gclients[cl].prev_recv_size;
			while (0 != to_process) {
				if (0 == psize) psize = buf_ptr[0];
				if (psize <= to_process + pr_size) {
					memcpy(packet_buf, gclients[cl].packet_buf, pr_size);
					memcpy(packet_buf + pr_size, buf_ptr, psize - pr_size);
					ProcessPacket(static_cast<int>(cl), packet_buf);
					to_process -= psize - pr_size; buf_ptr += psize - pr_size;
					psize = 0; pr_size = 0;
				}
				else {
					memcpy(gclients[cl].packet_buf + pr_size, buf_ptr, to_process);
					pr_size += to_process;
					buf_ptr += to_process;
					to_process = 0;
				}
			}
			gclients[cl].curr_packet_size = psize;
			gclients[cl].prev_recv_size = pr_size;
			DWORD recvFlag = 0;
			int ret = WSARecv(gclients[cl].s, &gclients[cl].recv_over.wsabuf,
				1, NULL, &recvFlag, &gclients[cl].recv_over.over,
				NULL);
			if (0 != ret) {
				int err_no = WSAGetLastError();
				if (WSA_IO_PENDING != err_no)
					error_display("Recv Error in worker thread", err_no);
			}
		}
		else if (E_SEND == pOver->event_type) {
			std::cout << "Send Complete to Client : " << cl << std::endl;
			if (io_size != pOver->IOCP_buf[0]) {
				std::cout << "Incomplete Packet Send Error!\n";
				exit(-1);
			}
			delete pOver;
		}
		else {
			std::cout << "Unknown GQCS Event Type!\n";
			exit(-1);
		}
	}
}

void AcceptThread()
{
	SOCKADDR_IN clientAddr;
	ZeroMemory(&clientAddr, sizeof(SOCKADDR_IN));
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(MY_SERVER_PORT);
	clientAddr.sin_addr.s_addr = INADDR_ANY;
	int addr_len = sizeof(clientAddr);
	while (true) {
		SOCKET sClient = WSAAccept(gsServer, reinterpret_cast<sockaddr *>(&clientAddr), &addr_len, NULL, NULL);

		if (INVALID_SOCKET == sClient)
			error_display("Accept Thread Accept Error :", WSAGetLastError());
		std::cout << "New Client Arrived! :";
		int new_client_id = -1;
		for (int i = 0; i < MAX_USER; ++i)
			if (false == gclients[i].bConnected) {
				new_client_id = i;
				break;
			}
		if (-1 == new_client_id) {
			closesocket(sClient);
			std::cout << "Max User Overflow!!!\n";
			continue;
		}
		std::cout << "ID = " << new_client_id << std::endl;
		//ZeroMemory(&gclients[new_client_id], sizeof(gclients[new_client_id]));
		gclients[new_client_id].curr_packet_size = 0;
		gclients[new_client_id].prev_recv_size = 0;
		gclients[new_client_id].vl_lock.lock();
		gclients[new_client_id].view_list.clear();
		gclients[new_client_id].vl_lock.unlock();
		gclients[new_client_id].s = sClient;
		gclients[new_client_id].bConnected = true;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(sClient), ghIOCP, new_client_id, 0);
		ZeroMemory(&(gclients[new_client_id].recv_over.over), sizeof(WSAOVERLAPPED));
		gclients[new_client_id].recv_over.event_type = E_RECV;
		gclients[new_client_id].recv_over.wsabuf.buf = reinterpret_cast<CHAR *>(gclients[new_client_id].recv_over.IOCP_buf);
		gclients[new_client_id].recv_over.wsabuf.len = sizeof(gclients[new_client_id].recv_over.IOCP_buf);
		DWORD recvFlag = 0;
		int ret = WSARecv(sClient, &gclients[new_client_id].recv_over.wsabuf,
			1, NULL, &recvFlag, &gclients[new_client_id].recv_over.over,
			NULL);
		if (0 != ret) {
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				error_display("Recv Error in Accept Thread ", err_no);
		}
		SendPutPlayerPacket(new_client_id, new_client_id);

		unordered_set <int> locallist;

		for (int i = 0; i < MAX_USER; ++i)
			if (true == gclients[i].bConnected)
				if (i != new_client_id) {
					if (IsNear(i, new_client_id)) {
						gclients[i].vl_lock.lock();
						if (0 == gclients[i].view_list.count(new_client_id)) {
							gclients[i].view_list.insert(new_client_id);
							gclients[i].vl_lock.unlock();
							SendPutPlayerPacket(i, new_client_id);
						}
						else gclients[i].vl_lock.unlock();
						locallist.insert(i);
						SendPutPlayerPacket(new_client_id, i);
					}
					gclients[new_client_id].vl_lock.lock();
					for (auto p : locallist) gclients[new_client_id].view_list;
					gclients[new_client_id].vl_lock.unlock();
				}
	}
}

int main()
{
	std::vector <std::thread *> worker_threads;
	//
	InitializeServer();
	//
	for (int i = 0; i < 6; ++i)
		worker_threads.push_back(new std::thread{ WorkerThread });
	//
	std::thread accept_thread{ AcceptThread };
	accept_thread.join();
	for (auto pth : worker_threads) { pth->join(); delete pth; }
	worker_threads.clear();
	CloseServer();
}