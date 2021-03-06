
#include <iostream>
#include <string>

#include "src/ed25519.h"
#include "base16.h"


int main()
{
	unsigned char private_key[32] = { 0 }, public_key[32] = { 0 };
	std::string str_private_key = "11ec18b7cffacfb46c57e027bd63f6558a78ec4ee4e929c07c6d9c68eb42c218";
	//str_private_key = "26c76712d89d906e6672dafa614c42e5cb1caac8c6568e4d2493087db51f0d36";
	decode_base16(private_key, str_private_key);

	ed25519_pubkey(public_key, private_key);
	std::string str_public_key;
	encode_base16(str_public_key, public_key, sizeof(public_key));
	std::cout << str_public_key << std::endl;

	return 0;
}