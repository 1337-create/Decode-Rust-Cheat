#pragma once
#include "stdint.h"
#include "imports.h"

// Make sure the values match on cheat files aswell
constexpr auto packet_magic = 0x7F000001;   // use https://www.browserling.com/tools/ip-to-hex to convert IP to hex value
constexpr auto server_ip = 0x7F000001; // 127.0.0.1 Never change this
constexpr auto server_port = 65215;  // Random Port

enum class PacketType
{
	packet_copy_memory,
	packet_get_base_address,
	packet_completed
};

struct PacketCopyMemory
{
	uint32_t dest_process_id;
	uint64_t dest_address;

	uint32_t src_process_id;
	uint64_t src_address;

	uint32_t size;
};

struct PacketGetBaseAddress
{
	uint32_t process_id;
	int name;
};

struct PackedCompleted
{
	uint64_t result;
};

struct PacketHeader
{
	uint32_t   magic;
	PacketType type;
};

struct Packet
{
	PacketHeader header;
	union
	{
		PacketCopyMemory	 copy_memory;
		PacketGetBaseAddress get_base_address;
		PackedCompleted		 completed;
	} data;
};