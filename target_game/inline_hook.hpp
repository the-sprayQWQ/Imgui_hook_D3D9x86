#pragma once
#include <Windows.h>

constexpr int byte_length = 5;

class inline_hook {
private:
	using uchar = unsigned char;
	using dword = DWORD;
	uchar m_orginal_byte[byte_length]; //原始函数处的汇编指令大小
	uchar m_self_byte[byte_length];   //我们自己构造的汇编指令大小

	int m_orginal_address; //原始函数地址
	int m_self_address; //自己构造的函数地址

	dword motify_memory_attributes(int address,dword attributes = PAGE_EXECUTE_READWRITE) {
		dword old_attributes;
		VirtualProtect(reinterpret_cast<void *>(address),byte_length,attributes,&old_attributes);
		return old_attributes;
	}


public:
	inline_hook(int original_address, int self_address) :m_orginal_address(original_address), m_self_address(self_address) {
		m_self_byte[0] = '\xe9';
		//计算偏移
		int offset = self_address - (original_address + byte_length);
		
		//构造跳转到我们地址的字节 例如 E9 12 34 56 78
		memcpy(&m_self_byte[1],&offset, byte_length - 1);
		//修改内存属性
		dword attributes = motify_memory_attributes(original_address);

		//保存原始字节用来还原
		memcpy(m_orginal_byte, reinterpret_cast<void*>(original_address), byte_length);

		//恢复原有属性
		motify_memory_attributes(original_address, attributes);
	}

	void motify_address(){
		dword attributes = motify_memory_attributes(m_orginal_address);
		memcpy(reinterpret_cast<void*>(m_orginal_address), m_self_byte, byte_length);
		motify_memory_attributes(m_orginal_address, attributes);
	}

	void restory_address(){
		dword attributes = motify_memory_attributes(m_orginal_address);
		memcpy(reinterpret_cast<void*>(m_orginal_address), m_orginal_byte, byte_length);
		motify_memory_attributes(m_orginal_address, attributes);
	}
};