#include "server_shared.h"
#include "sockets.h"
#include "JUNK.h"

extern uint64_t handle_incoming_packet(const Packet& packet);
extern bool		complete_request(SOCKET connection_client, uint64_t result);

static SOCKET create_process()
{
	SOCKADDR_IN address{ };

	address.sin_family	= AF_INET;
	address.sin_port	= htons(server_port);

	const auto process_listen = socket_listen(AF_INET, SOCK_STREAM, 0);
	if (process_listen == INVALID_SOCKET)
	{
		return INVALID_SOCKET;
	}

	if (bind(process_listen, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR)
	{

		closesocket(process_listen);
		return INVALID_SOCKET;
	}

	if (listen(process_listen, 10) == SOCKET_ERROR)
	{

		closesocket(process_listen);
		return INVALID_SOCKET;
	}

	return process_listen;
}

static void NTAPI mainThreadLoop(void* connection_socket)
{
	J();
	const auto connection_client = SOCKET(ULONG_PTR(connection_socket));

	Packet packet{ };
	while (true)
	{
		const auto result = recv(connection_client, (void*)&packet, sizeof(packet), 0);
		if (result <= 0)
			break;

		if (result < sizeof(PacketHeader))
			continue;

		if (packet.header.magic != packet_magic)
			continue;

		const auto packet_result = handle_incoming_packet(packet);
		if (!complete_request(connection_client, packet_result))
			break;
	}

	closesocket(connection_client);
}

void NTAPI startThread(void*)
{
	J();
	auto status = KsInitialize();
	if (!NT_SUCCESS(status))
	{
		J();
		return;
	}

	const auto process_listen = create_process();
	if (process_listen == INVALID_SOCKET)
	{
		J();
		KsDestroy();
		return;
	}


	while (true)
	{
		J();
		sockaddr  socket_addr{ };
		socklen_t socket_length{ };

		const auto connection_client = accept(process_listen, &socket_addr, &socket_length);
		if (connection_client == INVALID_SOCKET)
		{
			break;
		}

		HANDLE handle_zone = nullptr;

		status = PsCreateSystemThread(
			&handle_zone,
			GENERIC_ALL,
			nullptr,
			nullptr,
			nullptr,
			mainThreadLoop,
			(void*)connection_client
		);

		if (!NT_SUCCESS(status))
		{
			closesocket(connection_client);
			break;
		}

		ZwClose(handle_zone);
	}

	closesocket(process_listen);
}