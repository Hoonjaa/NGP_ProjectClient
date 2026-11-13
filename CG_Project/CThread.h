#pragma once
#include "pch.h"
#include <queue>

std::queue< std::vector<char> > SendQueue; // 송신 데이터 모아두는 큐

inline DWORD WINAPI SendThread(LPVOID lpParam)
{
	const SOCKET socket = *static_cast<const SOCKET*>(lpParam);
	for (;;) {
		std::vector<char> data;
		if (SendQueue.empty()) Wait.
		data = std::move(SendQueue.front());                   
		SendQueue.pop();

		int size = data.size();
		int offset = 0;
		while (offset < size)
		{
			int sent = send(socket, data.data() + offset, size - offset, 0);
			if (sent <= 0) return;
			offset += sent;
		}	
	}
}

template<typename T>
void Enqueue(T t)
{
	static_assert(std::is_trivially_copyable_v<T>);
	static_assert(sizeof(T) <= 255);

	std::vector<char> data(sizeof(T));
	std::memcpy(data.data(), &t, sizeof(T));
	SendQueue.push(std::move(data));
}

inline DWORD WINAPI RecvThread(LPVOID lpParam)
{

}