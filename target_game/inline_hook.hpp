#pragma once
#include <Windows.h>

constexpr int byte_length = 5;

class inline_hook {
private:
	using uchar = unsigned char;
	using dword = DWORD;
	uchar m_orginal_byte[byte_length]; //ԭʼ�������Ļ��ָ���С
	uchar m_self_byte[byte_length];   //�����Լ�����Ļ��ָ���С

	int m_orginal_address; //ԭʼ������ַ
	int m_self_address; //�Լ�����ĺ�����ַ

	dword motify_memory_attributes(int address,dword attributes = PAGE_EXECUTE_READWRITE) {
		dword old_attributes;
		VirtualProtect(reinterpret_cast<void *>(address),byte_length,attributes,&old_attributes);
		return old_attributes;
	}


public:
	inline_hook(int original_address, int self_address) :m_orginal_address(original_address), m_self_address(self_address) {
		m_self_byte[0] = '\xe9';
		//����ƫ��
		int offset = self_address - (original_address + byte_length);
		
		//������ת�����ǵ�ַ���ֽ� ���� E9 12 34 56 78
		memcpy(&m_self_byte[1],&offset, byte_length - 1);
		//�޸��ڴ�����
		dword attributes = motify_memory_attributes(original_address);

		//����ԭʼ�ֽ�������ԭ
		memcpy(m_orginal_byte, reinterpret_cast<void*>(original_address), byte_length);

		//�ָ�ԭ������
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