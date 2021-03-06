
#include <iostream>
#include <string>

#include "base16.h"
#include "base58.h"

typedef uint8_t u8;

extern "C" void curve25519_donna(u8 *output, const u8 *secret, const u8 *bp);

int main()
{
	static const unsigned char basepoint[32] = { 9 };
	unsigned char mysecret[32], mypublic[32];
	std::string str_secret = "11ec18b7cffacfb46c57e027bd63f6558a78ec4ee4e929c07c6d9c68eb42c218";
	decode_base16(mysecret, str_secret);

	curve25519_donna(mypublic, mysecret, basepoint);

	std::string str_public;
	encode_base16(str_public, mypublic, sizeof(mypublic));
	// e5e9cf34424f55532fecac393d62bdb8b38b50635c09c7b29da65a678d3eaf47
	std::cout << str_public << std::endl;

	std::string str_address = "TPY8ZS3BvNdXUk2m3Xu2r72Shpn5pqoLFE";
	std::string str_pubhash = decode_base58(str_address);
	// 4194d3a771227a06deb13871295cbe0464f1458c5417dcd7f5
	std::cout << str_pubhash << std::endl;

	getchar();
	return 0;
}
