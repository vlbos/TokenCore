#ifndef TOKENCOMMON_H_
#define TOKENCOMMON_H_

#include <string>
#include <vector>
#include <algorithm>
#include <stdint.h>
#include <string.h>

using namespace std;

#include "u256.h"

//#include <boost/multiprecision/cpp_int.hpp>
//using u256 = boost::multiprecision::uint256_t;

class Binary : public vector<unsigned char>
{
public:
	Binary() : vector<unsigned char>() {}
	Binary(size_t count) : vector<unsigned char>(count) {}
	Binary(vector<unsigned char>::iterator begin, vector<unsigned char>::iterator end) : vector<unsigned char>(begin, end) {}
	Binary(string::const_iterator begin, string::const_iterator end)  { this->assign(begin, end); }
	Binary(const char* str) { this->assign(str, str + strlen(str)); }
	Binary(const char* str, int len) { this->assign(str, str + len); }
	Binary(const char* str, size_t len) { this->assign(str, str + len); }
	Binary(const unsigned char* data, int len) { this->assign(data, data + len); }
	Binary(const unsigned char* data, size_t len) { this->assign(data, data + len); }
	Binary(const string str) { this->assign(str.begin(), str.end()); }

	Binary& operator+=(const Binary& bin)
	{
		copy(bin.begin(), bin.end(), back_inserter(*this));
		return *this;
	}

	Binary operator+(const Binary& bin) const
	{
		Binary out(*this);

		out += bin;
		return(out);
	}

	bool operator== (const Binary& bin) const
	{
		if (this->size() != bin.size())
			return false;

		return memcmp(this->data(), bin.data(), this->size()) == 0;
	}

	bool operator!= (const Binary& bin) const
	{
		return(!operator==(bin));
	}

	size_t to(char* buffer, int size)
	{
		size_t len = (size_t)size > this->size() ? this->size() : size;
		copy(this->begin(), this->begin() + len, buffer);
		if ((size_t)size > this->size())
			buffer[len] = 0;
		return len;
	}

	size_t to(unsigned char* buffer, int size)
	{
		size_t len = (size_t)size > this->size() ? this->size() : size;
		copy(this->begin(), this->begin() + len, buffer);
		return len;
	}

	size_t to(string str)
	{
		str.assign(this->begin(), this->end());
		return this->size();
	}

	Binary& reverse()
	{
		std::reverse(begin(), end());
		return *this;
	}

	// ��ͬ�� encode_base16
	static string encode(const Binary& bin)
	{
		string str;
		if (0 == bin.size())
			return str;

		str.reserve((bin.size()) * 2);

		auto to_hex = [](unsigned char c)
		{
			if (c < 10)
				return(c + 0x30);
			return(c - 10 + 'a');
		};

		for (size_t i = 0; i < bin.size(); i++)
		{
			str.push_back(to_hex(bin[i] >> 4));
			str.push_back(to_hex(bin[i] & 0x0f));
		}

		return str;
	}

	// ��ͬ�� decode_base16
	static Binary decode(const string& str)
	{
		Binary bin;

		if (str.size() % 2 != 0)
			return bin;

		auto is_base16 = [](const char c)
		{
			return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
		};

		if (!all_of(str.begin(), str.end(), is_base16))
			return bin;

		auto from_hex = [](const char c)
		{
			if ('A' <= c && c <= 'F')
				return 10 + c - 'A';
			if ('a' <= c && c <= 'f')
				return 10 + c - 'a';
			return c - '0';
		};

		bin.resize(str.size() / 2);
		for (size_t i = 0; i < bin.size(); i++)
		{
			bin[i] = (from_hex(str[i * 2]) << 4) + from_hex(str[i * 2 + 1]);
		}

		return bin;
	}

	Binary left(size_t count)
	{
		size_t num = this->size() > count ? count : this->size();
		if (num == 0)
			return Binary();
		return Binary(this->begin(), this->begin() + num);
	}

	Binary right(size_t count)
	{
		size_t num = this->size() > count ? count : this->size();
		if (num == 0)
			return Binary();
		return Binary(this->end() - num, this->end());
	}

	Binary mid(size_t position, size_t len)
	{
		if (position >= this->size())
			return Binary();

		size_t epos = position + len;
		if (epos >= this->size())
			epos = this->size();

		if (len == 0)
			return Binary();
		return Binary(this->begin() + position, this->begin() + epos);
	}

	// all.insert(all.end(), checksum.begin(), checksum.begin() + 4);
};

struct Utxo
{
	string hash;
	uint32_t index;
	uint64_t value;
	string script;
};

struct UserTransaction
{
	// ��������
	string from_address;		// �����ַ
	string to_address;			// �տ��ַ
	string change_address;		// �����ַ
	u256 pay;					// ������
	u256 fee_count;				// ���׷ѵ� gas ����
	u256 fee_price;				// ���׷ѵ� gas ���ۣ�fee = fee_count * fee_price
	u256 nonce;					// �������(ETH)
	string contract_address;	// ��Լ��ַ

	int from_wallet_index;		// �������˽Կ������
	int change_wallet_index;	// �����ַ���ӹ�Կ������
	vector<Utxo> utxo_list;

	// �ڲ�ʹ��
	int  input_count;			// ����ĸ���
	Binary public_key;			// �����ַ�Ĺ�Կ
	vector<Binary> sig_data;	// ǩ�������б�

	// �������
	string tx_str;				// ���ɵ� tx ��

	void clear(int mode = 0)
	{
		nonce = 0;
		input_count = from_wallet_index = change_wallet_index = 0;
		contract_address.clear();
		public_key.clear();
		utxo_list.clear();
		sig_data.clear();
		tx_str.clear();

		if (mode == 1)
		{
			from_address.clear();
			to_address.clear();
			change_address.clear();
			pay = fee_count = fee_price = 0;
		}
	}

	UserTransaction()
	{
		clear(1);
	}
};

// ����ר��
void VF(const char* hint, bool check);

// ����ָ�����ȵ�α�������
string gen_seed(size_t bit_length);

// ����·�������ӻ�ȡBIP44��׼�ķֲ�˽Կ
string bip44_get_private_key(const string bip39_seed, const string bip32_path);

// ������ֵ��ȡ���Ǵ�
string entropy_to_mnemonic(const string entropy);

// ͨ�����Ǵ���������(passphraseΪBIP38���ܶ���)
string mnemonic_to_seed(const string mnemonic, const string passphrase);

// �������Ǵ��Ƿ���Ϲ���
bool check_mnemonic(const string mnemonic);

#endif // TOKENCOMMON_H_
