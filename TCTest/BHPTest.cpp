#include <iostream>
#include <BhpApi.h>

#include "Coin.h"
#include "HttpAPI\BtxonAPI.h"
#include "CosExt.h"

using namespace std;
using namespace BHPAPI;

// 测试用
// 私钥: fca599567eb49585c2c091f257551b31beeed4bd0eea68cc48bfac25507a847e
// 公钥: 03ac092fcc4f20dd38c3ff8fccc24e3fe7e153145aaed9cac7814838936cfb322d
// 地址: AJz7xuTeZS3qG2zCgtP4Cs6kNyTBFPDiHw

static void test_get_private_key()
{
	string private_key = BHPAPI::get_private_key("37f63c464ed1e319103598012d13b5f48f4712fdd55766390eeb58f3812d71ef3da4d5eec187b80bc3896d95e3e7aaead526bedd999c3dd74ba0b137a9b194ae");
	VF("get_private_key", (private_key == "94f628597b09c9664887657afcdf3366fab9d4b4824ac1380b4514b3e680c19c"));
}

static void test_get_public_key()
{
	string public_key1 = get_public_key("fdd7596639575967305867a31cb219a27e4b73cfc22894bca2c62a9c4dc7a2c6");
	VF("get_public_key", (public_key1 == "02ef512ebc820ec8977fa955056c296ea137ba6c3c32742281e15dfbe4277a8b44"));

	string public_key2 = get_public_key("68a1639fa801be966c6da92570f7a24caa26618061b4a3248b03247d7fef418a");
	VF("get_public_key", (public_key2 == "03d1998c163fc5137bb2ccc8062968db728081857ecf9d5c489c93935ab1aeece9"));
}

static void test_get_address()
{
	string address1 = get_address("02ef512ebc820ec8977fa955056c296ea137ba6c3c32742281e15dfbe4277a8b44");
	VF("get_address", (address1 == "AHkJJa79Rrwijoco3v34L4hu5CUVqSSmNy"));

	string address2 = get_address("03d1998c163fc5137bb2ccc8062968db728081857ecf9d5c489c93935ab1aeece9");
	VF("get_address", (address2 == "AUgM1yNpeQb23kZrKPBPxGNvCEgXiDgiVo"));
}

static void test_make_unsign_tx(UserTransaction* ut)
{
	make_unsign_tx(ut);
	VF("make_unsign_tx", (ut->tx_str == string("80000001fc5b8a6f2b52a650c0d9d1b39a568c98c136f00756b1d8f90b9aa3ac10c7944e01000254a80a4c72f6157a7af0a753fc4ac4af6b159a17634dd57fecf319feab6ff713bff4f50e000000008d8dfca8e949d5548e3193b88b59d682e3426b1c54a80a4c72f6157a7af0a753fc4ac4af6b159a17634dd57fecf319feab6ff713316c5c5b0000000015a40075f96efb432420b1d599dc5b3a52dca5e0")));
}

// 签名无法测试，因为每次都不一样
static void test_sign_tx(UserTransaction* ut)
{
	string private_key = "68a1639fa801be966c6da92570f7a24caa26618061b4a3248b03247d7fef418a";
	sign_tx(ut, private_key);
	//VF("sign_tx", (ut->sig_data[0] == "692bc6e7e52feea2e166fba7ea0b9caf7d427aef2d785103ce6afe09c9b2ca9ca7bfc52650f3330396b104b15027e3790808cf4da624163cfb07c39a55e5d007"));
}

static void test_make_sign_tx(UserTransaction* ut)
{
	ut->sig_data[0] = Binary::decode("692bc6e7e52feea2e166fba7ea0b9caf7d427aef2d785103ce6afe09c9b2ca9ca7bfc52650f3330396b104b15027e3790808cf4da624163cfb07c39a55e5d007");
	make_sign_tx(ut);

	VF("make_sign_tx", (ut->tx_str == "80000001fc5b8a6f2b52a650c0d9d1b39a568c98c136f00756b1d8f90b9aa3ac10c7944e01000254a80a4c72f6157a7af0a753fc4ac4af6b159a17634dd57fecf319feab6ff713bff4f50e000000008d8dfca8e949d5548e3193b88b59d682e3426b1c54a80a4c72f6157a7af0a753fc4ac4af6b159a17634dd57fecf319feab6ff713316c5c5b0000000015a40075f96efb432420b1d599dc5b3a52dca5e0014140692bc6e7e52feea2e166fba7ea0b9caf7d427aef2d785103ce6afe09c9b2ca9ca7bfc52650f3330396b104b15027e3790808cf4da624163cfb07c39a55e5d007232103d1998c163fc5137bb2ccc8062968db728081857ecf9d5c489c93935ab1aeece9ac"));
}

// 这个是 OK 的
static int firmware_sign_transaction(CoinType& coin_type, UserTransaction* ut)
{
	Cos cos;
	SW sw;

	if (cos.find() <= 0)
		return -1;

	if (!cos.connect(0))
		return -1;

	if (!cos.open_channel())
		return -1;

	if (verify_pin(cos) != 0)
		return -1;

	int retcode = BHPAPI::make_unsign_tx(ut);

	int result_size;
	char result[4096];

	Binary fdata = BHPAPI::firmware_prepare_data(ut);
	printf("firmware_data:\n");
	binout(fdata.data(), fdata.size());
	printf("firmware_size:%d\n", (int)fdata.size());

	sw = cos.sign_transaction(coin_type.major, coin_type.minor, coin_type.chain_id, 0, fdata.data(), fdata.size(), result, result_size);
	if (sw != SW_6D82_GET_INTERACTION_LATER)
	{
		printf("签名失败:%04x\n", sw);
		return -1;
	}
	char buffer[1024];
	printf("输入验证码:");
	scanf_s("%s", buffer, 1024);

	sw = cos.sign_transaction_end(buffer, result, result_size);
	if (sw != SW_9000_SUCCESS)
	{
		printf("验证失败\n");
		return -1;
	}

	BHPAPI::firmware_process_result(ut, result, result_size);
	BHPAPI::make_sign_tx(ut);

	printf("%s\n", ut->tx_str.c_str());

	return 0;
}

// 硬件签名的交易过程
static void test_firmware_sign()
{
	UserTransaction ut;
	ut.from_address = "AbyHP1BnfytNpyG3fXThEC7VH8rDN894uW";
	ut.to_address = "AUgM1yNpeQb23kZrKPBPxGNvCEgXiDgiVo";		// string str_pubkey_hash_1 = "8d8dfca8e949d5548e3193b88b59d682e3426b1c";	
	ut.change_address = "AbyHP1BnfytNpyG3fXThEC7VH8rDN894uW";
	ut.pay = (uint64_t)gCoin["tBHP"].from_display("100.1");
	ut.from_wallet_index = 0;
	ut.change_wallet_index = 0;

	CoinType coin_type = gCoin["tBHP"].type;

	BtxonAPI api;
	int ret = api.getUTXO(coin_type, ut.from_address, ut.utxo_list);
	if (ret || ut.utxo_list.size() == 0)
		return;

	// 必须在取得 utxo 后才能去算交易费
	int len = get_tx_len(&ut);
	ut.fee_count = (len + 511) / 512;
	if (ut.fee_count > 5)
		ut.fee_count = 5;
	ut.fee_price = 10000;

	// 硬件签名
	firmware_sign_transaction(coin_type, &ut);
}

// 软件签名的交易过程测试
static void test_sign()
{
	UserTransaction ut;
	ut.from_address = "AHkJJa79Rrwijoco3v34L4hu5CUVqSSmNy";		// string str_pubkey_hash_2 = "15a40075f96efb432420b1d599dc5b3a52dca5e0";
	ut.to_address = "AUgM1yNpeQb23kZrKPBPxGNvCEgXiDgiVo";		// string str_pubkey_hash_1 = "8d8dfca8e949d5548e3193b88b59d682e3426b1c";	
	ut.change_address = "AHkJJa79Rrwijoco3v34L4hu5CUVqSSmNy";
	ut.pay = uint64_t(2.51 * 100000000);

	Utxo utxo;
	utxo.hash = "4e94c710aca39a0bf9d8b15607f036c1988c569ab3d1d9c050a6522b6f8a5bfc";
	utxo.index = 1;
	utxo.value = uint64_t(5000 * 100000000);
	ut.utxo_list.push_back(utxo);

	test_make_unsign_tx(&ut);
	test_sign_tx(&ut);
	test_make_sign_tx(&ut);
}

// 这个是软签名的方式给我们自己的账户打点钱
int MakeBhpTransaction_sp()
{
	UserTransaction ut;
	ut.from_address = "AUgM1yNpeQb23kZrKPBPxGNvCEgXiDgiVo";		// string str_pubkey_hash_2 = "15a40075f96efb432420b1d599dc5b3a52dca5e0";
	ut.to_address = "AbyHP1BnfytNpyG3fXThEC7VH8rDN894uW";		// 模拟器钱包的地址
	ut.change_address = "AUgM1yNpeQb23kZrKPBPxGNvCEgXiDgiVo";
	ut.pay = (uint64_t)gCoin["tBHP"].from_display("800");
	ut.from_wallet_index = 0;
	ut.change_wallet_index = 0;

	CoinType& coin_type = gCoin["tBHP"].type;

	BtxonAPI api;
	int ret = api.getUTXO(coin_type, ut.from_address, ut.utxo_list);
	if (ret)
		return ret;

	BHPAPI::make_unsign_tx(&ut);

	string private_key = "68a1639fa801be966c6da92570f7a24caa26618061b4a3248b03247d7fef418a";
	BHPAPI::sign_tx(&ut, private_key);

	BHPAPI::make_sign_tx(&ut);

	printf("%s\n", ut.tx_str.c_str());

	return 0;
}

void BHPTest()
{
	//test_get_private_key();
	//test_get_public_key();
	//test_get_address();

	//test_firmware_sign();
	test_sign();
	//BHPAPI::dump_tx("80000001a7d84de97ca0df0531e7057f6880d675265a52311443defcfa3c130d3111dd4a01000254a80a4c72f6157a7af0a753fc4ac4af6b159a17634dd57fecf319feab6ff71300d8c142000000008d8dfca8e949d5548e3193b88b59d682e3426b1c54a80a4c72f6157a7af0a753fc4ac4af6b159a17634dd57fecf319feab6ff713c8d63a5c0b000000dd8ae706baa7acb84b894ba60a44eb68149ffcc60141400cea0bccf7bc42be15d7f00867f7be0f12ea9e1c21eaa8d96cf9304294510fc6547d38d4e6adf03ba7c4fbb5d6ac27c3216e04f5d517aa367d0d78bf70652efd2321032cc74af88291b8d7ad36812fd78152618a18c8914dad3662f8a0a0cb5aeea26eac");

	//char wallet_address[512];
	//get_wallet_address(gCoin["tBHP"].type, 0, wallet_address);
	//printf("%s\n", wallet_address);

	//string private_key = "68a1639fa801be966c6da92570f7a24caa26618061b4a3248b03247d7fef418a";
	//string public_key = BHPAPI::get_public_key(private_key);
	//string address = BHPAPI::get_address(public_key);
	//BHPAPI::validate_address(address);

	//MakeBhpTransaction_sp();
	//string hash = BHPAPI::tx_hash("80000003364007f6bccf1168b1284ada0086b8f80b371a45c4588c3ab1c50ff691c52a840000364007f6bccf1168b1284ada0086b8f80b371a45c4588c3ab1c50ff691c52a840100fc5b8a6f2b52a650c0d9d1b39a568c98c136f00756b1d8f90b9aa3ac10c7944e00000254a80a4c72f6157a7af0a753fc4ac4af6b159a17634dd57fecf319feab6ff71300205fa012000000dd8ae706baa7acb84b894ba60a44eb68149ffcc654a80a4c72f6157a7af0a753fc4ac4af6b159a17634dd57fecf319feab6ff713fff37911620000008d8dfca8e949d5548e3193b88b59d682e3426b1c0141405d9c54c80f65df47d46289e25b8901c07068b1bd78529e06ce7b5b939511aff136eb461ec91dc4d2643d80a3bfa60e2c6574336753fb47ca60335430b6e99592232103d1998c163fc5137bb2ccc8062968db728081857ecf9d5c489c93935ab1aeece9ac");
	//string hash = BHPAPI::tx_hash("80000003364007f6bccf1168b1284ada0086b8f80b371a45c4588c3ab1c50ff691c52a840000364007f6bccf1168b1284ada0086b8f80b371a45c4588c3ab1c50ff691c52a840100fc5b8a6f2b52a650c0d9d1b39a568c98c136f00756b1d8f90b9aa3ac10c7944e00000254a80a4c72f6157a7af0a753fc4ac4af6b159a17634dd57fecf319feab6ff71300205fa012000000dd8ae706baa7acb84b894ba60a44eb68149ffcc654a80a4c72f6157a7af0a753fc4ac4af6b159a17634dd57fecf319feab6ff713fff37911620000008d8dfca8e949d5548e3193b88b59d682e3426b1c");
	//BHPAPI::dump_tx("80000002d49333d271fd52b9ba3dd9ace5113249c1e6681640cebf8f2aa73a9acc29b09400006b1c9762e491b2a6291023a23e22317cdc4bdc626b2f4f0e10e3d120faf6edb701000254a80a4c72f6157a7af0a753fc4ac4af6b159a17634dd57fecf319feab6ff71300ca9a3b000000006da3cd7869e449546177c5fdac7fa02edd4f9f2054a80a4c72f6157a7af0a753fc4ac4af6b159a17634dd57fecf319feab6ff7130053ae1610000000dd8ae706baa7acb84b894ba60a44eb68149ffcc60141403cdfc5632c777bf6feabcb86c0024f2b2792caa9a397d53168faefb082cacc57108d6673241eade5d8a6d479d903cbef7611fbe0022984f75dcdafaad623545a2321032cc74af88291b8d7ad36812fd78152618a18c8914dad3662f8a0a0cb5aeea26eac");
}
